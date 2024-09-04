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

static void addToHcEndpointList(u8 type, UsbdHcED_t *ed)
{
	ed->m_next = memPool->m_hcEdBuf[type].m_next;
	memPool->m_hcEdBuf[type].m_next = ed;
}

static void removeHcEdFromList(int type, const UsbdHcED_t *hcEd)
{
	UsbdHcED_t *prev;
	UsbdHcED_t *pos;

	prev = &memPool->m_hcEdBuf[type];
	for ( pos = prev->m_next; pos && pos != hcEd; prev = pos, pos = pos->m_next )
	{
	}
	if ( pos )
	{
		prev->m_next = pos->m_next;
	}
}

static int
setupBandwidthInterruptScheduling(UsbdEndpoint_t *ep, const UsbEndpointDescriptor *endpDesc, int isLowSpeedDevice)
{
	int maxPacketSize;
	u32 *interruptBandwidthSchedulingValues;
	int endpType;
	int waitHigh;
	int waitLow;
	int schedulingIndex;
	int i;
	int packetSizeForScheduling;
	u32 *value_ptr2;

	maxPacketSize = endpDesc->wMaxPacketSizeLB + (endpDesc->wMaxPacketSizeHB << 8);
	interruptBandwidthSchedulingValues = memPool->m_interruptBandwidthSchedulingValues;
	if ( endpDesc->bInterval >= 0x20u )
	{
		endpType = 31;
		waitHigh = 1;
		waitLow = 32;
	}
	else if ( endpDesc->bInterval >= 0x10u )
	{
		endpType = 15;
		waitHigh = 2;
		waitLow = 16;
	}
	else if ( endpDesc->bInterval >= 8u )
	{
		endpType = 7;
		waitHigh = 4;
		waitLow = 8;
	}
	else if ( endpDesc->bInterval >= 4u )
	{
		endpType = 3;
		waitHigh = 8;
		waitLow = 4;
	}
	else if ( endpDesc->bInterval >= 2u )
	{
		endpType = 1;
		waitHigh = 16;
		waitLow = 2;
	}
	else
	{
		endpType = 0;
		waitHigh = 32;
		waitLow = 1;
	}
	schedulingIndex = 0;
	if ( waitLow >= 2 )
	{
		int maxValueSum;

		schedulingIndex = -1;
		maxValueSum = 0;
		for ( i = 0; i < waitLow; i += 1 )
		{
			int valueSum;
			u32 *value_ptr1;
			int j;

			valueSum = 0;
			value_ptr1 = &interruptBandwidthSchedulingValues[i];
			for ( j = 0; j < waitHigh; j += 1 )
			{
				valueSum += *value_ptr1;
				value_ptr1 += waitLow;
			}
			if ( schedulingIndex < 0 || valueSum < maxValueSum )
			{
				schedulingIndex = i;
				maxValueSum = valueSum;
			}
		}
		endpType += schedulingIndex;
	}
	packetSizeForScheduling = maxPacketSize + 13;
	if ( maxPacketSize >= 65 )
		packetSizeForScheduling = 77;
	ep->m_schedulingIndex = schedulingIndex;
	ep->m_waitHigh = waitHigh;
	ep->m_waitLow = waitLow;
	ep->m_packetSizeForScheduling = packetSizeForScheduling;
	if ( isLowSpeedDevice )
		packetSizeForScheduling *= 8;
	value_ptr2 = &interruptBandwidthSchedulingValues[schedulingIndex];
	for ( i = 0; i < waitHigh; i += 1 )
	{
		*value_ptr2 += packetSizeForScheduling;
		value_ptr2 += waitLow;
	}
	return endpType;
}

static void removeEndpointFromQueue(const UsbdEndpoint_t *ep, int isLowSpeedDevice)
{
	int i;
	u32 *value_ptr;

	value_ptr = &memPool->m_interruptBandwidthSchedulingValues[ep->m_schedulingIndex];
	for ( i = 0; i < ep->m_waitHigh; i += 1 )
	{
		*value_ptr -= (ep->m_packetSizeForScheduling * (isLowSpeedDevice ? 8 : 1));
		if ( (int)*value_ptr < 0 )
			*value_ptr = 0;
		value_ptr += ep->m_waitLow;
	}
}

