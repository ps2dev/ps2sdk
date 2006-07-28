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
#include "hcd.h"
#include "mem.h"
#include "usbio.h"
#include "driver.h"

#include "stdio.h"
#include "sysclib.h"
#include "sysmem.h"

static UsbHub *hubBufferList;

int hubDrvProbe(int devId);
int hubDrvConnect(int devId);
int hubDrvDisconnect(int devId);

UsbDriver HubDriver = {
	NULL, NULL, // next, prev
	"hub",
	hubDrvProbe,
	hubDrvConnect,
	hubDrvDisconnect,
	0, 0, 0, 0, 0, // reserved fields
	0	// gp
};

int initHubDriver(void) {
	int i;
	UsbHub *hub;
	uint32 needMem = usbConfig.maxHubDevices * sizeof(UsbHub);

	hubBufferList = (UsbHub *)AllocSysMemory(ALLOC_FIRST, needMem, 0);
	if (!hubBufferList) {
		dbg_printf("ERROR: unable to alloc hub buffer\n");
		return -1;
	}
	memset(hubBufferList, 0, needMem);

	hub = hubBufferList;
	for (i = 0; i < usbConfig.maxHubDevices - 1; i++) {
		hub->next = hub + 1;
		hub++;
	}
	hub->next = NULL;

	doRegisterDriver(&HubDriver, NULL); // gp, actually
	return 0;
}

UsbHub *allocHubBuffer(void) {
	UsbHub *res = hubBufferList;
	if (res) {
		hubBufferList = res->next;
		res->desc.bDescriptorType = 0;
		res->controlIoReq.busyFlag = 0;
		res->statusIoReq.busyFlag = 0;
	}
	return res;
}

void freeHubBuffer(UsbHub *hub) {
	if (hub) {
		hub->next = hubBufferList;
		hubBufferList = hub;
	}
}

void HubControlTransfer(UsbHub *hubDev, uint8 requestType, uint8 request, uint16 value,
						  uint16 index, uint16 length, void *destData,
						  void *callback)
{
	if (!hubDev->controlIoReq.busyFlag) {
		doControlTransfer(hubDev->controlEp, &hubDev->controlIoReq,
			requestType, request, value, index, length, destData,
			callback);
	} else
		dbg_printf("ERROR: HubControlTransfer %p: ioReq busy\n", hubDev);
}

void hubGetHubStatusCallback(IoRequest *req);
void hubGetPortStatusCallback(IoRequest *req);
void getHubStatusChange(UsbHub *dev);

void hubStatusChangeCallback(IoRequest *req) {
	UsbHub *dev = (UsbHub *)req->userCallbackArg;
	int port;

	printf("hubStatusChangeCallback\n");

	if (req->resultCode == USB_RC_OK) {
		if (dev->statusChangeInfo[0] & 1) {
			dev->statusChangeInfo[0] &= ~1;
			HubControlTransfer(dev,
				USB_DIR_IN | USB_RT_HUB, USB_REQ_GET_STATUS, 0, 0, 4, &dev->hubStatus,
				hubGetHubStatusCallback);
		} else {
			if (dev->hubStatusCounter > 0) {
				port = dev->hubStatusCounter;
				if (dev->statusChangeInfo[port >> 3] & BIT(port & 7)) {
					dev->statusChangeInfo[port >> 3] &= ~BIT(port & 7);
					dev->portCounter = port;
					HubControlTransfer(dev,
						USB_DIR_IN | USB_RT_PORT, USB_REQ_GET_STATUS, 0, port, 4, &dev->portStatusChange,
						hubGetPortStatusCallback);
                    return;
				}
			} else {
				for (port = 1; port <= dev->numChildDevices; port++) {
					if (dev->statusChangeInfo[port >> 3] & BIT(port & 7)) {
						dev->statusChangeInfo[port >> 3] &= ~BIT(port & 7);
						dev->portCounter = port;
						HubControlTransfer(dev,
							USB_DIR_IN | USB_RT_PORT, USB_REQ_GET_STATUS, 0, port, 4, &dev->portStatusChange,
							hubGetPortStatusCallback);
						return;
					}
				}
			}
			getHubStatusChange(dev);
		}
	} else
		dbg_printf("hubStatusChangeCallback, iores %d\n", req->resultCode);
}

