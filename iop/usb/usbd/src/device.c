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

static void fetchNextReportDescriptor(UsbdIoRequest_t *req);
static void requestDeviceDescriptor(UsbdIoRequest_t *req);
static void hubSetFuncAddress(UsbdEndpoint_t *ep);

static const char *usbdVersionString = "Version 1.6.0";

void *doGetDeviceStaticDescriptor(int devId, void *data, u8 type)
{
	UsbdDevice_t *dev;
	UsbDeviceDescriptor *descBuf;

	dev = fetchDeviceById(devId);
	if ( !dev )
	{
		return NULL;
	}
	descBuf = data ? (UsbDeviceDescriptor *)((u8 *)data + ((UsbDeviceDescriptor *)data)->bLength) :
									 (UsbDeviceDescriptor *)dev->m_staticDeviceDescPtr;
	for ( ; descBuf->bLength >= 2u && (u8 *)descBuf < (u8 *)dev->m_staticDeviceDescEndPtr;
				descBuf = (UsbDeviceDescriptor *)((u8 *)descBuf + descBuf->bLength) )
	{
		if ( !type || descBuf->bDescriptorType == type )
			return descBuf;
	}
	return NULL;
}

int doGetDeviceLocation(UsbdDevice_t *dev, u8 *path)
{
	const UsbdDevice_t *deviceTreeRoot;
	int count;
	int cpCount;
	u8 tmpPath[16];

	deviceTreeRoot = getDeviceTreeRoot();
	for ( count = 0; count < 6 && dev != deviceTreeRoot; count += 1, dev = dev->m_parent )
	{
		tmpPath[count] = dev->m_attachedToPortNo;
	}
	if ( dev != deviceTreeRoot )
		return USB_RC_BADHUBDEPTH;
	if ( count >= 6 )
		return USB_RC_BADHUBDEPTH;
	for ( cpCount = 0; cpCount < 7; cpCount += 1 )
	{
		path[cpCount] = (cpCount < count) ? tmpPath[count - cpCount - 1] : 0;
	}
	return USB_RC_OK;
}

UsbdEndpoint_t *doOpenEndpoint(UsbdDevice_t *dev, const UsbEndpointDescriptor *endpDesc, u32 alignFlag)
{
	if ( !dev->m_parent )
	{
		return NULL;
	}
	if ( !endpDesc )
		return dev->m_endpointListStart;  // default control EP was already opened
	return openDeviceEndpoint(dev, endpDesc, alignFlag);
}

int doCloseEndpoint(UsbdEndpoint_t *ep)
{
	if ( ep->m_correspDevice->m_endpointListStart == ep )
		return 0;
	return removeEndpointFromDevice(ep->m_correspDevice, ep);
}

int attachIoReqToEndpoint(UsbdEndpoint_t *ep, UsbdIoRequest_t *req, void *destdata, u16 length, void *callback)
{
	if ( !ep->m_correspDevice )
	{
		return USB_RC_BUSY;
	}
	if ( req->m_busyFlag )
	{
		return USB_RC_BUSY;
	}
	req->m_busyFlag = 1;
	req->m_correspEndpoint = ep;
	req->m_destPtr = destdata;
	req->m_length = length;
	req->m_resultCode = USB_RC_OK;
	req->m_callbackProc = (InternCallback)callback;
	req->m_prev = ep->m_ioReqListEnd;
	if ( ep->m_ioReqListEnd )
		ep->m_ioReqListEnd->m_next = req;
	else
		ep->m_ioReqListStart = req;
	req->m_next = NULL;
	ep->m_ioReqListEnd = req;
	handleIoReqList(ep);
	return USB_RC_OK;
}

int doControlTransfer(
	UsbdEndpoint_t *ep,
	UsbdIoRequest_t *req,
	u8 requestType,
	u8 request,
	u16 value,
	u16 index,
	u16 length,
	void *destdata,
	void *callback)
{
	if ( req->m_busyFlag )
	{
		dbg_printf("ERROR: doControlTransfer: IoReq busy\n");
		return USB_RC_BUSY;
	}
	req->m_devReq.requesttype = requestType;
	req->m_devReq.request = request;
	req->m_devReq.value = value;
	req->m_devReq.index = index;
	req->m_devReq.length = length;
	return attachIoReqToEndpoint(ep, req, destdata, length, callback);
}

