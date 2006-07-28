/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# USB Driver function prototypes and constants.
*/
#include "usbdpriv.h"
#include "mem.h"
#include "hcd.h"
#include "hub.h"
#include "usbio.h"

#include "stdio.h"
#include "sysclib.h"
#include "thsemap.h"
#include "loadcore.h"

#define WELCOME_STR "FreeUsbd v.0.1.2\n"

extern struct irx_export_table _exp_usbd;

UsbdConfig usbConfig = {
	0x20,	// maxDevices
	0x40,	// maxEndpoints
	0x80,	// maxTransDesc
	0x80,	// maxIsoTransfDesc
	0x100,	// maxIoReqs
	0x200,	// maxStaticDescSize
	8,		// maxHubDevices
	8,		// maxPortsPerHub

	0x1E,	// hcdThreadPrio
	0x24	// cbThreadPrio
};

int usbdSema;

int usbdLock(void) {
	return WaitSema(usbdSema);
}

int usbdUnlock(void) {
	return SignalSema(usbdSema);
}

int doGetDeviceLocation(Device *dev, uint8 *path) {
	uint8 tempPath[6];
	int count, cpCount;
	for (count = 0; (count < 6) && (dev != memPool.deviceTreeRoot); count++) {
		tempPath[count] = dev->attachedToPortNo;
		dev = dev->parent;
	}
	if (dev == memPool.deviceTreeRoot) {
		for (cpCount = 0; cpCount < 7; cpCount++) {
			if (cpCount < count)
				path[cpCount] = tempPath[count - (cpCount + 1)];
			else
				path[cpCount] = 0;
		}
		return 0;
	} else
		return USB_RC_BADHUBDEPTH;
}

void processDoneQueue_IsoTd(HcIsoTD *arg) {
	uint32 tdHcRes = arg->hcArea >> 28;
	uint32 pswRes = arg->psw[0] >> 12;
	uint32 pswOfs = arg->psw[0] & 0x7FF;

	IoRequest *listStart = NULL, *listEnd = NULL;

	IoRequest *req = memPool.hcIsoTdToIoReqLUT[arg - memPool.hcIsoTdBuf];
	if (!req)
		return;
	
	memPool.hcIsoTdToIoReqLUT[arg - memPool.hcIsoTdBuf] = NULL;
	freeIsoTd(arg);
	req->transferedBytes = 0;
	req->resultCode = (pswRes << 4) | tdHcRes;

	if ((tdHcRes == USB_RC_OK) && ((pswRes == USB_RC_OK) || (pswRes == USB_RC_DATAUNDER))) {
		if ((req->correspEndpoint->hcEd.hcArea & HCED_DIR_MASK) == HCED_DIR_IN)
			req->transferedBytes = pswOfs;
		else
			req->transferedBytes = req->length;
	}

	req->prev = listEnd;
	if (listEnd)
		listEnd->next = req;
	else
		listStart = req;
	req->next = NULL;
	listEnd = req;

	HcED *ed = &req->correspEndpoint->hcEd;
	if (ED_HALTED(req->correspEndpoint->hcEd)) {
		HcIsoTD *curTd = (HcIsoTD *)((uint32)ed->tdHead & ~0xF);

		while (curTd && (curTd != (HcIsoTD*)ed->tdTail)) {
			HcIsoTD *nextTd = curTd->next;
			freeIsoTd(curTd);

			IoRequest *req = memPool.hcIsoTdToIoReqLUT[curTd - memPool.hcIsoTdBuf];
			if (req) {
				memPool.hcIsoTdToIoReqLUT[arg - memPool.hcIsoTdBuf] = NULL;
				IoRequest *listPos;
				for (listPos = listStart; listPos != NULL; listPos = listPos->next)
					if (listPos == req)
						break;
				if (listPos == NULL) {
					req->resultCode = USB_RC_ABORTED;
					req->prev = listEnd;
					if (listEnd)
						listEnd->next = req;
					else
						listStart = req;
					req->next = NULL;
					listEnd = req;
				}
			}
			curTd = nextTd;
		}
		ed->tdHead = ed->tdTail;
	}

	IoRequest *listPos = listStart;
	while (listPos) {
		IoRequest *listNext = listPos->next;
		listPos->busyFlag = 0;
		if (listPos->correspEndpoint->correspDevice) {
			if (listPos->callbackProc)
				listPos->callbackProc(listPos);
		} else
			freeIoRequest(listPos);
		listPos = listNext;
	}
	checkTdQueue(ISOTD_QUEUE);
}