void hubGetHubStatusCallback(IoRequest *req) {
	UsbHub *dev = (UsbHub *)req->userCallbackArg;
	if (req->resultCode == USB_RC_OK) {
		if (dev->hubStatusChange & BIT(C_HUB_LOCAL_POWER)) {
			dev->hubStatusChange &= ~BIT(C_HUB_LOCAL_POWER);
			HubControlTransfer(dev,
				USB_DIR_OUT | USB_RT_HUB, USB_REQ_CLEAR_FEATURE, C_HUB_LOCAL_POWER, 0, 0, NULL,
				hubGetHubStatusCallback);

		} else if (dev->hubStatusChange & BIT(C_HUB_OVER_CURRENT)) {
			dev->hubStatusChange &= ~BIT(C_HUB_OVER_CURRENT);
			HubControlTransfer(dev,
				USB_DIR_OUT | USB_RT_HUB, USB_REQ_CLEAR_FEATURE, C_HUB_OVER_CURRENT, 0, 0, NULL,
				hubGetHubStatusCallback);

		} else
			hubStatusChangeCallback(&dev->statusIoReq);
	}
}

int cancelTimerCallback(TimerCbStruct *arg) {
	if (arg->isActive) {
		if (arg->next)
			arg->next->prev = arg->prev;
		else
			memPool.timerListEnd = arg->prev;

		if (arg->prev)
			arg->prev->next = arg->next;
		else
			memPool.timerListStart = arg->next;

		arg->prev = arg->next = NULL;
		arg->isActive = 0;
        return 0;
	} else
		return -1;
}

int addTimerCallback(TimerCbStruct *arg, TimerCallback func, void *cbArg, uint32 delay) {
	if (arg->isActive)
		return -1;

	arg->isActive = 1;
	arg->callbackProc = func;
	arg->callbackArg  = cbArg;

	TimerCbStruct *pos = memPool.timerListStart;

	while (pos && (delay > pos->delayCount)) {
		delay -= pos->delayCount;
		pos = pos->next;
	}
	if (pos) {
		arg->prev = pos->prev;
		if (pos->prev)
			pos->prev->next = arg;
		else
			memPool.timerListStart = arg;

		arg->next = pos;
		pos->prev = arg;
		pos->delayCount -= delay;
	} else {
		arg->prev = memPool.timerListEnd;
		if (memPool.timerListEnd)
			memPool.timerListEnd->next = arg;
		else
			memPool.timerListStart = arg;
		memPool.timerListEnd = arg;
		arg->next = NULL;
	}
	arg->delayCount = delay;
	memPool.ohciRegs->HcInterruptEnable = OHCI_INT_SF;
	return 0;
}

void killEndpoint(Endpoint *ep) {
	int i = 0;
	IoRequest *req;
	HcED *hcEd = &ep->hcEd;
	if (ep->endpointType == TYPE_ISOCHRON) {
		for (i = 0; i < usbConfig.maxIsoTransfDesc; i++) {
			req = memPool.hcIsoTdToIoReqLUT[i];
			if (req && (req->correspEndpoint == ep)) {
				freeIoRequest(req);
				memPool.hcIsoTdToIoReqLUT[i] = NULL;
				freeIsoTd(memPool.hcIsoTdBuf + i);
			}
		}
		freeIsoTd((HcIsoTD*)hcEd->tdTail);
	} else {
		for (i = 0; i < usbConfig.maxTransfDesc; i++) {
			req = memPool.hcTdToIoReqLUT[i];
			if (req && (req->correspEndpoint == ep)) {
				freeIoRequest(req);
				memPool.hcTdToIoReqLUT[i] = NULL;
				freeTd(memPool.hcTdBuf + i);
			}
		}
		freeTd(hcEd->tdTail);
	}
	hcEd->tdTail = NULL;
	hcEd->tdHead = NULL;

	while ((req = ep->ioReqListStart)) {
		if (req->next)
			req->next->prev = req->prev;
		else
			ep->ioReqListEnd = req->prev;

		if (req->prev)
			req->prev->next = req->next;
		else
			ep->ioReqListStart = req->next;

		freeIoRequest(req);
	}
	removeEndpointFromQueue(ep);

	ep->next = NULL;
	ep->prev = memPool.freeEpListEnd;
	if (memPool.freeEpListEnd)
		memPool.freeEpListEnd->next = ep;
	else
		memPool.freeEpListStart = ep;
	memPool.freeEpListEnd = ep;
}

