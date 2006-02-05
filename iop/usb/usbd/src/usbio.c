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
#include "usbio.h"

#include "stdio.h"

void removeEndpointFromQueue(Endpoint *ep) {
	if (!ep->inTdQueue)
		return;

	if (ep->busyNext)
		ep->busyNext->busyPrev = ep->busyPrev;
	else
		memPool.tdQueueEnd[ep->inTdQueue - 1] = ep->busyPrev;

	if (ep->busyPrev)
		ep->busyPrev->busyNext = ep->busyNext;
	else
		memPool.tdQueueStart[ep->inTdQueue - 1] = ep->busyNext;

	ep->inTdQueue = 0;
}

void enqueueEndpoint(Endpoint *ep, uint32 listType) {
	if (ep->inTdQueue)
		return;

	ep->busyNext = NULL;
	ep->busyPrev = memPool.tdQueueEnd[listType - 1];
	if (memPool.tdQueueEnd[listType - 1])
		memPool.tdQueueEnd[listType - 1]->busyNext = ep;
	else
		memPool.tdQueueStart[listType - 1] = ep;
	memPool.tdQueueEnd[listType - 1] = ep;

	ep->inTdQueue = listType;
}

void checkTdQueue(int type) {
	if ((type == GENTD_QUEUE) && !memPool.freeHcTdList)
		return;
	else if ((type == ISOTD_QUEUE) && !memPool.freeHcIsoTdList)
		return;

	if (memPool.tdQueueStart[type - 1])
		handleIoReqList(memPool.tdQueueStart[type - 1]);
}

int setupControlTransfer(Endpoint *ep) {
	HcTD *statusTd, *tailTd, *dataTd = NULL;
	IoRequest *curIoReq = ep->ioReqListStart;

	if (ep->hcEd.tdTail && !ED_HALTED(ep->hcEd) && !ED_SKIPPED(ep->hcEd) && curIoReq) {
		if (curIoReq->destPtr && curIoReq->length) {
			dataTd = allocTd();
			if (!dataTd) {
				enqueueEndpoint(ep, GENTD_QUEUE);
				return 0;
			}
		}
		statusTd = allocTd();
		tailTd = allocTd();

		if (!statusTd || !tailTd) {
			freeTd(statusTd);
			freeTd(tailTd);
			freeTd(dataTd);

			enqueueEndpoint(ep, GENTD_QUEUE);
			return 0;
		}

		if (curIoReq->next)
			curIoReq->next->prev = curIoReq->prev;
		else
			ep->ioReqListEnd = curIoReq->prev;

		if (curIoReq->prev)
			curIoReq->prev->next = curIoReq->next;
		else
			ep->ioReqListStart = curIoReq->next;

		// first stage: setup
		ep->hcEd.tdTail->HcArea = TD_HCAREA(USB_RC_NOTACCESSED, 2, 7, TD_SETUP, 0) << 16;
		ep->hcEd.tdTail->curBufPtr = &curIoReq->devReq;
		ep->hcEd.tdTail->bufferEnd = ((uint8 *)&curIoReq->devReq) + sizeof(UsbDeviceRequest) - 1;

		memPool.hcTdToIoReqLUT[ep->hcEd.tdTail - memPool.hcTdBuf] = curIoReq;

		// second stage: data
		if (dataTd) {
			ep->hcEd.tdTail->next = dataTd;

			if (curIoReq->devReq.requesttype & USB_DIR_IN)
				dataTd->HcArea = TD_HCAREA(USB_RC_NOTACCESSED, 3, 7,  TD_IN, 1) << 16;
			else
				dataTd->HcArea = TD_HCAREA(USB_RC_NOTACCESSED, 3, 7, TD_OUT, 1) << 16;

			dataTd->curBufPtr = curIoReq->destPtr;
			dataTd->bufferEnd = (uint8 *)curIoReq->destPtr + curIoReq->length - 1;
			dataTd->next = statusTd;

			memPool.hcTdToIoReqLUT[dataTd - memPool.hcTdBuf] = curIoReq;
		} else
			ep->hcEd.tdTail->next = statusTd;

		// third stage: status
		if (curIoReq->devReq.requesttype & USB_DIR_IN)
			statusTd->HcArea = TD_HCAREA(USB_RC_NOTACCESSED, 3, 0, TD_OUT, 0) << 16;
		else
			statusTd->HcArea = TD_HCAREA(USB_RC_NOTACCESSED, 3, 0,  TD_IN, 0) << 16;

		statusTd->curBufPtr = NULL;
		statusTd->bufferEnd = NULL;
		statusTd->next = tailTd;
		memPool.hcTdToIoReqLUT[statusTd - memPool.hcTdBuf] = curIoReq;

		ep->hcEd.tdTail = tailTd;

		memPool.ohciRegs->HcCommandStatus |= OHCI_COM_CLF; // control list filled

		// remove endpoint from busy list if there are no IoRequests left
		if (!ep->ioReqListStart)
			removeEndpointFromQueue(ep);
		return 1;
	} else {
		// endpoint error
		removeEndpointFromQueue(ep);
		return 0;
	}
}

