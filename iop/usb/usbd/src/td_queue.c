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

static void checkGenTdQueue(void)
{
	UsbdEndpoint_t *queueStart_tmp1;

	if ( !memPool->m_freeHcTdList )
	{
		return;
	}
	for ( queueStart_tmp1 = memPool->m_tdQueueStart; queueStart_tmp1 && queueStart_tmp1->m_inTdQueue != GENTD_QUEUE;
				queueStart_tmp1 = queueStart_tmp1->m_busyNext )
	{
	}
	if ( queueStart_tmp1 )
		handleIoReqList(queueStart_tmp1);
}

void processDoneQueue_GenTd(UsbdHcTD_t *arg)
{
	u32 tdHcArea;
	const u8 *curBufPtr;
	const void *bufferEnd;
	UsbdIoRequest_t **lut_ptr1;
	u32 hcRes;
	UsbdIoRequest_t *req_1;
	UsbdHcED_t *ed;
	UsbdHcTD_t *tdListPos_2;
	UsbdHcTD_t *nextTd;
	UsbdIoRequest_t *req_2;
	UsbdIoRequest_t *listPos;
	UsbdIoRequest_t *pos;
	UsbdIoRequest_t *next_tmp1;
	UsbdIoRequest_t *firstElem;
	UsbdIoRequest_t *lastElem;

	lastElem = NULL;
	firstElem = NULL;
	tdHcArea = arg->m_hcArea;
	curBufPtr = (u8 *)arg->m_curBufPtr;
	bufferEnd = arg->m_bufferEnd;
	lut_ptr1 = &memPool->m_hcTdToIoReqLUT[arg - memPool->m_hcTdBuf];
	hcRes = tdHcArea >> 28;
	req_1 = *lut_ptr1;
	*lut_ptr1 = NULL;
	if ( !req_1 )
	{
		return;
	}
	freeTd(arg);
	if ( bufferEnd && ((tdHcArea & 0x180000) != 0) )  // dir != SETUP
	{
		// transfer successful when !curBufPtr
		req_1->m_transferedBytes = curBufPtr ? (u32)(curBufPtr - (u8 *)req_1->m_destPtr) : req_1->m_length;
	}
	if ( req_1->m_resultCode == USB_RC_OK )
		req_1->m_resultCode = hcRes;
	if ( hcRes || ((tdHcArea & 0xE00000) != 0xE00000) )  // E00000: interrupts disabled
	{
		req_1->m_prev = NULL;
		firstElem = req_1;
		req_1->m_next = NULL;
		lastElem = req_1;
	}
	ed = req_1->m_correspEndpoint->m_hcEd;
	if ( hcRes && ED_HALTED(*ed) )
	{
		for ( tdListPos_2 = (UsbdHcTD_t *)((uiptr)ed->m_tdHead & ~0xF); tdListPos_2 && tdListPos_2 != ed->m_tdTail;
					tdListPos_2 = nextTd )
		{
			UsbdIoRequest_t **lut_ptr2;

			nextTd = tdListPos_2->m_next;
			freeTd(tdListPos_2);
			lut_ptr2 = &memPool->m_hcTdToIoReqLUT[tdListPos_2 - memPool->m_hcTdBuf];
			req_2 = *lut_ptr2;
			*lut_ptr2 = NULL;
			if ( !req_2 )
			{
				continue;
			}
			for ( listPos = firstElem; listPos && listPos != req_2; listPos = listPos->m_next )
			{
			}
			if ( listPos )
			{
				continue;
			}
			req_2->m_resultCode = USB_RC_ABORTED;
			req_2->m_prev = lastElem;
			if ( lastElem )
				lastElem->m_next = req_2;
			else
				firstElem = req_2;
			req_2->m_next = NULL;
			lastElem = req_2;
		}
		ed->m_tdHead = ed->m_tdTail;
	}
	for ( pos = firstElem; pos; pos = next_tmp1 )
	{
		pos->m_busyFlag = 0;
		next_tmp1 = pos->m_next;
		if ( pos->m_correspEndpoint->m_correspDevice )
		{
			if ( pos->m_callbackProc )
				pos->m_callbackProc(pos);
		}
		else
		{
			freeIoRequest(pos);
		}
	}
	checkGenTdQueue();
}

