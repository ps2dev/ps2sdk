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

UsbdDevice_t *fetchDeviceById(int devId)
{
	UsbdDevice_t *dev;

	if ( devId <= 0 )
	{
		return NULL;
	}
	if ( devId >= usbConfig.m_maxDevices )
	{
		return NULL;
	}
	dev = &memPool->m_deviceTreeBuf[devId];
	return ( !dev->m_parent ) ? NULL : dev;
}

UsbdEndpoint_t *fetchEndpointById(int id)
{
	UsbdEndpoint_t *ep;

	if ( id < 0 )
	{
		return NULL;
	}
	if ( id >= usbConfig.m_maxEndpoints )
	{
		return NULL;
	}
	ep = &memPool->m_endpointBuf[id];
	return ( !ep->m_correspDevice ) ? NULL : ep;
}

UsbdDevice_t *getDeviceTreeRoot(void)
{
	return memPool->m_deviceTreeRoot;
}

UsbdDevice_t *attachChildDevice(UsbdDevice_t *parent, u32 portNum)
{
	UsbdDevice_t *newDev;

	newDev = memPool->m_freeDeviceListStart;
	if ( !newDev )
	{
		dbg_printf("Ran out of device handles\n");
		return NULL;
	}
	*( newDev->m_next ? &(newDev->m_next->m_prev) : &(memPool->m_freeDeviceListEnd) ) = newDev->m_prev;
	*( newDev->m_prev ? &(newDev->m_prev->m_next) : &(memPool->m_freeDeviceListStart) ) = newDev->m_next;
	newDev->m_endpointListEnd = NULL;
	newDev->m_endpointListStart = NULL;
	newDev->m_devDriver = NULL;
	newDev->m_deviceStatus = DEVICE_NOTCONNECTED;
	newDev->m_resetFlag = 0;
	newDev->m_magicPowerValue = 0;
	newDev->m_childListEnd = NULL;
	newDev->m_childListStart = NULL;
	newDev->m_parent = parent;
	newDev->m_attachedToPortNo = portNum;
	newDev->m_privDataField = NULL;
	if ( parent )
	{
		newDev->m_prev = parent->m_childListEnd;
		*( parent->m_childListEnd ? &(parent->m_childListEnd->m_next) : &(parent->m_childListStart) ) = newDev;
		newDev->m_next = NULL;
		parent->m_childListEnd = newDev;
	}
	return newDev;
}

void freeDevice(UsbdDevice_t *dev)
{
	if ( !dev || dev < memPool->m_deviceTreeBuf || dev >= memPool->m_deviceTreeBuf + usbConfig.m_maxDevices )
	{
		dbg_printf("freeDevice %p: Arg is not part of dev buffer\n", dev);
		return;
	}
	dev->m_prev = memPool->m_freeDeviceListEnd;
	*( memPool->m_freeDeviceListEnd ? &(memPool->m_freeDeviceListEnd->m_next) : &(memPool->m_freeDeviceListStart) ) = dev;
	dev->m_next = NULL;
	memPool->m_freeDeviceListEnd = dev;
	dev->m_parent = NULL;
}

UsbdIoRequest_t *allocIoRequest(void)
{
	UsbdIoRequest_t *res;

	res = memPool->m_freeIoReqList;
	if ( !res )
	{
		dbg_printf("ran out of IoReqs\n");
		return NULL;
	}
	*( res->m_next ? &(res->m_next->m_prev) : &(memPool->m_freeIoReqListEnd) ) = res->m_prev;
	*( res->m_prev ? &(res->m_prev->m_next) : &(memPool->m_freeIoReqList) ) = res->m_next;
	return res;
}

void freeIoRequest(UsbdIoRequest_t *req)
{
	UsbdIoRequest_t *pos;

	if ( !req )
	{
		return;
	}
	if ( req < memPool->m_ioReqBufPtr || req >= memPool->m_ioReqBufPtr + usbConfig.m_maxIoReqs )
	{
		req->m_busyFlag = 0;
		return;
	}
	for ( pos = memPool->m_freeIoReqList; pos && pos != req; pos = pos->m_next )
	{
	}
	if ( pos )
	{
		dbg_printf("freeIoRequest %p: already free.\n", req);
		return;
	}
	req->m_busyFlag = 0;
	req->m_prev = memPool->m_freeIoReqListEnd;
	*( memPool->m_freeIoReqListEnd ? &(memPool->m_freeIoReqListEnd->m_next) : &(memPool->m_freeIoReqList) ) = req;
	req->m_next = NULL;
	memPool->m_freeIoReqListEnd = req;
}