int setupIsocronTransfer(Endpoint *ep) {
	IoRequest *curIoReq = ep->ioReqListStart;

	HcED	  *ed = &ep->hcEd;
	HcIsoTD	  *curTd = (HcIsoTD*)ed->tdTail;
	HcIsoTD	  *newTd;

	uint32 frameNo;

	if (ep->hcEd.tdTail && !ED_HALTED(ep->hcEd) && !ED_SKIPPED(ep->hcEd) && curIoReq) {
		newTd = allocIsoTd();
		if (!newTd) {
			enqueueEndpoint(ep, ISOTD_QUEUE);
			return 0;
		}
		
		if (curIoReq->next)
			curIoReq->next->prev = curIoReq->prev;
		else
			ep->ioReqListEnd = curIoReq->prev;

		if (curIoReq->prev)
			curIoReq->prev->next = curIoReq->next;
		else
			ep->ioReqListStart = curIoReq->next;

		if (ed->tdTail == (HcTD *)((uint32)ed->tdHead & ~0xF))
			frameNo = memPool.hcHCCA->FrameNumber + 2;
		else
			frameNo = ep->isochronLastFrameNum;

		frameNo = (frameNo + curIoReq->waitFrames) & 0xFFFF;

		ep->isochronLastFrameNum = frameNo + 1;

		curTd->hcArea	   = (USB_RC_NOTACCESSED << 28) | frameNo;
		curTd->bufferPage0 = (void*)((uint32)curIoReq->destPtr & ~0xFFF);
		curTd->next		   = newTd;

		if (curIoReq->destPtr && curIoReq->length)
			curTd->bufferEnd = (uint8*)curIoReq->destPtr + curIoReq->length - 1;
		else
			curTd->bufferEnd = NULL;

		curTd->psw[0] = (USB_RC_NOTACCESSED << 12) | ((uint32)curIoReq->destPtr & 0xFFF);

		memPool.hcIsoTdToIoReqLUT[curTd - memPool.hcIsoTdBuf] = curIoReq;

		ed->tdTail = (HcTD *)newTd;

		// remove endpoint from busy list if there are no IoRequests left
		if (!ep->ioReqListStart)
			removeEndpointFromQueue(ep);
		return 1;
	} else {
		// endpoint error
		removeEndpointFromQueue(ep);
		return 0;
	}
}

int setupBulkTransfer(Endpoint *ep) {
	IoRequest *curIoReq = ep->ioReqListStart;

	HcED	  *ed = &ep->hcEd;
	HcTD	  *curTd = ed->tdTail;
	HcTD	  *newTd;

	if (ep->hcEd.tdTail && !ED_HALTED(ep->hcEd) && !ED_SKIPPED(ep->hcEd) && curIoReq) {
		newTd = allocTd();
		if (!newTd) {
			enqueueEndpoint(ep, GENTD_QUEUE);
			return 0;
		}

		if (curIoReq->next)
			curIoReq->next->prev = curIoReq->prev;
		else
			ep->ioReqListEnd = curIoReq->prev;

		if (curIoReq->prev)
			curIoReq->prev->next = curIoReq->next;
		else
			ep->ioReqListStart = curIoReq->next;

		curTd->HcArea	 = TD_HCAREA(USB_RC_NOTACCESSED, 0, 0, 3, 1) << 16;
		curTd->next		 = newTd;
		curTd->curBufPtr = curIoReq->destPtr;

		if (curIoReq->destPtr && curIoReq->length)
			curTd->bufferEnd = (uint8*)curIoReq->destPtr + curIoReq->length - 1;
		else
			curTd->bufferEnd = NULL;

		memPool.hcTdToIoReqLUT[curTd - memPool.hcTdBuf] = curIoReq;

		ed->tdTail = newTd;

		if (ep->endpointType == TYPE_BULK)
			memPool.ohciRegs->HcCommandStatus |= OHCI_COM_BLF; // Bulk List Filled

		// remove endpoint from busy list if there are no IoRequests left
		if (!ep->ioReqListStart)
			removeEndpointFromQueue(ep);
		return 1;
	} else {
		// endpoint error
		dbg_printf("ERROR Endpoint error\n");
		removeEndpointFromQueue(ep);
		return 0;
	}
}

void handleIoReqList(Endpoint *ep) {
	if (ep->endpointType == TYPE_CONTROL)
		setupControlTransfer(ep);
	else if (ep->endpointType == TYPE_ISOCHRON)
		setupIsocronTransfer(ep);
	else	// bulk or interrupt
		setupBulkTransfer(ep);
}

int attachIoReqToEndpoint(Endpoint *ep, IoRequest *req, void *destdata, uint16 length, void *callback) {
	if (!ep->correspDevice)
		return USB_RC_BUSY;
	if (req->busyFlag)
		return USB_RC_BUSY;

	req->busyFlag = 1;
	req->correspEndpoint = ep;
	req->destPtr = destdata;
	req->length  = length;
	req->resultCode = 0;
	req->transferedBytes = 0;
	req->callbackProc = callback;

	req->next = NULL;
	req->prev = ep->ioReqListEnd;
	if (ep->ioReqListEnd)
		ep->ioReqListEnd->next = req;
	else
		ep->ioReqListStart = req;
	ep->ioReqListEnd = req;

	handleIoReqList(ep);
	return 0;
}

int doControlTransfer(Endpoint *ep, IoRequest *req,
					   uint8 requestType, uint8 request, uint16 value, uint16 index, uint16 length,
					   void *destdata, void *callback)
{
	if (req->busyFlag) {
		dbg_printf("ERROR: doControlTransfer: IoReq busy\n");
		return USB_RC_BUSY;
	}

	req->devReq.requesttype = requestType;
	req->devReq.request     = request;
	req->devReq.value       = value;
	req->devReq.index       = index;
	req->devReq.length      = length;
    return attachIoReqToEndpoint(ep, req, destdata, length, callback);
}