UsbdEndpoint_t *openDeviceEndpoint(UsbdDevice_t *dev, const UsbEndpointDescriptor *endpDesc, u32 alignFlag)
{
	UsbdHcTD_t *td;
	int endpType;
	UsbdEndpoint_t *newEp;

	td = NULL;
	endpType = 0;
	newEp = allocEndpointForDevice(dev, alignFlag);
	if ( !newEp )
	{
		dbg_printf("ran out of endpoints\n");
		return NULL;
	}
	if ( endpDesc )
	{
		int hcMaxPktSize;
		int flags;

		hcMaxPktSize = endpDesc->wMaxPacketSizeLB + (endpDesc->wMaxPacketSizeHB << 8);
		switch ( endpDesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK )
		{
			case USB_ENDPOINT_XFER_CONTROL:
			{
				endpType = TYPE_CONTROL;
				break;
			}
			case USB_ENDPOINT_XFER_ISOC:
			{
				endpType = TYPE_ISOCHRON;
				td = (UsbdHcTD_t *)allocIsoTd();
				if ( !td )
				{
					cleanUpFunc(dev, newEp);
					dbg_printf("Open ISOC EP: no TDs left\n");
					return NULL;
				}
				alignFlag = 1;
				break;
			}
			case USB_ENDPOINT_XFER_BULK:
			{
				endpType = TYPE_BULK;
				if ( (endpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN )
					alignFlag = 1;
				break;
			}
			case USB_ENDPOINT_XFER_INT:
			{
				endpType = setupBandwidthInterruptScheduling(newEp, endpDesc, dev->m_isLowSpeedDevice);
				dbg_printf(
					"opening INT endpoint (%d - %p), interval %d, list %d\n", newEp->m_id, newEp, endpDesc->bInterval, endpType);
				if ( (endpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN )
					alignFlag = 1;
				break;
			}
			default:
				break;
		}
		if ( !alignFlag && hcMaxPktSize > 62 )
		{
			hcMaxPktSize = 62;
		}
		flags = (hcMaxPktSize << 16) & 0x7FF0000;
		if ( endpType == TYPE_ISOCHRON )
			flags |= HCED_ISOC;
		if ( dev->m_isLowSpeedDevice )
			flags |= HCED_SPEED;
		flags |= ((endpDesc->bEndpointAddress & 0x1F) << 7)
					 | ((endpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN ? HCED_DIR_IN : HCED_DIR_OUT);
		newEp->m_hcEd->m_hcArea.asu32 = flags | (dev->m_functionAddress & 0x7F);
	}
	else
	{
		newEp->m_hcEd->m_hcArea.asu32 = 0x80000;
		newEp->m_hcEd->m_hcArea.asu32 |= dev->m_isLowSpeedDevice ? HCED_SPEED : 0;
		endpType = TYPE_CONTROL;
	}
	newEp->m_endpointType = endpType;
	if ( !td )
	{
		td = allocTd();
	}
	if ( !td )
	{
		dbg_printf("Ran out of TDs\n");
		cleanUpFunc(dev, newEp);
		return NULL;
	}
	newEp->m_hcEd->m_tdHead = td;
	newEp->m_hcEd->m_tdTail = td;
	addToHcEndpointList(endpType & 0xFF, newEp->m_hcEd);
	return newEp;
}

static void killEndpoint(UsbdEndpoint_t *ep)
{
	UsbdHcED_t *hcEd;
	int i;
	UsbdIoRequest_t *req;

	hcEd = ep->m_hcEd;
	if ( ep->m_endpointType == TYPE_ISOCHRON )
	{
		for ( i = 0; i < usbConfig.m_maxIsoTransfDesc; i += 1 )
		{
			if ( memPool->m_hcIsoTdToIoReqLUT[i] && memPool->m_hcIsoTdToIoReqLUT[i]->m_correspEndpoint == ep )
			{
				freeIoRequest(memPool->m_hcIsoTdToIoReqLUT[i]);
				memPool->m_hcIsoTdToIoReqLUT[i] = NULL;
				freeIsoTd(&memPool->m_hcIsoTdBuf[i]);
			}
		}
		freeIsoTd((UsbdHcIsoTD_t *)hcEd->m_tdTail);
		hcEd->m_tdTail = NULL;
	}
	else
	{
		for ( i = 0; i < usbConfig.m_maxTransfDesc; i += 1 )
		{
			if ( memPool->m_hcTdToIoReqLUT[i] && memPool->m_hcTdToIoReqLUT[i]->m_correspEndpoint == ep )
			{
				freeIoRequest(memPool->m_hcTdToIoReqLUT[i]);
				memPool->m_hcTdToIoReqLUT[i] = NULL;
				freeTd(&memPool->m_hcTdBuf[i]);
			}
		}
		freeTd(hcEd->m_tdTail);
		hcEd->m_tdTail = NULL;
	}
	hcEd->m_tdHead = NULL;
	for ( req = ep->m_ioReqListStart; req; req = ep->m_ioReqListStart )
	{
		if ( req->m_next )
			req->m_next->m_prev = req->m_prev;
		else
			ep->m_ioReqListEnd = req->m_prev;
		if ( req->m_prev )
			req->m_prev->m_next = req->m_next;
		else
			ep->m_ioReqListStart = req->m_next;
		freeIoRequest(req);
	}
	removeEndpointFromQueue(ep, ep->m_correspDevice->m_isLowSpeedDevice);
	if ( ep->m_inTdQueue != NOTIN_QUEUE )
	{
		if ( ep->m_busyNext )
			ep->m_busyNext->m_busyPrev = ep->m_busyPrev;
		else
			memPool->m_tdQueueEnd = ep->m_busyPrev;
		if ( ep->m_busyPrev )
			ep->m_busyPrev->m_busyNext = ep->m_busyNext;
		else
			memPool->m_tdQueueStart = ep->m_busyNext;
		ep->m_inTdQueue = NOTIN_QUEUE;
	}
	ep->m_prev = memPool->m_freeEpListEnd;
	if ( memPool->m_freeEpListEnd )
		memPool->m_freeEpListEnd->m_next = ep;
	else
		memPool->m_freeEpListStart = ep;
	ep->m_next = NULL;
	memPool->m_freeEpListEnd = ep;
}

int removeEndpointFromDevice(UsbdDevice_t *dev, UsbdEndpoint_t *ep)
{
	ep->m_hcEd->m_hcArea.stru.m_hcArea |= HCED_SKIP;
	removeHcEdFromList(ep->m_endpointType, ep->m_hcEd);
	if ( ep->m_next )
		ep->m_next->m_prev = ep->m_prev;
	else
		dev->m_endpointListEnd = ep->m_prev;
	if ( ep->m_prev )
		ep->m_prev->m_next = ep->m_next;
	else
		dev->m_endpointListStart = ep->m_next;
	ep->m_correspDevice = NULL;
	addTimerCallback(&ep->m_timer, (TimerCallback)killEndpoint, ep, 200);
	return 0;
}
