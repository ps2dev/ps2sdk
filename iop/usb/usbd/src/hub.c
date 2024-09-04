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

static void hubStatusChangeCallback(UsbdIoRequest_t *req);
static int hubDrvConnect(int devId);
static int hubDrvDisconnect(int devId);
static int hubDrvProbe(int devId);

static UsbdUsbHub_t *hubBufferList = NULL;
static void *hubBufferListMemoryBuffer = NULL;
static int hubAllocatedCount = 0;
static sceUsbdLddOps usbHubDriver = {
	NULL,
	NULL,
	"hub",
	&hubDrvProbe,
	&hubDrvConnect,
	&hubDrvDisconnect,
	0u,
	0u,
	0u,
	0u,
	0u,
	NULL,
};

static UsbdUsbHub_t *allocHubBuffer(void)
{
	UsbdUsbHub_t *res;

	if ( !hubBufferList )
	{
		return NULL;
	}
	res = hubBufferList;
	hubBufferList = hubBufferList->m_next;
	res->m_desc.bDescriptorType = 0;
	res->m_statusIoReq.m_busyFlag = 0;
	res->m_controlIoReq.m_busyFlag = 0;
	res->m_curAllocatedCount = hubAllocatedCount;
	hubAllocatedCount += 1;
	return res;
}

static void freeHubBuffer(UsbdUsbHub_t *hub)
{
	hub->m_next = hubBufferList;
	hubBufferList = hub;
}

static UsbdDevice_t *fetchPortElemByNumber(UsbdDevice_t *hub, int port)
{
	UsbdDevice_t *res;

	for ( res = hub->m_childListStart; res && port > 0; port -= 1, res = res->m_next )
	{
	}
	return res;
}

static void getHubStatusChange(UsbdUsbHub_t *dev)
{
	if ( dev->m_statusIoReq.m_busyFlag )
	{
		dbg_printf("getHubStatusChange: StatusChangeEP IoReq is busy!\n");
		return;
	}
	attachIoReqToEndpoint(
		dev->m_statusChangeEp,
		&dev->m_statusIoReq,
		dev->m_statusChangeInfo,
		(int)(dev->m_numChildDevices + 7) >> 3,
		hubStatusChangeCallback);
}

static void hubControlTransfer(
	UsbdUsbHub_t *hubDev,
	u8 requestType,
	u8 request,
	u16 value,
	u16 index,
	u16 length,
	void *destData,
	void *callback,
	const char *fromfunc)
{
	if ( hubDev->m_controlIoReq.m_busyFlag )
	{
		dbg_printf("ERROR: hubControlTransfer %p: ioReq busy\n", hubDev);
		printf("hub_control_transfer: busy - %s\n", fromfunc);
	}
	else
		doControlTransfer(
			hubDev->m_controlEp, &hubDev->m_controlIoReq, requestType, request, value, index, length, destData, callback);
}

static void hubGetPortStatusCallback(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;
	int feature;
	UsbdDevice_t *port;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	feature = -1;
	if ( req->m_resultCode != USB_RC_OK )
	{
		dbg_printf("HubGetPortStatusCallback res %d\n", req->m_resultCode);
		return;
	}
	dbg_printf("port status change: %d: %08X\n", dev->m_portCounter, dev->m_portStatusChange);
	if ( (dev->m_portStatusChange & BIT(C_PORT_CONNECTION)) != 0 )
	{
		port = fetchPortElemByNumber(dev->m_controlEp->m_correspDevice, dev->m_portCounter);
		if ( !port )
		{
			hubStatusChangeCallback(&dev->m_statusIoReq);
			return;
		}
		if ( port->m_deviceStatus >= (unsigned int)DEVICE_CONNECTED )
		{
			flushPort(port);
		}
		feature = C_PORT_CONNECTION;
	}
	else if ( (dev->m_portStatusChange & BIT(C_PORT_ENABLE)) != 0 )
	{
		feature = C_PORT_ENABLE;
	}
	else if ( (dev->m_portStatusChange & BIT(C_PORT_SUSPEND)) != 0 )
	{
		feature = C_PORT_SUSPEND;
	}
	else if ( (dev->m_portStatusChange & BIT(C_PORT_OVER_CURRENT)) != 0 )
	{
		feature = C_PORT_OVER_CURRENT;
	}
	else if ( (dev->m_portStatusChange & BIT(C_PORT_RESET)) != 0 )
	{
		feature = C_PORT_RESET;
	}
	if ( feature >= 0 )
	{
		dev->m_portStatusChange &= ~BIT(feature);
		hubControlTransfer(
			dev,
			USB_DIR_OUT | USB_RT_PORT,
			USB_REQ_CLEAR_FEATURE,
			feature & 0xFFFF,
			dev->m_portCounter & 0xFFFF,
			0,
			NULL,
			hubGetPortStatusCallback,
			"clear_port_feature");
		return;
	}
	port = fetchPortElemByNumber(dev->m_controlEp->m_correspDevice, dev->m_portCounter);
	if ( port )
	{
		if ( (dev->m_portStatusChange & BIT(PORT_CONNECTION)) != 0 )
		{
			dbg_printf("Hub Port CCS\n");
			if ( port->m_deviceStatus == DEVICE_NOTCONNECTED )
			{
				dbg_printf("resetting dev\n");
				port->m_deviceStatus = DEVICE_CONNECTED;
				addTimerCallback(&port->m_timer, (TimerCallback)hubResetDevice, port, 501);
				return;
			}
			if ( port->m_deviceStatus == DEVICE_RESETPENDING && (dev->m_portStatusChange & BIT(PORT_ENABLE)) != 0 )
			{
				dbg_printf("hub port reset done, opening control EP\n");
				port->m_deviceStatus = DEVICE_RESETCOMPLETE;
				port->m_isLowSpeedDevice = (dev->m_portStatusChange & BIT(PORT_LOW_SPEED)) != 0;
				if ( openDeviceEndpoint(port, NULL, 0) )
					hubTimedSetFuncAddress(port);
				else
					dbg_printf("Can't open default control ep.\n");
				dev->m_hubStatusCounter = 0;
			}
		}
		else
		{
			dbg_printf("disconnected; flushing port\n");
			flushPort(port);
		}
	}
	hubStatusChangeCallback(&dev->m_statusIoReq);
}