int removeEndpointFromDevice(Device *dev, Endpoint *ep) {
	ep->hcEd.hcArea |= HCED_SKIP;
	removeHcEdFromList(ep->endpointType, &ep->hcEd);

	if (ep->next)
		ep->next->prev = ep->prev;
	else
		dev->endpointListEnd = ep->prev;

	if (ep->prev)
		ep->prev->next = ep->next;
	else
		dev->endpointListStart = ep->next;

	ep->correspDevice = NULL;
	addTimerCallback(&ep->timer, (TimerCallback)killEndpoint, ep, 200);
	return 0;
}

void hubDeviceResetCallback(IoRequest *arg) {
	if (arg->resultCode == USB_RC_OK)
		getHubStatusChange((UsbHub *)arg->userCallbackArg);
	else
		dbg_printf("port reset err: %d\n", arg->resultCode);
}

int hubResetDevice(Device *dev) {
	UsbHub *hub;
	if (memPool.delayResets) {
		dev->deviceStatus = DEVICE_RESETDELAYED;
		return 0;
	} else {
		memPool.delayResets = 1;
		dev->deviceStatus = DEVICE_RESETPENDING;
		dev->resetFlag = 1;
		if (dev->parent == memPool.deviceTreeRoot) { // root hub port
			memPool.ohciRegs->HcRhPortStatus[dev->attachedToPortNo - 1] = BIT(PORT_RESET);
		} else { // normal hub port
			hub = (UsbHub *)dev->parent->privDataField;
			hub->hubStatusCounter = dev->attachedToPortNo;

			HubControlTransfer(hub,
				USB_DIR_OUT | USB_RT_PORT, USB_REQ_SET_FEATURE, PORT_RESET, dev->attachedToPortNo, 0, NULL,
				hubDeviceResetCallback);
		}
		return 1;
	}
}

int checkDelayedResetsTree(Device *tree) {
	Device *dev;
	for (dev = tree->childListStart; dev != NULL; dev = dev->next) {
		if (dev->deviceStatus == DEVICE_RESETDELAYED) {
			hubResetDevice(dev);
			return 1;
		}
		if ((dev->deviceStatus == DEVICE_READY) && dev->childListStart) {
			if (checkDelayedResetsTree(dev))
				return 1;
		}
	}
	return 0;
}

int checkDelayedResets(Device *dev) {
	memPool.delayResets = 0;
	dev->resetFlag = 0;
	checkDelayedResetsTree(memPool.deviceTreeRoot);
	return 0;
}

void killDevice(Device *dev, Endpoint *ep) {
	removeEndpointFromDevice(dev, ep);
	checkDelayedResets(dev);
	hubResetDevice(dev);
}

void flushPort(Device *dev) {
	Device *child;
	if (dev->deviceStatus != DEVICE_NOTCONNECTED) {
		dev->deviceStatus = DEVICE_NOTCONNECTED;
		if (dev->devDriver) {
			callUsbDriverFunc(dev->devDriver->disconnect, dev->id, dev->devDriver->gp);
			dev->devDriver = NULL;
		}

		if (dev->timer.isActive)
			cancelTimerCallback(&dev->timer);

		while (dev->endpointListStart)
			removeEndpointFromDevice(dev, dev->endpointListStart);

		while ((child = dev->childListStart)) {
			if (child->next)
				child->next->prev = child->prev;
			else
				dev->childListEnd = child->prev;

			if (child->prev)
				child->prev->next = child->next;
			else
				dev->childListStart = child->next;

			flushPort(child);

			freeDevice(child);
		}
		dev->ioRequest.busyFlag = 0;
		dev->privDataField = NULL;
	}
	if (dev->resetFlag)
		checkDelayedResets(dev);
}