UsbdEndpoint_t *allocEndpointForDevice(UsbdDevice_t *dev, u32 align)
{
	UsbdEndpoint_t *newEp;

	newEp = memPool->m_freeEpListStart;
	if ( !newEp )
	{
		return NULL;
	}
	*( newEp->m_next ? &(newEp->m_next->m_prev) : &(memPool->m_freeEpListEnd) ) = newEp->m_prev;
	*( newEp->m_prev ? &(newEp->m_prev->m_next) : &(memPool->m_freeEpListStart) ) = newEp->m_next;
	newEp->m_correspDevice = dev;
	newEp->m_ioReqListEnd = NULL;
	newEp->m_ioReqListStart = NULL;
	newEp->m_busyPrev = NULL;
	newEp->m_busyNext = NULL;
	newEp->m_inTdQueue = 0;
	newEp->m_alignFlag = align;
	newEp->m_prev = dev->m_endpointListEnd;
	*( dev->m_endpointListEnd ? &(dev->m_endpointListEnd->m_next) : &(dev->m_endpointListStart) ) = newEp;
	newEp->m_next = NULL;
	dev->m_endpointListEnd = newEp;
	return newEp;
}

int cleanUpFunc(UsbdDevice_t *dev, UsbdEndpoint_t *ep)
{
	if ( !ep || ep < memPool->m_endpointBuf || ep >= memPool->m_endpointBuf + usbConfig.m_maxEndpoints )
	{
		return 0;
	}
	if ( ep->m_inTdQueue != NOTIN_QUEUE )
	{
		*( ep->m_busyNext ? &(ep->m_busyNext->m_busyPrev) : &(memPool->m_tdQueueEnd) ) = ep->m_busyPrev;
		*( ep->m_busyPrev ? &(ep->m_busyPrev->m_busyNext) : &(memPool->m_tdQueueStart) ) = ep->m_busyNext;
		ep->m_inTdQueue = NOTIN_QUEUE;
	}
	*( ep->m_next ? &(ep->m_next->m_prev) : &(dev->m_endpointListEnd) ) = ep->m_prev;
	*( ep->m_prev ? &(ep->m_prev->m_next) : &(dev->m_endpointListStart) ) = ep->m_next;
	ep->m_prev = memPool->m_freeEpListEnd;
	*( memPool->m_freeEpListEnd ? &(memPool->m_freeEpListEnd->m_next) : &(memPool->m_freeEpListStart) ) = ep;
	ep->m_next = NULL;
	memPool->m_freeEpListEnd = ep;
	ep->m_correspDevice = NULL;
	return 0;
}

UsbdHcTD_t *allocTd(void)
{
	UsbdHcTD_t *res;

	res = memPool->m_freeHcTdList;
	if ( !res )
	{
		return NULL;
	}
	memPool->m_freeHcTdList = res->m_next;
	res->m_next = NULL;
	return res;
}

void freeTd(UsbdHcTD_t *argTd)
{
	UsbdHcTD_t *pos;

	if ( !argTd )
	{
		return;
	}
	for ( pos = memPool->m_freeHcTdList; pos && argTd != pos; pos = pos->m_next )
	{
	}
	if ( pos )
	{
		dbg_printf("FreeTD %p: already free\n", argTd);
		return;
	}
	argTd->m_next = memPool->m_freeHcTdList;
	memPool->m_freeHcTdList = argTd;
}

UsbdHcIsoTD_t *allocIsoTd(void)
{
	UsbdHcIsoTD_t *newTd;

	newTd = memPool->m_freeHcIsoTdList;
	if ( !newTd )
	{
		return NULL;
	}
	memPool->m_freeHcIsoTdList = newTd->m_next;
	newTd->m_next = NULL;
	return newTd;
}

void freeIsoTd(UsbdHcIsoTD_t *argTd)
{
	UsbdHcIsoTD_t *pos;

	if ( !argTd )
	{
		return;
	}
	for ( pos = memPool->m_freeHcIsoTdList; pos && argTd != pos; pos = pos->m_next )
	{
	}
	if ( pos )
	{
		dbg_printf("freeIsoTd %p: already free\n", argTd);
		return;
	}
	argTd->m_next = memPool->m_freeHcIsoTdList;
	memPool->m_freeHcIsoTdList = argTd;
}