static void connectNewDevice(UsbdDevice_t *dev)
{
	sceUsbdLddOps *drv;

	dbg_printf("searching driver for dev %d, FA %02X\n", dev->m_id, dev->m_functionAddress);
	dev->m_deviceStatus = DEVICE_READY;
	for ( drv = drvListStart; drv; drv = drv->next )
	{
		dev->m_privDataField = NULL;
		if ( callUsbDriverFunc(drv->probe, dev->m_id, drv->gp) )
		{
			dev->m_devDriver = drv;
			dbg_printf("Driver found (%s)\n", drv->name);
			callUsbDriverFunc(drv->connect, dev->m_id, drv->gp);
			return;
		}
	}
	// No driver found yet. Call autoloader.
	drv = drvAutoLoader;
	if ( drv )
	{
		dev->m_privDataField = NULL;
		if ( callUsbDriverFunc(drv->probe, dev->m_id, drv->gp) )
		{
			dev->m_devDriver = drv;
			dbg_printf("(autoloader) Driver found (%s)\n", drv->name);
			callUsbDriverFunc(drv->connect, dev->m_id, drv->gp);
			return;
		}
	}
	dbg_printf("no driver found\n");
}

static void fetchNextReportDescriptorCB(UsbdIoRequest_t *req)
{
	req->m_correspEndpoint->m_correspDevice->m_reportDescriptorCurForFetch =
		req->m_correspEndpoint->m_correspDevice->m_reportDescriptorCurForFetch->m_next;
	if ( req->m_correspEndpoint->m_correspDevice->m_reportDescriptorCurForFetch )
		fetchNextReportDescriptor(req);
	else
		connectNewDevice(req->m_correspEndpoint->m_correspDevice);
}

static void fetchNextReportDescriptor(UsbdIoRequest_t *req)
{
	doControlTransfer(
		req->m_correspEndpoint,
		&req->m_correspEndpoint->m_correspDevice->m_ioRequest,
		USB_DIR_IN | USB_RECIP_INTERFACE,
		6u,
		0x2200u,
		req->m_correspEndpoint->m_correspDevice->m_reportDescriptorCurForFetch->m_ifNum,
		req->m_correspEndpoint->m_correspDevice->m_reportDescriptorCurForFetch->m_length,
		req->m_correspEndpoint->m_correspDevice->m_reportDescriptorCurForFetch->m_data,
		fetchNextReportDescriptorCB);
}

static void killDevice(UsbdDevice_t *dev, UsbdEndpoint_t *ep)
{
	removeEndpointFromDevice(dev, ep);
	checkDelayedResets(dev);
	hubResetDevice(dev);
}