void fetchConfigDescriptors(IoRequest *req) {
	Endpoint *ep = req->correspEndpoint;
	Device *dev = ep->correspDevice;
	uint16 readLen;
	int fetchDesc;

	if ((req->resultCode == USB_RC_OK) || (dev->fetchDescriptorCounter == 0)) {
		uint32 curDescNum = dev->fetchDescriptorCounter++;

		fetchDesc = curDescNum & 1;
		curDescNum >>= 1;

		if ((curDescNum > 0) && !fetchDesc) {
			UsbConfigDescriptor *desc = dev->staticDeviceDescEndPtr;
			dev->staticDeviceDescEndPtr += READ_UINT16(&desc->wTotalLength);
		}

		if (fetchDesc) {
			UsbConfigDescriptor *desc = dev->staticDeviceDescEndPtr;
			readLen = READ_UINT16(&desc->wTotalLength);
		} else
			readLen = 4;

		if ((uint8*)dev->staticDeviceDescEndPtr + readLen > (uint8*)dev->staticDeviceDescPtr + usbConfig.maxStaticDescSize) {
			dbg_printf("USBD: Device ignored, Device descriptors too large\n");
			return; // buffer is too small, silently ignore the device
		}

		if (curDescNum < ((UsbDeviceDescriptor*)dev->staticDeviceDescPtr)->bNumConfigurations) {
			doControlTransfer(ep, &dev->ioRequest,
				USB_DIR_IN | USB_RECIP_DEVICE, USB_REQ_GET_DESCRIPTOR, (USB_DT_CONFIG << 8) | curDescNum, 0, readLen,
				dev->staticDeviceDescEndPtr, fetchConfigDescriptors);
		} else
			connectNewDevice(dev);
	} else
		killDevice(dev, ep);
}

void requestDeviceDescriptor(IoRequest *req, uint16 length);

void requestDevDescrptCb(IoRequest *req) {
	Endpoint *ep = req->correspEndpoint;
	Device *dev = ep->correspDevice;
	UsbDeviceDescriptor *desc = dev->staticDeviceDescPtr;

	if (req->resultCode == USB_RC_OK) {
		if (req->transferedBytes < sizeof(UsbDeviceDescriptor)) {
			ep->hcEd.maxPacketSize = (ep->hcEd.maxPacketSize & 0xF800) | desc->bMaxPacketSize0;
			requestDeviceDescriptor(req, sizeof(UsbDeviceDescriptor));
		} else {
			dev->fetchDescriptorCounter = 0;
			dev->staticDeviceDescEndPtr = (uint8*)dev->staticDeviceDescEndPtr + sizeof(UsbDeviceDescriptor);
			fetchConfigDescriptors(req);
		}
	} else {
		dbg_printf("unable to read device descriptor, err %d\n", req->resultCode);
		killDevice(dev, ep);
	}
}

void requestDeviceDescriptor(IoRequest *req, uint16 length) {
	Endpoint *ep = req->correspEndpoint;
	Device *dev = ep->correspDevice;

	dev->staticDeviceDescEndPtr = dev->staticDeviceDescPtr;

	doControlTransfer(ep, &dev->ioRequest,
		USB_DIR_IN | USB_RECIP_DEVICE, USB_REQ_GET_DESCRIPTOR, USB_DT_DEVICE << 8, 0, length, dev->staticDeviceDescEndPtr,
		requestDevDescrptCb);
}

void hubPeekDeviceDescriptor(IoRequest *req) {
	requestDeviceDescriptor(req, 8);

	// we've assigned a function address to the device and can reset the next device now, if there is one
	checkDelayedResets(req->correspEndpoint->correspDevice);
}

void hubSetFuncAddress(Endpoint *ep);

void hubSetFuncAddressCB(IoRequest *req) {
	Endpoint *ep = req->correspEndpoint;
	Device *dev = ep->correspDevice;

	if (req->resultCode == USB_RC_NORESPONSE) {
		dbg_printf("device not responding\n");
		dev->functionDelay <<= 1;
		if (dev->functionDelay <= 0x500)
			addTimerCallback(&dev->timer, (TimerCallback)hubSetFuncAddress, ep, dev->functionDelay);
		else
			killDevice(dev, ep);
	} else {
		ep->hcEd.hcArea |= dev->functionAddress & 0x7F;
		dev->deviceStatus = DEVICE_READY;

		addTimerCallback(&dev->timer, (TimerCallback)hubPeekDeviceDescriptor, req, 4);
	}
}

void hubSetFuncAddress(Endpoint *ep) {
	Device *dev = ep->correspDevice;

	//printf("setting FA %02X\n", dev->functionAddress);
	doControlTransfer(ep, &dev->ioRequest,
		USB_DIR_OUT | USB_RECIP_DEVICE, USB_REQ_SET_ADDRESS, dev->functionAddress, 0, 0, NULL, hubSetFuncAddressCB);
}

