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

int sceUsbdRegisterLdd(sceUsbdLddOps *driver)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = doRegisterDriver(driver, OldGP);
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdRegisterAutoloader(sceUsbdLddOps *drv)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = doRegisterAutoLoader(drv, OldGP);
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdUnregisterLdd(sceUsbdLddOps *driver)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = doUnregisterDriver(driver);
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdUnregisterAutoloader(void)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = doUnregisterAutoLoader();
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

void *sceUsbdScanStaticDescriptor(int devId, void *data, u8 type)
{
	void *OldGP;
	const UsbdDevice_t *dev;
	void *res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = NULL;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	dev = fetchDeviceById(devId);
	if ( dev && dev->m_deviceStatus == DEVICE_READY )
		res = doGetDeviceStaticDescriptor(devId, data, type);
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdGetDeviceLocation(int devId, u8 *path)
{
	void *OldGP;
	int res;
	UsbdDevice_t *dev;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = USB_RC_BADDEV;
	dev = fetchDeviceById(devId);
	if ( dev && dev->m_deviceStatus == DEVICE_READY )
		res = doGetDeviceLocation(dev, path);
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdSetPrivateData(int devId, void *data)
{
	void *OldGP;
	int res;
	UsbdDevice_t *dev;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = USB_RC_BADDEV;
	dev = fetchDeviceById(devId);
	if ( dev )
	{
		dev->m_privDataField = data;
		res = USB_RC_OK;
	}
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

void *sceUsbdGetPrivateData(int devId)
{
	void *OldGP;
	void *res;
	UsbdDevice_t *dev;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = NULL;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	dev = fetchDeviceById(devId);
	if ( dev )
		res = dev->m_privDataField;
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdOpenPipe(int devId, UsbEndpointDescriptor *desc)
{
	void *OldGP;
	UsbdDevice_t *dev;
	int res;
	const UsbdEndpoint_t *ep;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = -1;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	ep = NULL;
	dev = fetchDeviceById(devId);
	if ( dev )
		ep = doOpenEndpoint(dev, desc, 0);
	if ( ep )
		res = ep->m_id;
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdOpenPipeAligned(int devId, UsbEndpointDescriptor *desc)
{
	void *OldGP;
	UsbdDevice_t *dev;
	int res;
	const UsbdEndpoint_t *ep;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = -1;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	ep = NULL;
	dev = fetchDeviceById(devId);
	if ( dev )
		ep = doOpenEndpoint(dev, desc, 1u);
	if ( ep )
		res = ep->m_id;
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdClosePipe(int id)
{
	void *OldGP;
	UsbdEndpoint_t *ep;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = USB_RC_BADPIPE;
	ep = fetchEndpointById(id);
	if ( ep )
		res = doCloseEndpoint(ep);
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

static void signalCallbackThreadFunc(UsbdIoRequest_t *req)
{
	int state;

	CpuSuspendIntr(&state);
	req->m_prev = cbListEnd;
	if ( !cbListEnd )
		cbListStart = req;
	else
		cbListEnd->m_next = req;
	req->m_next = NULL;
	cbListEnd = req;
	CpuResumeIntr(state);
	SetEventFlag(usbKernelResources.m_callbackEvent, 1u);
}

static int usbdTransferPipeImpl(
	void *gp_val,
	int id,
	void *data,
	u32 length,
	UsbDeviceRequest *option,
	void *callback,
	void *cbArg,
	sceUsbdMultiIsochronousRequest *request)
{
	int res;
	UsbdEndpoint_t *ep;
	int bNumPackets;
	UsbdIoRequest_t *req;

	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
		return res;
	}
	res = USB_RC_OK;
	ep = fetchEndpointById(id);
	if ( !ep )
	{
		dbg_printf("sceUsbdTransferPipe: UsbdEndpoint_t %d not found\n", id);
		res = USB_RC_BADPIPE;
	}
	bNumPackets = 0;
	if ( request && res == USB_RC_OK )
	{
		bNumPackets = request->bNumPackets;
		data = request->bBufStart;
	}
	if ( request && res == USB_RC_OK )
	{
		if ( (unsigned int)(bNumPackets - 1) >= 8 || !data )
		{
			res = USB_RC_BADLENGTH;
		}
	}
	if ( (request || (data && (int)length > 0)) && res == USB_RC_OK )
	{
		if ( ep->m_alignFlag && ((uiptr)data & 3) != 0 )
		{
			res = USB_RC_BADALIGN;
		}
	}
	if ( request && res == USB_RC_OK )
	{
		if ( ep->m_endpointType != TYPE_ISOCHRON )
		{
			res = USB_RC_BADPIPE;
		}
	}
	if ( request && res == USB_RC_OK )
	{
		int i_pkt;

		length = 0;
		for ( i_pkt = 0; i_pkt < bNumPackets; i_pkt += 1 )
		{
			if ( (ep->m_hcEd->m_hcArea.stru.m_maxPacketSize & 0x7FFu) < request->Packets[i_pkt].bLength )
			{
				res = USB_RC_BADLENGTH;
				break;
			}
			length += request->Packets[i_pkt].bLength;
			if ( ep->m_alignFlag && (((((uiptr)data) & 0xFF) + (length & 0xFF)) & 3) != 0 )
			{
				res = USB_RC_BADALIGN;
				break;
			}
		}
	}
	if ( (request || (data && (int)length > 0)) && res == USB_RC_OK )
	{
		if ( (((uiptr)data + length - 1) >> 12) - ((uiptr)data >> 12) >= 2 )
		{
			res = USB_RC_BADLENGTH;
		}
	}
	if ( (!request && (data && (int)length > 0)) && res == USB_RC_OK )
	{
		switch ( ep->m_endpointType )
		{
			case TYPE_CONTROL:
			{
				if ( ((uiptr)data & 3) != 0 && (option->requesttype & 0x80) == 0 )
				{
					if ( (ep->m_hcEd->m_hcArea.stru.m_maxPacketSize & 0x7FF) == 64 && (int)length >= 63 )
					{
						res = USB_RC_BADALIGN;
					}
				}
				break;
			}
			case TYPE_ISOCHRON:
			{
				if ( (ep->m_hcEd->m_hcArea.stru.m_maxPacketSize & 0x7FFu) < length )
				{
					res = USB_RC_BADLENGTH;
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	if ( res == USB_RC_OK )
	{
		req = allocIoRequest();
		if ( !req )
		{
			dbg_printf("Ran out of IoReqs\n");
			res = USB_RC_IOREQ;
		}
	}
	if ( res == USB_RC_OK )
	{
		req->m_userCallbackArg = cbArg;
		req->m_gpSeg = gp_val;
		req->m_req.bNumPackets = 0;
		req->m_userCallbackProc = callback;
		switch ( ep->m_endpointType )
		{
			case TYPE_ISOCHRON:
			{
				if ( request )
				{
					req->m_waitFrames = request->bRelStartFrame;
					memcpy(&(req->m_req), request, sizeof(sceUsbdMultiIsochronousRequest));
				}
				else
				{
					req->m_waitFrames = (u32)option;
				}
				res = attachIoReqToEndpoint(ep, req, data, length, signalCallbackThreadFunc);
				break;
			}
			case TYPE_CONTROL:
			{
				if ( !option )
				{
					freeIoRequest(req);
					res = USB_RC_BADOPTION;
				}
				if ( res == USB_RC_OK )
				{
					if ( option->length != length )
					{
						freeIoRequest(req);
						res = USB_RC_BADLENGTH;
					}
				}
				if ( res == USB_RC_OK )
				{
					res = doControlTransfer(
						ep,
						req,
						option->requesttype,
						option->request,
						option->value,
						option->index,
						option->length,
						data,
						signalCallbackThreadFunc);
				}
				break;
			}
			default:
			{
				res = attachIoReqToEndpoint(ep, req, data, length, signalCallbackThreadFunc);
				break;
			}
		}
	}
	usbdUnlock();
	return res;
}

int sceUsbdTransferPipe(int id, void *data, u32 len, void *option, sceUsbdDoneCallback callback, void *cbArg)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = usbdTransferPipeImpl(OldGP, id, data, len, (UsbDeviceRequest *)option, callback, cbArg, NULL);
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdMultiIsochronousTransfer(
	int pipeId, sceUsbdMultiIsochronousRequest *request, sceUsbdMultiIsochronousDoneCallback callback, void *cbArg)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = usbdTransferPipeImpl(OldGP, pipeId, NULL, 0, NULL, callback, cbArg, request);
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdChangeThreadPriority(int prio1, int prio2)
{
	void *OldGP;
	int res;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = 0;
	if ( usbConfig.m_hcdThreadPrio != prio1 )
	{
		usbConfig.m_hcdThreadPrio = prio1;
		res = ChangeThreadPriority(usbKernelResources.m_hcdTid, prio1);
	}
	if ( res == 0 && usbConfig.m_cbThreadPrio != prio2 )
	{
		usbConfig.m_cbThreadPrio = prio2;
		res = ChangeThreadPriority(usbKernelResources.m_callbackTid, prio2);
	}
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}

int sceUsbdGetReportDescriptor(int devId, int cfgNum, int ifNum, void **desc, u32 *len)
{
	void *OldGP;
	int res;
	UsbdDevice_t *dev;
	UsbdReportDescriptor_t *hidDescriptorStart;

#if USE_GP_REGISTER
	OldGP = SetModuleGP();
#else
	OldGP = NULL;
#endif
	res = USB_RC_BADCONTEXT;
	if ( usbdLock() )
	{
#if USE_GP_REGISTER
		SetGP(OldGP);
#endif
		return res;
	}
	res = USB_RC_BADDEV;
	hidDescriptorStart = NULL;
	dev = fetchDeviceById(devId);
	if ( dev )
	{
		res = USB_RC_UNKNOWN;
		for ( hidDescriptorStart = dev->m_reportDescriptorStart;
					hidDescriptorStart
					&& ((int)hidDescriptorStart->m_cfgNum != cfgNum || (int)hidDescriptorStart->m_ifNum != ifNum);
					hidDescriptorStart = hidDescriptorStart->m_next )
		{
		}
	}
	if ( hidDescriptorStart )
	{
		res = USB_RC_OK;
		if ( desc )
			*desc = hidDescriptorStart->m_data;
		if ( len )
			*len = hidDescriptorStart->m_length;
	}
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(OldGP);
#endif
	return res;
}