static void fetchConfigDescriptors(UsbdIoRequest_t *req)
{
	UsbdDevice_t *dev;
	const UsbDeviceDescriptor *staticDeviceDescPtr;
	int fetchDescriptorCounter;
	u32 fetchDesc_1;
	int fetchDesc_2;
	int curDescNum_1;
	UsbConfigDescriptor *destdata;
	int readLen;

	dev = req->m_correspEndpoint->m_correspDevice;
	staticDeviceDescPtr = (UsbDeviceDescriptor *)dev->m_staticDeviceDescPtr;
	if ( (int)dev->m_fetchDescriptorCounter > 0 && req->m_resultCode != USB_RC_OK )
	{
		killDevice(req->m_correspEndpoint->m_correspDevice, req->m_correspEndpoint);
		return;
	}
	fetchDescriptorCounter = dev->m_fetchDescriptorCounter;
	fetchDesc_1 = fetchDescriptorCounter + 1;
	dev->m_fetchDescriptorCounter = fetchDesc_1;
	fetchDesc_2 = fetchDescriptorCounter & 1;
	curDescNum_1 = fetchDescriptorCounter >> 1;
	destdata = (UsbConfigDescriptor *)dev->m_staticDeviceDescEndPtr;
	if ( curDescNum_1 > 0 && !fetchDesc_2 )
		dev->m_staticDeviceDescEndPtr = (u8 *)dev->m_staticDeviceDescEndPtr + READ_UINT16(&destdata->wTotalLength);
	readLen = fetchDesc_2 ? READ_UINT16(&destdata->wTotalLength) : 4;
	if ( (u8 *)dev->m_staticDeviceDescPtr + usbConfig.m_maxStaticDescSize < (u8 *)(&destdata->bLength + readLen) )
	{
		dbg_printf("USBD: UsbdDevice_t ignored, UsbdDevice_t descriptors too large\n");
		return;  // buffer is too small, silently ignore the device
	}
	if ( curDescNum_1 < staticDeviceDescPtr->bNumConfigurations )
	{
		doControlTransfer(
			req->m_correspEndpoint,
			&dev->m_ioRequest,
			USB_DIR_IN | USB_RECIP_DEVICE,
			USB_REQ_GET_DESCRIPTOR,
			curDescNum_1 | (USB_DT_CONFIG << 8),
			0,
			readLen,
			destdata,
			fetchConfigDescriptors);
		return;
	}
	dev->m_reportDescriptorStart = NULL;
	if ( usbConfig.m_curDescNum )
	{
		handleStaticDeviceDescriptor(
			dev, (UsbDeviceDescriptor *)dev->m_staticDeviceDescPtr, (UsbDeviceDescriptor *)dev->m_staticDeviceDescEndPtr);
		if ( dev->m_reportDescriptorStart )
		{
			dev->m_reportDescriptorCurForFetch = dev->m_reportDescriptorStart;
			fetchNextReportDescriptor(req);
			return;
		}
	}
	connectNewDevice(dev);
}

static void requestDeviceDescrptorCB(UsbdIoRequest_t *req)
{
	UsbdDevice_t *dev;
	const UsbDeviceDescriptor *desc;

	dev = req->m_correspEndpoint->m_correspDevice;
	if ( req->m_resultCode != USB_RC_OK )
	{
		dbg_printf("unable to read device descriptor, err %d\n", req->m_resultCode);
		killDevice(req->m_correspEndpoint->m_correspDevice, req->m_correspEndpoint);
		return;
	}
	if ( req->m_transferedBytes >= sizeof(UsbDeviceDescriptor) )
	{
		dev->m_fetchDescriptorCounter = 0;
		dev->m_staticDeviceDescEndPtr = &((UsbDeviceDescriptor *)dev->m_staticDeviceDescEndPtr)[1];
		fetchConfigDescriptors(req);
		return;
	}
	desc = dev->m_staticDeviceDescPtr;
	req->m_correspEndpoint->m_hcEd->m_hcArea.stru.m_maxPacketSize =
		(req->m_correspEndpoint->m_hcEd->m_hcArea.stru.m_maxPacketSize & 0xF800) | desc->bMaxPacketSize0;
	req->m_length = sizeof(UsbDeviceDescriptor);
	requestDeviceDescriptor(req);
}

static void requestDeviceDescriptor(UsbdIoRequest_t *req)
{
	UsbdDevice_t *dev;

	dev = req->m_correspEndpoint->m_correspDevice;
	dev->m_staticDeviceDescEndPtr = dev->m_staticDeviceDescPtr;
	doControlTransfer(
		req->m_correspEndpoint,
		&dev->m_ioRequest,
		USB_DIR_IN | USB_RECIP_DEVICE,
		USB_REQ_GET_DESCRIPTOR,
		USB_DT_DEVICE << 8,
		0,
		req->m_length,
		dev->m_staticDeviceDescPtr,
		requestDeviceDescrptorCB);
}

static void hubPeekDeviceDescriptor(UsbdIoRequest_t *req)
{
	req->m_length = 8;
	requestDeviceDescriptor(req);

	// we've assigned a function address to the device and can reset the next device now, if there is one
	checkDelayedResets(req->m_correspEndpoint->m_correspDevice);
}