int hubTimedSetFuncAddress(Device *dev) {
	dev->functionDelay = 20;
	addTimerCallback(&dev->timer, (TimerCallback)hubSetFuncAddress, dev->endpointListStart, 20);
	return 0;
}

void hubGetPortStatusCallback(IoRequest *req) {
	int feature = -1;
	UsbHub *dev = (UsbHub *)req->userCallbackArg;
	Device *port;
	if (req->resultCode == USB_RC_OK) {
		dbg_printf("port status change: %d: %08X\n", dev->portCounter, dev->portStatusChange);
		if (dev->portStatusChange & BIT(C_PORT_CONNECTION))
			feature = C_PORT_CONNECTION;
		else if (dev->portStatusChange & BIT(C_PORT_ENABLE))
			feature = C_PORT_ENABLE;
		else if (dev->portStatusChange & BIT(C_PORT_SUSPEND))
			feature = C_PORT_SUSPEND;
		else if (dev->portStatusChange & BIT(C_PORT_OVER_CURRENT))
			feature = C_PORT_OVER_CURRENT;
		else if (dev->portStatusChange & BIT(C_PORT_RESET))
			feature = C_PORT_RESET;

		if (feature >= 0) {
			dev->portStatusChange &= ~BIT(feature);
			HubControlTransfer(dev,
				USB_DIR_OUT | USB_RT_PORT, USB_REQ_CLEAR_FEATURE, feature, dev->portCounter, 0, NULL,
				hubGetPortStatusCallback);
		} else {
			port = fetchPortElemByNumber(dev->controlEp->correspDevice, dev->portCounter);
			if (port) {
				if (dev->portStatusChange & BIT(PORT_CONNECTION)) {
					dbg_printf("Hub Port CCS\n");
					if (port->deviceStatus == DEVICE_NOTCONNECTED) {
						dbg_printf("resetting dev\n");
						port->deviceStatus = DEVICE_CONNECTED;
						addTimerCallback(&port->timer, (TimerCallback)hubResetDevice, port, 500);
						return;
					} else if (port->deviceStatus == DEVICE_RESETPENDING) {
						if (dev->portStatusChange & BIT(PORT_ENABLE)) {
							dbg_printf("hub port reset done, opening control EP\n");
							port->deviceStatus = DEVICE_RESETCOMPLETE;
							port->isLowSpeedDevice = (dev->portStatusChange >> PORT_LOW_SPEED) & 1;

							if (openDeviceEndpoint(port, NULL, 0))
								hubTimedSetFuncAddress(port);
							else
								dbg_printf("Can't open default control ep.\n");
							dev->hubStatusCounter = 0;
						}
					}
				} else {
					dbg_printf("disconnected; flushing port\n");
					flushPort(port);
				}
			}
			hubStatusChangeCallback(&dev->statusIoReq);
		}
	} else
		dbg_printf("HubGetPortStatusCallback res %d\n", req->resultCode);
}

void getHubStatusChange(UsbHub *dev) {
	if (dev->statusIoReq.busyFlag == 0) {
		attachIoReqToEndpoint(dev->statusChangeEp, &dev->statusIoReq,
			dev->statusChangeInfo, (dev->numChildDevices + 8) >> 3, hubStatusChangeCallback);
	} else
		dbg_printf("getHubStatusChange: StatusChangeEP IoReq is busy!\n");
}

void hubSetPortPower(IoRequest *req) {
	UsbHub *dev = (UsbHub *)req->userCallbackArg;

	if ((req->resultCode == USB_RC_OK) || (dev->portCounter == 0)) {
		// there is no result to check if this is the first call for this hub
		dev->portCounter++;
		if (dev->portCounter <= dev->numChildDevices) {
			HubControlTransfer(dev,
				USB_DIR_OUT | USB_RT_PORT, USB_REQ_SET_FEATURE, PORT_POWER, dev->portCounter, 0, NULL,
				hubSetPortPower);
		} else
			getHubStatusChange(dev);
	}
}

void hubSetupPorts(IoRequest *req) {
	int port;
	UsbHub *dev = (UsbHub *)req->userCallbackArg;
	Device *usbDev = dev->controlEp->correspDevice;

	if (req->resultCode == USB_RC_OK) {
		dev->hubStatusCounter = 0;
		dev->portCounter = 0;
		dev->numChildDevices = dev->desc.bNbrPorts;
		for (port = 0; port < dev->desc.bNbrPorts; port++) {
			if (!attachChildDevice(usbDev, port + 1)) {
				dev->numChildDevices = port;
				break;
			}
		}
		if (dev->numChildDevices > 0)
			hubSetPortPower(req);
	}
}

