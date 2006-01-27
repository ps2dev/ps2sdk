/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: usbd_macro.h 1034 2005-04-23 09:58:42Z tyranid $
# USB Driver function prototypes and constants.
*/
#include "usbdpriv.h"
#include "driver.h"
#include "mem.h"
#include "hub.h"

#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "thevent.h"
#include "intrman.h"

UsbDriver *drvListStart = NULL, *drvListEnd = NULL;
IoRequest *cbListStart = NULL, *cbListEnd = NULL;

int callbackEvent;

int callUsbDriverFunc(int (*func)(int devId), int devId, void *gp) {
	int res;
	if (func) {
	    usbdUnlock();
		// move gp..
		res = func(devId);
		// restore gp...
		usbdLock();
		return res;
	} else
		return 0;
}

void probeDeviceTree(Device *tree, UsbDriver *drv) {
	Device *curDevice;
	for (curDevice = tree->childListStart; curDevice != NULL; curDevice = curDevice->next)
		if (curDevice->deviceStatus == DEVICE_READY) {
			if (curDevice->devDriver == NULL) {
				if (callUsbDriverFunc(drv->probe, curDevice->id, drv->gp) != 0) {
					curDevice->devDriver = drv;
					callUsbDriverFunc(drv->connect, curDevice->id, drv->gp);
				}
			} else if (curDevice->childListStart)
				probeDeviceTree(curDevice, drv);
		}
}

int doRegisterDriver(UsbDriver *drv, void *drvGpSeg) {
	if (drv->next || drv->prev)
		return USB_RC_BUSY;
	if (drvListStart == drv)
		return USB_RC_BUSY;

	if (!drv->name)
		return USB_RC_BADDRIVER;
	if (drv->reserved1 || drv->reserved2)
		return USB_RC_BADDRIVER;

	drv->gp = drvGpSeg;

	drv->prev = drvListEnd;
    if (drvListEnd)
		drvListEnd->next = drv;
	else
		drvListStart = drv;
	drvListEnd = drv;

	if (drv->probe)
		probeDeviceTree(memPool.deviceTreeRoot, drv);

	return 0;
}

void disconnectDriver(Device *tree, UsbDriver *drv) {
	Endpoint *ep, *nextEp;
	if (tree->devDriver == drv) {
		if (tree->endpointListStart) {
			ep = tree->endpointListStart->next;
			
			while (ep) {
				nextEp = ep->next;
				removeEndpointFromDevice(tree, ep);
				ep = nextEp;
			}
		}
		tree->devDriver = NULL;
		tree->privDataField = NULL;
	}

	for (tree = tree->childListStart; tree != NULL; tree = tree->next)
		disconnectDriver(tree, drv);
}

int doUnregisterDriver(UsbDriver *drv) {
	UsbDriver *pos;
	for (pos = drvListStart; pos != NULL; pos = pos->next)
		if (pos == drv) {
			if (drv->next)
				drv->next->prev = drv->prev;
			else
				drvListEnd = drv->prev;

			if (drv->prev)
				drv->prev->next = drv->next;
			else
				drvListStart = drv->next;

			disconnectDriver(memPool.deviceTreeRoot, drv);
			return 0;
		}
	return USB_RC_BADDRIVER;
}

void connectNewDevice(Device *dev) {
	UsbDriver *drv;
	dbg_printf("searching driver for dev %d, FA %02X\n", dev->id, dev->functionAddress);
	for (drv = drvListStart; drv != NULL; drv = drv->next)
		if (callUsbDriverFunc(drv->probe, dev->id, drv->gp) != 0) {
			dev->devDriver = drv;
			dbg_printf("Driver found (%s)\n", drv->name);
			callUsbDriverFunc(drv->connect, dev->id, drv->gp);
			return;
		}
	dbg_printf("no driver found\n");
	// todo: call autoloader here
}

void signalCallbackThreadFunc(IoRequest *req) {
	int intrStat;

	CpuSuspendIntr(&intrStat);
	
	req->prev = cbListEnd;
	req->next = NULL;
	if (cbListEnd)
		cbListEnd->next = req;
	else
		cbListStart = req;
	cbListEnd = req;

	CpuResumeIntr(intrStat);

	SetEventFlag(callbackEvent, 1);
}

void callbackThreadFunc(void *arg) {
	u32 eventRes;
	int intrStat;
	IoRequest *req;
	IoRequest reqCopy;
	while (1) {
		WaitEventFlag(callbackEvent, 1, WEF_CLEAR | WEF_OR, &eventRes);
		do {
			CpuSuspendIntr(&intrStat);

			req = cbListStart;
			if (req) {
				if (req->next)
					req->next->prev = req->prev;
				else
					cbListEnd = req->prev;

				if (req->prev)
					req->prev->next = req->next;
				else
					cbListStart = req->next;
			}
			CpuResumeIntr(intrStat);

			if (req) {
				memcpy(&reqCopy, req, sizeof(IoRequest));
				usbdLock();
				freeIoRequest(req);
				usbdUnlock();

				// add GP stuff
				if (reqCopy.userCallbackProc)
					reqCopy.userCallbackProc(reqCopy.resultCode, reqCopy.transferedBytes, reqCopy.userCallbackArg);
			}
		} while (req);
	}
}

int initCallbackThread(void) {
	iop_event_t event;
	iop_thread_t thread;
	int callbackTid;

	event.attr = event.option = event.bits = 0;
	callbackEvent = CreateEventFlag(&event);

	thread.attr = TH_C;
	thread.option = 0;
	thread.thread = callbackThreadFunc;
	thread.stacksize = 0x4000;
	thread.priority = usbConfig.cbThreadPrio;
	callbackTid = CreateThread(&thread);
	StartThread(callbackTid, NULL);

	return 0;
}


