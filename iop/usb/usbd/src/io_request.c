/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "usbdpriv.h"

static int setupControlTransfer(UsbdEndpoint_t *ep)
{
	UsbdIoRequest_t *curIoReq;
	UsbdHcTD_t *dataTd;
	UsbdHcTD_t *tailTd;
	UsbdHcTD_t *statusTd;

	if ( !ep->m_hcEd->m_tdTail || ED_HALTED(*(ep->m_hcEd)) || ED_SKIPPED(*(ep->m_hcEd)) || !ep->m_ioReqListStart )
	{
		// endpoint error
		if ( ep->m_inTdQueue == NOTIN_QUEUE )
			return 0;
		if ( ep->m_busyNext )
			ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
		else
			memPool->m_tdQueueEnd = ep->m_busyPrev;
		if ( ep->m_busyPrev )
		{
			ep->m_busyPrev->m_busyNext = ep->m_busyNext;
		}
		else
		{
			memPool->m_tdQueueStart = ep->m_busyNext;
		}
		ep->m_inTdQueue = NOTIN_QUEUE;
		return 0;
	}
	curIoReq = ep->m_ioReqListStart;
	dataTd = NULL;
	if ( curIoReq->m_destPtr && (int)curIoReq->m_length > 0 )
	{
		dataTd = allocTd();
		if ( !dataTd )
		{
			if ( ep->m_inTdQueue != NOTIN_QUEUE )
				return 0;
			ep->m_busyPrev = memPool->m_tdQueueEnd;
			if ( memPool->m_tdQueueEnd )
			{
				memPool->m_tdQueueEnd->m_busyNext = ep;
			}
			else
			{
				memPool->m_tdQueueStart = ep;
			}
			ep->m_busyNext = NULL;
			memPool->m_tdQueueEnd = ep;
			ep->m_inTdQueue = GENTD_QUEUE;
			return 0;
		}
	}
	statusTd = allocTd();
	tailTd = allocTd();
	if ( !statusTd || !tailTd )
	{
		freeTd(dataTd);
		freeTd(statusTd);
		freeTd(tailTd);
		if ( ep->m_inTdQueue != NOTIN_QUEUE )
			return 0;
		ep->m_busyPrev = memPool->m_tdQueueEnd;
		if ( memPool->m_tdQueueEnd )
		{
			memPool->m_tdQueueEnd->m_busyNext = ep;
		}
		else
		{
			memPool->m_tdQueueStart = ep;
		}
		ep->m_busyNext = NULL;
		memPool->m_tdQueueEnd = ep;
		ep->m_inTdQueue = GENTD_QUEUE;
		return 0;
	}
	if ( curIoReq->m_next )
		curIoReq->m_next->m_prev = curIoReq->m_prev;
	else
		ep->m_ioReqListEnd = curIoReq->m_prev;
	if ( curIoReq->m_prev )
		curIoReq->m_prev->m_next = curIoReq->m_next;
	else
		ep->m_ioReqListStart = curIoReq->m_next;
	// first stage: setup
	ep->m_hcEd->m_tdTail->m_hcArea = TD_HCAREA(USB_RC_NOTACCESSED, 2, 7, TD_SETUP, 0) << 16;
	ep->m_hcEd->m_tdTail->m_curBufPtr = &curIoReq->m_devReq;
	ep->m_hcEd->m_tdTail->m_next = dataTd ? dataTd : statusTd;
	ep->m_hcEd->m_tdTail->m_bufferEnd = ((u8 *)&curIoReq->m_devReq) + sizeof(UsbDeviceRequest) - 1;
	memPool->m_hcTdToIoReqLUT[ep->m_hcEd->m_tdTail - memPool->m_hcTdBuf] = curIoReq;
	// second stage: data
	if ( dataTd )
	{
		dataTd->m_hcArea = (curIoReq->m_devReq.requesttype & USB_ENDPOINT_DIR_MASK) != USB_DIR_IN ?
												 (TD_HCAREA(USB_RC_NOTACCESSED, 3, 7, TD_OUT, 1) << 16) :
												 (TD_HCAREA(USB_RC_NOTACCESSED, 3, 7, TD_IN, 1) << 16);
		dataTd->m_next = statusTd;
		dataTd->m_curBufPtr = curIoReq->m_destPtr;
		dataTd->m_bufferEnd = (u8 *)curIoReq->m_destPtr + curIoReq->m_length - 1;
		memPool->m_hcTdToIoReqLUT[dataTd - memPool->m_hcTdBuf] = curIoReq;
	}
	// third stage: status
	statusTd->m_hcArea = (curIoReq->m_devReq.requesttype & USB_ENDPOINT_DIR_MASK) != USB_DIR_IN ?
												 (TD_HCAREA(USB_RC_NOTACCESSED, 3, 0, TD_IN, 0) << 16) :
												 (TD_HCAREA(USB_RC_NOTACCESSED, 3, 0, TD_OUT, 0) << 16);
	statusTd->m_curBufPtr = NULL;
	statusTd->m_next = tailTd;
	statusTd->m_bufferEnd = NULL;
	memPool->m_hcTdToIoReqLUT[statusTd - memPool->m_hcTdBuf] = curIoReq;
	ep->m_hcEd->m_tdTail = tailTd;
	memPool->m_ohciRegs->HcCommandStatus |= OHCI_COM_CLF;  // control list filled
	if ( ep->m_ioReqListStart )
	{
		return 1;
	}
	if ( ep->m_inTdQueue == NOTIN_QUEUE )
	{
		return 1;
	}
	// remove endpoint from busy list if there are no IoRequests left
	if ( ep->m_busyNext )
		ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
	else
		memPool->m_tdQueueEnd = ep->m_busyPrev;
	if ( ep->m_busyPrev )
		ep->m_busyPrev->m_busyNext = ep->m_busyNext;
	else
		memPool->m_tdQueueStart = ep->m_busyNext;
	ep->m_inTdQueue = NOTIN_QUEUE;
	return 1;
}