void hubCheckPorts(IoRequest *req) {
	UsbHub *dev = (UsbHub *)req->userCallbackArg;
	if (req->resultCode == USB_RC_OK) {
		if (dev->desc.bNbrPorts <= usbConfig.maxPortsPerHub) {
			HubControlTransfer(dev,
				USB_DIR_IN | USB_RT_HUB, USB_REQ_GET_STATUS, 0, 0, 4,
				&dev->hubStatus, hubSetupPorts);
		} else
			dbg_printf("Hub has too many ports (%d > %d)\n", dev->desc.bNbrPorts, usbConfig.maxPortsPerHub);
	}
}

void hubCheckDeviceDesc(IoRequest *req) {
	UsbHub *dev = (UsbHub *)req->userCallbackArg;
	if (req->resultCode == USB_RC_OK) {
		if (dev->desc.bDescriptorType == USB_DT_HUB)
			hubCheckPorts(req);	// we've got the descriptor already
		else
			HubControlTransfer(dev,
				USB_DIR_IN | USB_RT_HUB, USB_REQ_GET_DESCRIPTOR, USB_DT_HUB << 8, 0, sizeof(UsbHubDescriptor), &dev->desc,
				hubCheckPorts);
	}
}

int hubDrvProbe(int devId) {
	UsbDeviceDescriptor *devDesc;
	devDesc = (UsbDeviceDescriptor *)doGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE);

	if (devDesc && (devDesc->bDeviceClass == USB_CLASS_HUB) && (devDesc->bNumConfigurations == 1))
		return 1;
	else
		return 0;
}

int hubDrvConnect(int devId) {
	UsbConfigDescriptor *confDesc;
	UsbInterfaceDescriptor *intfDesc;
	UsbEndpointDescriptor *endpDesc;
	UsbHubDescriptor *hubDesc;
	Device *dev;
	UsbHub *hubDevice;

	dev = fetchDeviceById(devId);
	if (!dev)
		return -1;

	confDesc = doGetDeviceStaticDescriptor(devId, NULL, USB_DT_CONFIG);
	if (!confDesc || (confDesc->bNumInterfaces != 1))
		return -2;

	intfDesc = doGetDeviceStaticDescriptor(devId, confDesc, USB_DT_INTERFACE);
	if (!intfDesc || (intfDesc->bNumEndpoints != 1))
		return -3;

	endpDesc = doGetDeviceStaticDescriptor(devId, intfDesc, USB_DT_ENDPOINT);
	if (!endpDesc)
		return -4;

	if ((endpDesc->bEndpointAddress & USB_DIR_IN) == 0)
		return -5;

	if ((endpDesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT)
		return -6;

	hubDesc = doGetDeviceStaticDescriptor(devId, endpDesc, USB_DT_HUB);

	hubDevice = allocHubBuffer();
	if (!hubDevice)
		return -8;

	dev->privDataField = hubDevice;
	hubDevice->controlEp = dev->endpointListStart;

	hubDevice->statusChangeEp = doOpenEndpoint(dev, endpDesc, 0);
	if (!hubDevice->statusChangeEp) {
		freeHubBuffer(hubDevice);
		return -9;
	}

	hubDevice->controlIoReq.userCallbackArg = hubDevice;
	hubDevice->statusIoReq.userCallbackArg = hubDevice;

	if (hubDesc) {
		uint8 len = hubDesc->bLength;
		if (len > sizeof(UsbHubDescriptor))
			len = sizeof(UsbHubDescriptor);
		memcpy(&hubDevice->desc, hubDesc, len);
	} else
		memset(&hubDevice->desc, 0, sizeof(UsbHubDescriptor));

	HubControlTransfer(hubDevice,
		USB_DIR_OUT, USB_REQ_SET_CONFIGURATION, confDesc->bConfigurationValue, 0, 0, NULL,
		hubCheckDeviceDesc);

	return 0;
}

int hubDrvDisconnect(int devId) {
	Device *dev = fetchDeviceById(devId);
	if (dev) {
		freeHubBuffer((UsbHub *)dev->privDataField);
		return 0;
	} else
		return -1;
}

