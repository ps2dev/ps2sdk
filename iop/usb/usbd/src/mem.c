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

#include "stdio.h"

MemoryPool memPool;

HcIsoTD *allocIsoTd(void) {
	HcIsoTD *newTd = memPool.freeHcIsoTdList;
	if (newTd) {
		memPool.freeHcIsoTdList = newTd->next;
		newTd->next = NULL;
	}
	return newTd;
}

void freeIsoTd(HcIsoTD *argTd) {
	HcIsoTD *pos;
	if (argTd) {
		for (pos = memPool.freeHcIsoTdList; pos != NULL; pos = pos->next)
			if (pos == argTd) {
				printf("freeIsoTd %p: already free\n", argTd);
				return;
			}
		argTd->next = memPool.freeHcIsoTdList;
		memPool.freeHcIsoTdList = argTd;
	}
}

HcTD *allocTd(void) {
	HcTD *res = memPool.freeHcTdList;
	if (res) {
		memPool.freeHcTdList = res->next;
		res->next = NULL;
	}
	return res;
}

void freeTd(HcTD *argTd) {
	HcTD *pos;
	if (argTd) {
		for (pos = memPool.freeHcTdList; pos != NULL; pos = pos->next)
			if (pos == argTd) {
				printf("FreeTD %p: already free\n", argTd);
				return;
			}
		argTd->next = memPool.freeHcTdList;
		memPool.freeHcTdList = argTd;
	}
}

Device *attachChildDevice(Device *parent, uint32 portNum) {
	Device *newDev = memPool.freeDeviceListStart;
	if (!newDev) {
		dbg_printf("Ran out of device handles\n");
		return NULL;
	}

	if (newDev->next)
		newDev->next->prev = newDev->prev;
	else
		memPool.freeDeviceListEnd = newDev->prev;

	if (newDev->prev)
		newDev->prev->next = newDev->next;
	else
		memPool.freeDeviceListStart = newDev->next;

	newDev->endpointListEnd = newDev->endpointListStart = NULL;
	newDev->devDriver = NULL;
	newDev->deviceStatus = DEVICE_NOTCONNECTED;
	newDev->resetFlag = 0;
	newDev->childListEnd = newDev->childListStart = NULL;
	newDev->parent = parent;
	newDev->attachedToPortNo = portNum;
	newDev->privDataField = NULL;
	if (parent) {
		newDev->prev = parent->childListEnd;
		if (parent->childListEnd)
			parent->childListEnd->next = newDev;
		else
			parent->childListStart = newDev;
		newDev->next = NULL;
		parent->childListEnd = newDev;
	} else
		newDev->next = newDev->prev = NULL;
    return newDev;
}

void freeDevice(Device *dev) {
	if (!dev)
		return;

	if ((dev < memPool.deviceTreeBuf) || (dev >= memPool.deviceTreeBuf + usbConfig.maxDevices)) {
		printf("freeDevice %p: Arg is not part of dev buffer\n", dev);
		return;
	}

	dev->prev = memPool.freeDeviceListEnd;
	if (memPool.freeDeviceListEnd)
		memPool.freeDeviceListEnd->next = dev;
	else
		memPool.freeDeviceListStart = dev;

	dev->next = NULL;
	dev->parent = NULL;
	memPool.freeDeviceListEnd = dev;
}

Device *fetchPortElemByNumber(Device *hub, int port) {
	Device *res = hub->childListStart;
	while (--port > 0) {
		if (!res)
			return NULL;
		res = res->next;
	}
	return res;
}

void addToHcEndpointList(uint8 type, HcED *ed) {
	ed->next = memPool.hcEdBuf[type].next;
	memPool.hcEdBuf[type].next = ed;
}

void removeHcEdFromList(int type, HcED *hcEd) {
	HcED *prev = memPool.hcEdBuf + type;
	HcED *pos  = prev->next;
	while (pos) {
		if (pos == hcEd) {
			prev->next = pos->next;
			return;
		}
		prev = pos;
		pos = pos->next;
	}
}

Endpoint *allocEndpointForDevice(Device *dev, uint32 align) {
	Endpoint *newEp = memPool.freeEpListStart;
	if (!newEp)
		return NULL;

	if (newEp->next)
		newEp->next->prev = newEp->prev;
	else
		memPool.freeEpListEnd = newEp->prev;

	if (newEp->prev)
		newEp->prev->next = newEp->next;
	else
		memPool.freeEpListStart = newEp->next;

	newEp->correspDevice = dev;
	newEp->ioReqListStart = newEp->ioReqListEnd = NULL;
	newEp->busyNext = newEp->busyPrev = NULL;
	newEp->inTdQueue = 0;
	newEp->alignFlag = align;

	newEp->next = NULL;
	newEp->prev = dev->endpointListEnd;
	if (dev->endpointListEnd)
		dev->endpointListEnd->next = newEp;
	else
		dev->endpointListStart = newEp;

	dev->endpointListEnd = newEp;
	return newEp;
}

Device *fetchDeviceById(int devId) {
	Device *dev;
	if ((devId > 0) && (devId < usbConfig.maxDevices)) {
		dev = memPool.deviceTreeBuf + devId;
		if (dev->parent)
			return dev;
	}
	return NULL;
}

Endpoint *fetchEndpointById(int id) {
	Endpoint *res;
	if ((id >= 0) && (id < usbConfig.maxEndpoints)) {
		res = memPool.endpointBuf + id;
		if (res->correspDevice)
			return res;
	}
	return NULL;
}

IoRequest *allocIoRequest(void) {
	IoRequest *res = memPool.freeIoReqList;
	if (res) {
		if (res->next)
			res->next->prev = res->prev;
		else
			memPool.freeIoReqListEnd = res->prev;

		if (res->prev)
			res->prev->next = res->next;
		else
			memPool.freeIoReqList = res->next;
		res->prev = res->next = NULL;
	} else
		dbg_printf("ran out of IoReqs\n");
	return res;
}

void freeIoRequest(IoRequest *req) {
	int num = req - memPool.ioReqBufPtr;
	IoRequest *pos;
	if (req) {
		if ((num >= 0) && (num < usbConfig.maxIoReqs)) {
			for (pos = memPool.freeIoReqList; pos != NULL; pos = pos->next)
				if (pos == req) {
					printf("freeIoRequest %p: already free.\n", req);
					return;
				}
			req->prev = memPool.freeIoReqListEnd;
			if (memPool.freeIoReqListEnd)
				memPool.freeIoReqListEnd->next = req;
			else
				memPool.freeIoReqList = req;
			req->next = NULL;
			memPool.freeIoReqListEnd = req;
		}
		req->busyFlag = 0;
	}
}