static void hubSetFuncAddressCB(UsbdIoRequest_t *req)
{
	void *cb_arg;
	UsbdDevice_t *dev;
	void (*cb_func)(void *);
	int cb_delay;

	dev = req->m_correspEndpoint->m_correspDevice;
	if ( req->m_resultCode == USB_RC_NORESPONSE )
	{
		dbg_printf("device not responding\n");
		dev->m_functionDelay <<= 1;
		if ( dev->m_functionDelay > 0x500 )
		{
			killDevice(dev, req->m_correspEndpoint);
			return;
		}
		cb_func = (void (*)(void *))hubSetFuncAddress;
		cb_arg = req->m_correspEndpoint;
		cb_delay = dev->m_functionDelay | 1;
	}
	else
	{
		cb_func = (void (*)(void *))hubPeekDeviceDescriptor;
		cb_arg = req;
		cb_delay = 5;
		req->m_correspEndpoint->m_hcEd->m_hcArea.stru.m_hcArea |= dev->m_functionAddress & 0x7F;
		dev->m_deviceStatus = DEVICE_FETCHINGDESCRIPTOR;
	}
	addTimerCallback(&dev->m_timer, cb_func, cb_arg, cb_delay);
}

static void hubSetFuncAddress(UsbdEndpoint_t *ep)
{
	// dbg_printf("setting FA %02X\n", ep->m_correspDevice->m_functionAddress);
	doControlTransfer(
		ep,
		&ep->m_correspDevice->m_ioRequest,
		USB_DIR_OUT | USB_RECIP_DEVICE,
		USB_REQ_SET_ADDRESS,
		ep->m_correspDevice->m_functionAddress,
		0,
		0,
		NULL,
		hubSetFuncAddressCB);
}

int hubTimedSetFuncAddress(UsbdDevice_t *dev)
{
	dev->m_functionDelay = 20;
	addTimerCallback(&dev->m_timer, (TimerCallback)hubSetFuncAddress, dev->m_endpointListStart, 21);
	return 0;
}

void flushPort(UsbdDevice_t *dev)
{
	UsbdReportDescriptor_t *desc;
	UsbdDevice_t *child;
	int state;

	if ( dev->m_deviceStatus != DEVICE_NOTCONNECTED )
	{
		dev->m_deviceStatus = DEVICE_NOTCONNECTED;
		if ( dev->m_devDriver )
			callUsbDriverFunc(dev->m_devDriver->disconnect, dev->m_id, dev->m_devDriver->gp);
		dev->m_devDriver = NULL;
		if ( dev->m_timer.m_isActive )
			cancelTimerCallback(&dev->m_timer);
		while ( dev->m_endpointListStart )
		{
			removeEndpointFromDevice(dev, dev->m_endpointListStart);
		}
		for ( desc = dev->m_reportDescriptorStart; desc; desc = dev->m_reportDescriptorStart )
		{
			if ( desc->m_next )
				desc->m_next->m_prev = desc->m_prev;
			else
				dev->m_reportDescriptorEnd = desc->m_prev;
			if ( desc->m_prev )
				desc->m_prev->m_next = desc->m_next;
			else
				dev->m_reportDescriptorStart = desc->m_next;
			CpuSuspendIntr(&state);
			FreeSysMemory(desc);
			CpuResumeIntr(state);
		}
		while ( dev->m_childListStart )
		{
			child = dev->m_childListStart;
			if ( child->m_next )
				child->m_next->m_prev = child->m_prev;
			else
				dev->m_childListEnd = child->m_prev;
			if ( child->m_prev )
				child->m_prev->m_next = child->m_next;
			else
				dev->m_childListStart = child->m_next;
			flushPort(child);
			freeDevice(child);
		}
		dev->m_ioRequest.m_busyFlag = 0;
	}
	if ( dev->m_resetFlag )
		checkDelayedResets(dev);
}

int usbdInitInner(void)
{
	printf("USB Driver (%s)", usbdVersionString);
	printf("\n");
	dbg_printf("HCD init...\n");
	if ( initHcdStructs() < 0 )
		return -1;
	dbg_printf("Hub driver...\n");
	if ( initHubDriver() < 0 )
		return -1;
	return 0;
}