static int setupIsocronTransfer(UsbdEndpoint_t *ep)
{
	UsbdHcED_t *ed;
	UsbdIoRequest_t *curIoReq;
	UsbdHcIsoTD_t *curTd;
	UsbdHcIsoTD_t *newTd;
	u16 frameNo;

	ed = ep->m_hcEd;
	if ( !ed->m_tdTail || ED_HALTED(*ed) || ED_SKIPPED(*ed) || !ep->m_ioReqListStart )
	{
		// endpoint error
		if ( ep->m_inTdQueue == NOTIN_QUEUE )
			return 0;
		if ( ep->m_busyNext )
			ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
		else
			memPool->m_tdQueueEnd = ep->m_busyPrev;
		if ( ep->m_busyPrev )
		{
			ep->m_busyPrev->m_busyNext = ep->m_busyNext;
		}
		else
		{
			memPool->m_tdQueueStart = ep->m_busyNext;
		}
		ep->m_inTdQueue = NOTIN_QUEUE;
		return 0;
	}
	curIoReq = ep->m_ioReqListStart;
	curTd = (UsbdHcIsoTD_t *)ed->m_tdTail;
	newTd = allocIsoTd();
	if ( !newTd )
	{
		if ( ep->m_inTdQueue != NOTIN_QUEUE )
			return 0;
		ep->m_busyPrev = memPool->m_tdQueueEnd;
		if ( memPool->m_tdQueueEnd )
			memPool->m_tdQueueEnd->m_busyNext = ep;
		else
			memPool->m_tdQueueStart = ep;
		ep->m_busyNext = NULL;
		memPool->m_tdQueueEnd = ep;
		ep->m_inTdQueue = ISOTD_QUEUE;
		return 0;
	}
	if ( curIoReq->m_next )
		curIoReq->m_next->m_prev = curIoReq->m_prev;
	else
		ep->m_ioReqListEnd = curIoReq->m_prev;
	if ( curIoReq->m_prev )
		curIoReq->m_prev->m_next = curIoReq->m_next;
	else
		ep->m_ioReqListStart = curIoReq->m_next;
	if ( (UsbdHcTD_t *)((uiptr)ed->m_tdHead & ~0xF) == ed->m_tdTail )
		frameNo = (memPool->m_hcHCCA->FrameNumber + 2) & 0xFFFF;
	else
		frameNo = (ep->m_isochronLastFrameNum) & 0xFFFF;
	frameNo = (u16)(frameNo + (curIoReq->m_waitFrames & 0xFFFF));
	ep->m_isochronLastFrameNum = (curIoReq->m_req.bNumPackets ? (curIoReq->m_req.bNumPackets & 0xFFFF) : 1) + frameNo;
	if ( curIoReq->m_req.bNumPackets )
	{
		int psw0_tmp;
		int i;

		curTd->m_hcArea = ((curIoReq->m_req.bNumPackets - 1) << 24) | frameNo | (USB_RC_NOTACCESSED << 28);
		curTd->m_next = newTd;
		curTd->m_bufferPage0 = (void *)((uiptr)curIoReq->m_destPtr & ~0xFFF);
		curTd->m_bufferEnd = NULL;
		if ( curIoReq->m_destPtr && (int)(curIoReq->m_length) > 0 )
		{
			curTd->m_bufferEnd = (u8 *)curIoReq->m_destPtr + ((int)curIoReq->m_length - 1);
		}
		psw0_tmp = ((uiptr)curIoReq->m_destPtr & 0xFFF) | (USB_RC_NOTACCESSED << 12);
		for ( i = 0; i < (int)curIoReq->m_req.bNumPackets; i += 1 )
		{
			curTd->m_psw[i] = psw0_tmp;
			psw0_tmp += curIoReq->m_req.Packets[i].bLength;
		}
	}
	else
	{
		curTd->m_hcArea = frameNo | (USB_RC_NOTACCESSED << 28);
		curTd->m_next = newTd;
		curTd->m_bufferPage0 = (void *)((uiptr)curIoReq->m_destPtr & ~0xFFF);
		curTd->m_bufferEnd = NULL;
		if ( curIoReq->m_destPtr && (int)(curIoReq->m_length) > 0 )
		{
			curTd->m_bufferEnd = (u8 *)curIoReq->m_destPtr + ((int)curIoReq->m_length - 1);
		}
		curTd->m_psw[0] = ((uiptr)curIoReq->m_destPtr & 0xFFF) | (USB_RC_NOTACCESSED << 12);
	}
	memPool->m_hcIsoTdToIoReqLUT[curTd - memPool->m_hcIsoTdBuf] = curIoReq;
	ed->m_tdTail = (UsbdHcTD_t *)newTd;
	if ( ep->m_ioReqListStart )
	{
		return 1;
	}
	if ( ep->m_inTdQueue == NOTIN_QUEUE )
	{
		return 1;
	}
	// remove endpoint from busy list if there are no IoRequests left
	if ( ep->m_busyNext )
		ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
	else
		memPool->m_tdQueueEnd = ep->m_busyPrev;
	if ( ep->m_busyPrev )
		ep->m_busyPrev->m_busyNext = ep->m_busyNext;
	else
		memPool->m_tdQueueStart = ep->m_busyNext;
	ep->m_inTdQueue = NOTIN_QUEUE;
	return 1;
}