static void checkIsoTdQueue(void)
{
	UsbdEndpoint_t *queueStart_tmp1;

	if ( !memPool->m_freeHcIsoTdList )
	{
		return;
	}
	for ( queueStart_tmp1 = memPool->m_tdQueueStart; queueStart_tmp1 && queueStart_tmp1->m_inTdQueue != ISOTD_QUEUE;
				queueStart_tmp1 = queueStart_tmp1->m_busyNext )
	{
	}
	if ( queueStart_tmp1 )
		handleIoReqList(queueStart_tmp1);
}

void processDoneQueue_IsoTd(UsbdHcIsoTD_t *arg)
{
	u32 hcArea;
	unsigned int psw_tmp;
	u32 tdHcRes;
	unsigned int pswRes;
	UsbdIoRequest_t **lut_ptr1;
	int pswOfs;
	UsbdIoRequest_t *req_1;
	UsbdHcED_t *ed;
	UsbdHcTD_t *tdHead;
	UsbdHcIsoTD_t *curTd;
	UsbdHcIsoTD_t *nextTd;
	UsbdIoRequest_t *req_2;
	UsbdIoRequest_t *listPos;
	UsbdIoRequest_t *pos;
	UsbdIoRequest_t *next_tmp1;
	UsbdIoRequest_t *listStart;
	UsbdIoRequest_t *listEnd;

	hcArea = arg->m_hcArea;
	psw_tmp = arg->m_psw[0];
	tdHcRes = hcArea >> 28;
	pswRes = psw_tmp >> 12;
	lut_ptr1 = &memPool->m_hcIsoTdToIoReqLUT[arg - memPool->m_hcIsoTdBuf];
	pswOfs = psw_tmp & 0x7FF;
	req_1 = *lut_ptr1;
	*lut_ptr1 = NULL;
	if ( !req_1 )
	{
		return;
	}
	if ( req_1->m_req.bNumPackets )
		bcopy(arg->m_psw, req_1->m_req.Packets, 16);
	freeIsoTd(arg);
	req_1->m_transferedBytes = 0;
	if ( req_1->m_req.bNumPackets )
	{
		req_1->m_resultCode = tdHcRes;
	}
	else
	{
		req_1->m_resultCode = tdHcRes | (pswRes << 4);
		if ( tdHcRes == USB_RC_OK && (pswRes == USB_RC_OK || pswRes == USB_RC_DATAUNDER) )
		{
			if ( (req_1->m_correspEndpoint->m_hcEd->m_hcArea.stru.m_hcArea & HCED_DIR_MASK) == HCED_DIR_IN )
				req_1->m_transferedBytes = pswOfs;
			else
				req_1->m_transferedBytes = req_1->m_length;
		}
	}
	req_1->m_prev = NULL;
	listStart = req_1;
	req_1->m_next = NULL;
	listEnd = req_1;
	ed = req_1->m_correspEndpoint->m_hcEd;
	tdHead = ed->m_tdHead;
	if ( ED_HALTED(*ed) )
	{
		for ( curTd = (UsbdHcIsoTD_t *)((uiptr)tdHead & ~0xF); curTd && curTd != (UsbdHcIsoTD_t *)(ed->m_tdTail);
					curTd = nextTd )
		{
			UsbdIoRequest_t **lut_ptr2;

			nextTd = curTd->m_next;
			freeIsoTd(curTd);
			lut_ptr2 = &memPool->m_hcIsoTdToIoReqLUT[curTd - memPool->m_hcIsoTdBuf];
			req_2 = *lut_ptr2;
			*lut_ptr2 = NULL;
			if ( req_2 )
			{
				for ( listPos = listStart; listPos && listPos != req_2; listPos = listPos->m_next )
				{
				}
				if ( listPos )
				{
					continue;
				}
				req_2->m_resultCode = USB_RC_ABORTED;
				req_2->m_prev = listEnd;
				if ( listEnd )
					listEnd->m_next = req_2;
				else
					listStart = req_2;
				req_2->m_next = NULL;
				listEnd = req_2;
			}
		}
		ed->m_tdHead = ed->m_tdTail;
	}
	for ( pos = listStart; pos; pos = next_tmp1 )
	{
		pos->m_busyFlag = 0;
		next_tmp1 = pos->m_next;
		if ( pos->m_correspEndpoint->m_correspDevice )
		{
			if ( pos->m_callbackProc )
				pos->m_callbackProc(pos);
		}
		else
		{
			freeIoRequest(pos);
		}
	}
	checkIsoTdQueue();
}