static void hubDeviceResetCallback(UsbdIoRequest_t *arg)
{
	if ( arg->m_resultCode != USB_RC_OK )
	{
		dbg_printf("port reset err: %d\n", arg->m_resultCode);
		return;
	}
	getHubStatusChange((UsbdUsbHub_t *)arg->m_userCallbackArg);
}

void hubResetDevicePort(UsbdDevice_t *dev)
{
	UsbdUsbHub_t *privDataField;

	privDataField = (UsbdUsbHub_t *)dev->m_parent->m_privDataField;
	privDataField->m_hubStatusCounter = dev->m_attachedToPortNo;
	hubControlTransfer(
		privDataField,
		USB_DIR_OUT | USB_RT_PORT,
		USB_REQ_SET_FEATURE,
		PORT_RESET,
		dev->m_attachedToPortNo,
		0,
		NULL,
		hubDeviceResetCallback,
		"hub_port_reset");
}

static void hubCalculateMagicPowerValue(UsbdUsbHub_t *hubDevice)
{
	UsbdDevice_t *dev;

	dev = hubDevice->m_dev;
	if ( (int)dev->m_magicPowerValue )
	{
		if ( (int)dev->m_magicPowerValue >= 0 && (int)dev->m_magicPowerValue < 6 && (int)dev->m_magicPowerValue >= 3 )
		{
			if ( (hubDevice->m_hubStatus & 1) != 0 )
				dev->m_magicPowerValue = 4;
			else
				dev->m_magicPowerValue = 5;
		}
	}
	else if ( hubDevice->m_isSelfPowered )
	{
		if ( (int)hubDevice->m_maxPower <= 0 )
			dev->m_magicPowerValue = 2;
		else
			dev->m_magicPowerValue = 3;
	}
	else
	{
		dev->m_magicPowerValue = (int)hubDevice->m_maxPower > 0;
	}
}

static void hubGetHubStatusCallback(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	if ( req->m_resultCode != USB_RC_OK )
	{
		return;
	}
	if ( (dev->m_hubStatusChange & BIT(C_HUB_LOCAL_POWER)) != 0 )
	{
		dev->m_hubStatusChange &= ~BIT(C_HUB_LOCAL_POWER);
		hubControlTransfer(
			dev,
			USB_DIR_OUT | USB_RT_HUB,
			USB_REQ_CLEAR_FEATURE,
			C_HUB_LOCAL_POWER,
			0,
			0,
			NULL,
			hubGetHubStatusCallback,
			"clear_hub_feature");
		hubCalculateMagicPowerValue(dev);
	}
	else if ( (dev->m_hubStatusChange & BIT(C_HUB_OVER_CURRENT)) != 0 )
	{
		dev->m_hubStatusChange &= ~BIT(C_HUB_OVER_CURRENT);
		hubControlTransfer(
			dev,
			USB_DIR_OUT | USB_RT_HUB,
			USB_REQ_CLEAR_FEATURE,
			C_HUB_OVER_CURRENT,
			0,
			0,
			NULL,
			hubGetHubStatusCallback,
			"clear_hub_feature");
	}
	else
	{
		hubStatusChangeCallback(&dev->m_statusIoReq);
	}
}