void processDoneQueue_GenTd(HcTD *arg) {
	IoRequest *req;
	IoRequest *firstElem = NULL, *lastElem = NULL;

	uint32 hcRes;
	
	if ((req = memPool.hcTdToIoReqLUT[arg - memPool.hcTdBuf])) {
		memPool.hcTdToIoReqLUT[arg - memPool.hcTdBuf] = NULL;
		
		uint32 tdHcArea = arg->HcArea;
		
		if (arg->bufferEnd && (tdHcArea & 0x180000)) { // dir != SETUP
			if (arg->curBufPtr == 0) // transfer successful
				req->transferedBytes = req->length;
			else
				req->transferedBytes = (uint8 *)arg->curBufPtr - (uint8 *)req->destPtr;
		}
		hcRes = tdHcArea >> 28;
		freeTd(arg);

		if (req->resultCode == USB_RC_OK)
			req->resultCode = hcRes;

		if (hcRes || ((tdHcArea & 0xE00000) != 0xE00000)) { // E00000: interrupts disabled
			req->prev = lastElem;
			if (lastElem)
				lastElem->next = req;
			else
				firstElem = req;
			req->next = NULL;
			lastElem = req;
		}

		HcED *ed = &req->correspEndpoint->hcEd;
		if (hcRes && ED_HALTED(req->correspEndpoint->hcEd)) {
			HcTD *tdListPos = (HcTD*)((uint32)ed->tdHead & ~0xF);
			while (tdListPos && (tdListPos != ed->tdTail)) {
				HcTD *nextTd = tdListPos->next;
				freeTd(tdListPos);

				IoRequest *req = memPool.hcTdToIoReqLUT[tdListPos - memPool.hcTdBuf];
				if (req) {
					memPool.hcTdToIoReqLUT[tdListPos - memPool.hcTdBuf] = NULL;

					IoRequest *listPos;
					for (listPos = firstElem; listPos != NULL; listPos = listPos->next)
						if (listPos == req)
							break;

					if (!listPos) {
						req->resultCode = USB_RC_ABORTED;
						req->prev = lastElem;
						if (lastElem)
							lastElem->next = req;
						else
							firstElem = req;
						req->next = NULL;
						lastElem = req;
					}
				}
				tdListPos = nextTd;
			}
			ed->tdHead = ed->tdTail;
		}

		IoRequest *pos = firstElem;
		while (pos) {
			pos->busyFlag = 0;
			Device *dev = pos->correspEndpoint->correspDevice;
			IoRequest *next = pos->next;
			if (dev) {
				if (pos->callbackProc)
					pos->callbackProc(pos);
			} else
				freeIoRequest(pos);
			pos = next;
		}
		checkTdQueue(GENTD_QUEUE);
	}
}

void handleTimerList(void) {
	TimerCbStruct *timer = memPool.timerListStart;
	if (timer) {
		if (timer->delayCount > 0)
			timer->delayCount--;

		while (memPool.timerListStart && (memPool.timerListStart->delayCount == 0)) {
			dbg_printf("timer expired\n");
			timer = memPool.timerListStart;

			memPool.timerListStart = timer->next;
			if (timer->next)
				timer->next->prev = NULL;
			else
				memPool.timerListEnd = NULL;
			timer->next = timer->prev = NULL;
			timer->isActive = 0;
			timer->callbackProc(timer->callbackArg);
		}
	}
	// disable SOF interrupts if there are no timers left
	if (memPool.timerListStart == NULL)
		memPool.ohciRegs->HcInterruptDisable = OHCI_INT_SF;
}

int _start(int argc, char **argv) {
	iop_sema_t sema;

	// todo: parse args

	printf(WELCOME_STR);

	dbg_printf("library entries...\n");

	if (RegisterLibraryEntries(&_exp_usbd) != 0) {
		dbg_printf("RegisterLibraryEntries failed\n");
		// todo: handle correctly...
		return 1;
	}

    sema.attr = 1;
	sema.option = 0;
	sema.initial = 1;
	sema.max = 1;
	usbdSema = CreateSema(&sema);

	hcdInit();

	dbg_printf("Init done\n");
    return 0;
}

