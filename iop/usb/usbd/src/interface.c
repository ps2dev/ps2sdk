/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * USB Driver function prototypes and constants.
 */

#include <defs.h>

#include "usbdpriv.h"
#include "driver.h"
#include "mem.h"
#include "hcd.h"
#include "usbio.h"

int sceUsbdRegisterLdd(sceUsbdLddOps *driver) {
	int res;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return USB_RC_BADCONTEXT;
	}

	res = doRegisterDriver(driver, OldGP);

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int sceUsbdUnregisterLdd(sceUsbdLddOps *driver) {
	int res;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return USB_RC_BADCONTEXT;
	}

	res = doUnregisterDriver(driver);

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

void *sceUsbdScanStaticDescriptor(int devId, void *data, u8 type) {
	void *res;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return NULL;
	}

	res = doGetDeviceStaticDescriptor(devId, data, type);

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int sceUsbdGetDeviceLocation(int devId, u8 *path) {
	Device *dev;
	int res;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return USB_RC_BADCONTEXT;
	}

	dev = fetchDeviceById(devId);
	if (dev)
		res = doGetDeviceLocation(dev, path);
	else
		res = USB_RC_BADDEV;

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int UsbSetPrivateData(int devId, void *data) {
	Device *dev;
	int res = USB_RC_OK;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return USB_RC_BADCONTEXT;
	}

	dev = fetchDeviceById(devId);
	if (dev)
		dev->privDataField = data;
	else
		res = USB_RC_BADDEV;

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

void *UsbGetPrivateData(int devId) {
	Device *dev;
	void *res = NULL;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return NULL;
	}

	dev = fetchDeviceById(devId);
	if (dev)
		res = dev->privDataField;

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int sceUsbdOpenPipe(int devId, UsbEndpointDescriptor *desc) {
	Device *dev;
	Endpoint *ep;
	int res = -1;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return -1;
	}

	dev = fetchDeviceById(devId);
	if (dev) {
		ep = doOpenEndpoint(dev, desc, 0);
		if (ep)
			res = ep->id;
	}

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int sceUsbdClosePipe(int id) {
	Endpoint *ep;
	int res;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return -1;
	}

	ep = fetchEndpointById(id);
	if (ep)
		res = doCloseEndpoint(ep);
	else
		res = USB_RC_BADPIPE;

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int sceUsbdTransferPipe(int id, void *data, u32 len, void *option, sceUsbdDoneCallback callback, void *cbArg) {
	UsbDeviceRequest *ctrlPkt = (UsbDeviceRequest *)option;
	IoRequest *req;
	Endpoint *ep;
	int res = 0;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return USB_RC_BADCONTEXT;
	}

	ep = fetchEndpointById(id);
	if (!ep) {
		dbg_printf("sceUsbdTransferPipe: Endpoint %d not found\n", id);
		res = USB_RC_BADPIPE;
	}

	if ((res == 0) && data && len) {
		if ((((u32)(data + len - 1) >> 12) - ((u32)data >> 12)) > 1)
			res = USB_RC_BADLENGTH;
		else if (ep->alignFlag && ((u32)data & 3))
			res = USB_RC_BADALIGN;
		else if ((ep->endpointType == TYPE_ISOCHRON) && ((ep->hcEd.maxPacketSize & 0x7FF) < len))
			res = USB_RC_BADLENGTH;
	}
	if (res == 0) {
		req = allocIoRequest();
		if (!req) {
			dbg_printf("Ran out of IoReqs\n");
			res = USB_RC_IOREQ;
		}
	}
	if (res == 0) {
		req->userCallbackProc = callback;
		req->userCallbackArg = cbArg;
		req->gpSeg = GetGP(); // gp of the calling module
		if (ep->endpointType == TYPE_CONTROL) {
			if (!ctrlPkt) {
				res = USB_RC_BADOPTION;
				freeIoRequest(req);
			} else if (ctrlPkt->length != len) {
				res = USB_RC_BADLENGTH;
				freeIoRequest(req);
			} else {
				res = doControlTransfer(ep, req,
					ctrlPkt->requesttype, ctrlPkt->request, ctrlPkt->value, ctrlPkt->index, ctrlPkt->length,
					data, signalCallbackThreadFunc);
			}
		} else {
			if (ep->endpointType == TYPE_ISOCHRON)
				req->waitFrames = (u32)option;
			res = attachIoReqToEndpoint(ep, req, data, len, signalCallbackThreadFunc);
		}
	}

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int sceUsbdOpenPipeAligned(int devId, UsbEndpointDescriptor *desc) {
	Device *dev;
	Endpoint *ep;
	int res = -1;
	void *OldGP;

	OldGP = SetModuleGP();
	if (usbdLock() != 0)
	{
		SetGP(OldGP);
		return -1;
	}

	dev = fetchDeviceById(devId);
	if (dev) {
		ep = doOpenEndpoint(dev, desc, 1);
		if (ep)
			res = ep->id;
	}

	usbdUnlock();
	SetGP(OldGP);

	return res;
}

int UsbRegisterAutoloader(sceUsbdLddOps *drv) {
	void *OldGP;

	OldGP = SetModuleGP();

	dbg_printf("UsbRegisterAutoloader stub\n");

	SetGP(OldGP);

	return 0;
}

int UsbUnregisterAutoloader(sceUsbdLddOps *drv) {
	void *OldGP;

	OldGP = SetModuleGP();

	dbg_printf("UsbUnregisterAutoloader stub\n");

	SetGP(OldGP);

	return 0;
}

int sceUsbdChangeThreadPriority(int prio1, int prio2) {
	void *OldGP;

	OldGP = SetModuleGP();

	dbg_printf("sceUsbdChangeThreadPriority stub\n");

	SetGP(OldGP);

	return 0;
}