static void hubStatusChangeCallback(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;
	int port;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	if ( req->m_resultCode != USB_RC_OK )
	{
		dbg_printf("hubStatusChangeCallback, iores %d\n", req->m_resultCode);
		return;
	}
	if ( (dev->m_statusChangeInfo[0] & 1) != 0 )
	{
		dev->m_statusChangeInfo[0] &= ~1;
		hubControlTransfer(
			dev,
			USB_DIR_IN | USB_RT_HUB,
			USB_REQ_GET_STATUS,
			0,
			0,
			4u,
			&dev->m_hubStatus,
			hubGetHubStatusCallback,
			"scan_change_bitmap");
		return;
	}
	port = dev->m_hubStatusCounter;
	if ( port <= 0 )
	{
		port = 1;
		if ( (int)dev->m_numChildDevices <= 0 )
		{
			getHubStatusChange(dev);
			return;
		}
		while ( (((int)dev->m_statusChangeInfo[port >> 3] >> (port & 7)) & 1) == 0 )
		{
			port += 1;
			if ( (int)dev->m_numChildDevices < port )
			{
				getHubStatusChange(dev);
				return;
			}
		}
	}
	else if ( (((int)dev->m_statusChangeInfo[port >> 3] >> (port & 7)) & 1) == 0 )
	{
		getHubStatusChange(dev);
		return;
	}
	dev->m_statusChangeInfo[port >> 3] &= ~BIT(port & 7);
	dev->m_portCounter = port;
	hubControlTransfer(
		dev,
		USB_DIR_IN | USB_RT_PORT,
		USB_REQ_GET_STATUS,
		0,
		dev->m_portCounter,
		4u,
		&dev->m_portStatusChange,
		hubGetPortStatusCallback,
		"scan_change_bitmap");
}

static void hubSetPortPower(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	// there is no result to check if this is the first call for this hub
	if ( (int)dev->m_portCounter > 0 && req->m_resultCode != USB_RC_OK )
	{
		return;
	}
	dev->m_portCounter += 1;
	if ( ((int)dev->m_numChildDevices < (int)dev->m_portCounter) || usbConfig.m_maxPortsPerHub < (int)dev->m_portCounter )
		getHubStatusChange(dev);
	else
		hubControlTransfer(
			dev,
			USB_DIR_OUT | USB_RT_PORT,
			USB_REQ_SET_FEATURE,
			PORT_POWER,
			dev->m_portCounter,
			0,
			NULL,
			hubSetPortPower,
			"set_port_power");
}

static void hubSetupPorts(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;
	u32 i;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	if ( req->m_resultCode != USB_RC_OK )
	{
		return;
	}
	hubCalculateMagicPowerValue(dev);

	dev->m_hubStatusCounter = 0;
	dev->m_portCounter = 0;
	dev->m_numChildDevices = dev->m_desc.bNbrPorts;
	for ( i = 0; (int)i < (int)dev->m_numChildDevices && (int)i < usbConfig.m_maxPortsPerHub; i += 1 )
	{
		if ( !attachChildDevice(dev->m_controlEp->m_correspDevice, i + 1) )
		{
			dev->m_numChildDevices = i;
			break;
		}
	}
	if ( (int)dev->m_numChildDevices > 0 )
		hubSetPortPower(req);
}

static void hubCheckPorts(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	hubControlTransfer(
		dev, USB_DIR_IN | USB_RT_HUB, USB_REQ_GET_STATUS, 0, 0, 4u, &dev->m_hubStatus, hubSetupPorts, "get_hub_status");
}

static void hubCheckPortsCB(UsbdIoRequest_t *req)
{
	if ( req->m_resultCode != USB_RC_OK )
		return;
	hubCheckPorts(req);
}

static void hubCheckDeviceDesc(UsbdIoRequest_t *req)
{
	UsbdUsbHub_t *dev;

	dev = (UsbdUsbHub_t *)req->m_userCallbackArg;
	if ( req->m_resultCode != USB_RC_OK )
	{
		return;
	}
	if ( dev->m_desc.bDescriptorType == USB_DT_HUB )
		hubCheckPorts(req);  // we've got the descriptor already
	else
		hubControlTransfer(
			dev,
			USB_DIR_IN | USB_RT_HUB,
			USB_REQ_GET_DESCRIPTOR,
			USB_DT_HUB << 8,
			0,
			sizeof(UsbHubDescriptor),
			&dev->m_desc,
			hubCheckPortsCB,
			"set_configuration_done");
}

