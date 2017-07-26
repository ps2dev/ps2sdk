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

#include "usbdpriv.h"
#include "driver.h"
#include "mem.h"
#include "hcd.h"
#include "usbio.h"

int sceUsbdRegisterLdd(sceUsbdLddOps *driver) {
	int res;

	if (usbdLock() != 0)
		return USB_RC_BADCONTEXT;

	res = doRegisterDriver(driver, NULL); // gp

    usbdUnlock();
	return res;
}

int sceUsbdUnregisterLdd(sceUsbdLddOps *driver) {
	int res;

	if (usbdLock() != 0)
		return USB_RC_BADCONTEXT;

	res = doUnregisterDriver(driver);

	usbdUnlock();
	return res;
}

void *sceUsbdScanStaticDescriptor(int devId, void *data, u8 type) {
	void *res;

	if (usbdLock() != 0)
		return NULL;

	res = doGetDeviceStaticDescriptor(devId, data, type);

	usbdUnlock();
	return res;
}

int sceUsbdGetDeviceLocation(int devId, u8 *path) {
	Device *dev;
	int res;

	if (usbdLock() != 0)
		return USB_RC_BADCONTEXT;

	dev = fetchDeviceById(devId);
	if (dev)
		res = doGetDeviceLocation(dev, path);
	else
		res = USB_RC_BADDEV;

	usbdUnlock();
	return res;
}

int UsbSetPrivateData(int devId, void *data) {
	Device *dev;
	int res = USB_RC_OK;

	if (usbdLock() != 0)
		return USB_RC_BADCONTEXT;

	dev = fetchDeviceById(devId);
	if (dev)
		dev->privDataField = data;
	else
		res = USB_RC_BADDEV;

	usbdUnlock();
	return res;
}

void *UsbGetPrivateData(int devId) {
	Device *dev;
	void *res = NULL;

	if (usbdLock() != 0)
		return NULL;

	dev = fetchDeviceById(devId);
	if (dev)
		res = dev->privDataField;

	usbdUnlock();
	return res;
}

int sceUsbdOpenPipe(int devId, UsbEndpointDescriptor *desc) {
	Device *dev;
	Endpoint *ep;
	int res = -1;

	if (usbdLock() != 0)
		return -1;

	dev = fetchDeviceById(devId);
	if (dev) {
		ep = doOpenEndpoint(dev, desc, 0);
		if (ep)
			res = ep->id;
	}

	usbdUnlock();
	return res;
}

int sceUsbdClosePipe(int id) {
	Endpoint *ep;
	int res;

	if (usbdLock() != 0)
		return -1;

	ep = fetchEndpointById(id);
	if (ep)
		res = doCloseEndpoint(ep);
	else
		res = USB_RC_BADPIPE;

	usbdUnlock();
	return res;
}

int sceUsbdTransferPipe(int id, void *data, u32 len, void *option, sceUsbdDoneCallback callback, void *cbArg) {
	UsbDeviceRequest *ctrlPkt = (UsbDeviceRequest *)option;
	IoRequest *req;
	Endpoint *ep;
	int res = 0;

	if (usbdLock() != 0)
		return USB_RC_BADCONTEXT;

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
		req->gpSeg = NULL; // gp of the calling module
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
	return res;
}

int sceUsbdOpenPipeAligned(int devId, UsbEndpointDescriptor *desc) {
	Device *dev;
	Endpoint *ep;
	int res = -1;

	if (usbdLock() != 0)
		return -1;

	dev = fetchDeviceById(devId);
	if (dev) {
		ep = doOpenEndpoint(dev, desc, 1);
		if (ep)
			res = ep->id;
	}

	usbdUnlock();
	return res;
}

int UsbRegisterAutoloader(sceUsbdLddOps *drv) {
	dbg_printf("UsbRegisterAutoloader stub\n");
	return 0;
}

int UsbUnregisterAutoloader(sceUsbdLddOps *drv) {
	dbg_printf("UsbUnregisterAutoloader stub\n");
	return 0;
}

int sceUsbdChangeThreadPriority(int prio1, int prio2) {
	dbg_printf("sceUsbdChangeThreadPriority stub\n");
	return 0;
}