static int setupBulkTransfer(UsbdEndpoint_t *ep)
{
	UsbdHcED_t *ed;
	UsbdIoRequest_t *curIoReq;
	UsbdHcTD_t *curTd;
	UsbdHcTD_t *newTd;

	ed = ep->m_hcEd;
	if ( !ed->m_tdTail || ED_HALTED(*ed) || ED_SKIPPED(*ed) || !ep->m_ioReqListStart )
	{
		// endpoint error
		dbg_printf("ERROR UsbdEndpoint_t error\n");
		if ( ep->m_inTdQueue == NOTIN_QUEUE )
			return 0;
		if ( ep->m_busyNext )
			ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
		else
			memPool->m_tdQueueEnd = ep->m_busyPrev;
		if ( ep->m_busyPrev )
		{
			ep->m_busyPrev->m_busyNext = ep->m_busyNext;
		}
		else
		{
			memPool->m_tdQueueStart = ep->m_busyNext;
		}
		ep->m_inTdQueue = NOTIN_QUEUE;
		return 0;
	}
	curIoReq = ep->m_ioReqListStart;
	curTd = ed->m_tdTail;
	newTd = allocTd();
	if ( !newTd )
	{
		if ( ep->m_inTdQueue != NOTIN_QUEUE )
			return 0;
		ep->m_busyPrev = memPool->m_tdQueueEnd;
		if ( memPool->m_tdQueueEnd )
			memPool->m_tdQueueEnd->m_busyNext = ep;
		else
			memPool->m_tdQueueStart = ep;
		ep->m_busyNext = NULL;
		memPool->m_tdQueueEnd = ep;
		ep->m_inTdQueue = GENTD_QUEUE;
		return 0;
	}
	if ( curIoReq->m_next )
		curIoReq->m_next->m_prev = curIoReq->m_prev;
	else
		ep->m_ioReqListEnd = curIoReq->m_prev;
	if ( curIoReq->m_prev )
		curIoReq->m_prev->m_next = curIoReq->m_next;
	else
		ep->m_ioReqListStart = curIoReq->m_next;
	curTd->m_hcArea = TD_HCAREA(USB_RC_NOTACCESSED, 0, 0, 3, 1) << 16;
	curTd->m_next = newTd;
	curTd->m_curBufPtr = curIoReq->m_destPtr;
	curTd->m_bufferEnd = NULL;
	if ( curIoReq->m_destPtr && (int)curIoReq->m_length > 0 )
	{
		curTd->m_bufferEnd = (u8 *)curIoReq->m_destPtr + ((int)curIoReq->m_length - 1);
	}
	memPool->m_hcTdToIoReqLUT[curTd - memPool->m_hcTdBuf] = curIoReq;
	ed->m_tdTail = newTd;
	if ( ep->m_endpointType == TYPE_BULK )
		memPool->m_ohciRegs->HcCommandStatus |= OHCI_COM_BLF;  // Bulk List Filled
	if ( ep->m_ioReqListStart )
	{
		return 1;
	}
	if ( ep->m_inTdQueue == NOTIN_QUEUE )
	{
		return 1;
	}
	// remove endpoint from busy list if there are no IoRequests left
	if ( ep->m_busyNext )
		ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
	else
		memPool->m_tdQueueEnd = ep->m_busyPrev;
	if ( ep->m_busyPrev )
		ep->m_busyPrev->m_busyNext = ep->m_busyNext;
	else
		memPool->m_tdQueueStart = ep->m_busyNext;
	ep->m_inTdQueue = NOTIN_QUEUE;
	return 1;
}

void handleIoReqList(UsbdEndpoint_t *ep)
{
	switch ( ep->m_endpointType )
	{
		case TYPE_CONTROL:
		{
			setupControlTransfer(ep);
			break;
		}
		case TYPE_ISOCHRON:
		{
			setupIsocronTransfer(ep);
			break;
		}
		case TYPE_BULK:
		default:  // bulk or interrupt
		{
			setupBulkTransfer(ep);
			break;
		}
	}
}