static int hubDrvConnect(int devId)
{
	UsbdDevice_t *dev;
	UsbConfigDescriptor *confDesc;
	UsbInterfaceDescriptor *intfDesc;
	UsbEndpointDescriptor *endpDesc;
	UsbdUsbHub_t *hubDevice;
	const UsbHubDescriptor *hubDesc;

	dev = fetchDeviceById(devId);
	if ( !dev )
		return -1;
	confDesc = (UsbConfigDescriptor *)doGetDeviceStaticDescriptor(devId, NULL, USB_DT_CONFIG);
	if ( !confDesc || confDesc->bNumInterfaces != 1 )
		return -1;
	intfDesc = (UsbInterfaceDescriptor *)doGetDeviceStaticDescriptor(devId, confDesc, USB_DT_INTERFACE);
	if ( !intfDesc )
	{
		return -1;
	}
	if ( intfDesc->bNumEndpoints != 1 )
	{
		return -1;
	}
	endpDesc = (UsbEndpointDescriptor *)doGetDeviceStaticDescriptor(devId, intfDesc, USB_DT_ENDPOINT);
	if ( !endpDesc )
	{
		return -1;
	}
	if ( (endpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) != USB_DIR_IN )
		return -1;
	if ( (endpDesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT )
		return -1;
	hubDesc = (UsbHubDescriptor *)doGetDeviceStaticDescriptor(devId, endpDesc, USB_DT_HUB);
	hubDevice = allocHubBuffer();
	if ( !hubDevice )
		return -1;
	dev->m_privDataField = hubDevice;
	hubDevice->m_dev = dev;
	hubDevice->m_controlEp = dev->m_endpointListStart;
	hubDevice->m_statusChangeEp = doOpenEndpoint(dev, endpDesc, 0);
	if ( !hubDevice->m_statusChangeEp )
	{
		freeHubBuffer(hubDevice);
		return -1;
	}
	hubDevice->m_statusIoReq.m_userCallbackArg = hubDevice;
	hubDevice->m_controlIoReq.m_userCallbackArg = hubDevice;
	hubDevice->m_maxPower = confDesc->maxPower;
	hubDevice->m_isSelfPowered = (confDesc->bmAttributes >> 6) & 1;
	hubCalculateMagicPowerValue(hubDevice);
	if ( hubDesc )
	{
		bcopy(
			hubDesc,
			&hubDevice->m_desc,
			(hubDesc->bLength < sizeof(UsbHubDescriptor)) ? hubDesc->bLength : sizeof(UsbHubDescriptor));
	}
	hubControlTransfer(
		hubDevice,
		USB_DIR_OUT | USB_RECIP_DEVICE,
		USB_REQ_SET_CONFIGURATION,
		confDesc->bConfigurationValue,
		0,
		0,
		NULL,
		hubCheckDeviceDesc,
		"hub_attach");
	return 0;
}

static int hubDrvDisconnect(int devId)
{
	UsbdDevice_t *dev;

	dev = fetchDeviceById(devId);
	if ( !dev )
		return -1;
	freeHubBuffer((UsbdUsbHub_t *)dev->m_privDataField);
	return 0;
}

static int hubDrvProbe(int devId)
{
	const UsbDeviceDescriptor *devDesc;

	devDesc = (const UsbDeviceDescriptor *)doGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE);
	return devDesc && devDesc->bDeviceClass == USB_CLASS_HUB && devDesc->bNumConfigurations == 1;
}

int initHubDriver(void)
{
	int needMem;
	UsbdUsbHub_t *hub;
	int i;
	void *OldGP;

	needMem = usbConfig.m_maxHubDevices * sizeof(UsbdUsbHub_t);
	hubBufferList = (UsbdUsbHub_t *)AllocSysMemoryWrap(needMem);
	if ( !hubBufferList )
	{
		dbg_printf("ERROR: unable to alloc hub buffer\n");
		return -1;
	}
	hub = hubBufferList;
	hubBufferListMemoryBuffer = hub;
	bzero(hubBufferList, needMem);
	usbConfig.m_allocatedSize_unused += needMem;
	for ( i = 0; i < usbConfig.m_maxHubDevices; i += 1 )
	{
		hub[i].m_next = (i < usbConfig.m_maxHubDevices - 1) ? &hub[i + 1] : NULL;
	}
#if USE_GP_REGISTER
	OldGP = GetGP();
#else
	OldGP = NULL;
#endif
	doRegisterDriver(&usbHubDriver, OldGP);
	return 0;
}

void deinitHubDriver(void)
{
	doUnregisterDriver(&usbHubDriver);
	FreeSysMemoryWrap(hubBufferListMemoryBuffer);
}
