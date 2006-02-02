/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: $
# USB Driver function prototypes and constants.
*/

#include "usbdpriv.h"
#include "mem.h"
#include "usbio.h"
#include "hub.h"
#include "driver.h"

#include "stdio.h"
#include "sysclib.h"
#include "sysmem.h"
#include "thbase.h"
#include "thevent.h"
#include "thsemap.h"
#include "intrman.h"

int hcdIrqEvent;

int cleanUpFunc(Device *dev, Endpoint *ep) {
	if (!ep)
		return 0;

	if ((ep < memPool.endpointBuf) || (ep >= memPool.endpointBuf + usbConfig.maxEndpoints))
		return 0;

	if (ep->inTdQueue)
		removeEndpointFromQueue(ep);

	if (ep->next)
		ep->next->prev = ep->prev;
	else
		dev->endpointListEnd = ep->prev;

	if (ep->prev)
		ep->prev->next = ep->next;
	else
		dev->endpointListStart = ep->next;

	ep->correspDevice = NULL;
	ep->next = NULL;
	ep->prev = memPool.freeEpListEnd;
	if (memPool.freeEpListEnd)
		memPool.freeEpListEnd->next = ep;
	else
		memPool.freeEpListStart = ep;
	memPool.freeEpListEnd = ep;
	return 0;
}

Endpoint *openDeviceEndpoint(Device *dev, UsbEndpointDescriptor *endpDesc, uint32 alignFlag) {
	
	uint16 flags = 0;
	uint16 hcMaxPktSize;
	uint8  endpType = 0;
	uint8  type;
	HcTD   *td = NULL;

	Endpoint *newEp = allocEndpointForDevice(dev, alignFlag);
	
	if (!newEp) {
		dbg_printf("ran out of endpoints\n");
		return NULL;
	}

	if (endpDesc) {
		type = endpDesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
		hcMaxPktSize = (endpDesc->wMaxPacketSizeHB << 8) | endpDesc->wMaxPacketSizeLB;

		if (type == USB_ENDPOINT_XFER_ISOC) {
			endpType = TYPE_ISOCHRON;
			td = (HcTD *)allocIsoTd();
			if (!td) {
				cleanUpFunc(dev, newEp);
				dbg_printf("Open ISOC EP: no TDs left\n");
				return NULL;
			}				
		} else if (type == USB_ENDPOINT_XFER_CONTROL) {
			endpType = TYPE_CONTROL;
			if (!alignFlag)
				if (hcMaxPktSize >= 0x3F)
					hcMaxPktSize = 0x3E;
		} else { // BULK or INT
			if (type == USB_ENDPOINT_XFER_INT) {
				if (endpDesc->bInterval >= 0x20)
					endpType = 0x1F;
				else if (endpDesc->bInterval >= 0x10)
					endpType = 0xF;
				else if (endpDesc->bInterval >= 8)
					endpType = 7;
				else if (endpDesc->bInterval >= 4)
					endpType = 3;
				else if (endpDesc->bInterval >= 2)
					endpType = 1;
				else
					endpType = 0;

				// todo: add bandwidth scheduling

				dbg_printf("opening INT endpoint (%d - %p), interval %d, list %d\n", newEp->id, newEp, endpDesc->bInterval, endpType);
			} else
				endpType = TYPE_BULK;

			if (((endpDesc->bEndpointAddress & USB_DIR_IN) == 0) && !alignFlag)
				if (hcMaxPktSize >= 0x3F)
					hcMaxPktSize = 0x3E;
		}

		hcMaxPktSize &= 0x7FF;
		if (type == USB_ENDPOINT_XFER_ISOC)
			flags |= HCED_ISOC;
		if (dev->isLowSpeedDevice)
			flags |= HCED_SPEED;

		flags |= (endpDesc->bEndpointAddress & 0xF) << 7;

		if (endpDesc->bEndpointAddress & USB_DIR_IN)
			flags |= HCED_DIR_IN;
		else
			flags |= HCED_DIR_OUT;

		flags |= dev->functionAddress & 0x7F;

		newEp->hcEd.hcArea = flags;
		newEp->hcEd.maxPacketSize = hcMaxPktSize;
	} else {
		newEp->hcEd.maxPacketSize = 8;
		if (dev->isLowSpeedDevice)
			newEp->hcEd.hcArea = HCED_SPEED;
		else
			newEp->hcEd.hcArea = 0;
		endpType = TYPE_CONTROL;
	}

	newEp->endpointType = endpType;
	if (!td) {
		td = allocTd();
		if (!td) {
			dbg_printf("Ran out of TDs\n");
			cleanUpFunc(dev, newEp);
			return NULL;
		}
	}
	newEp->hcEd.tdHead = newEp->hcEd.tdTail = td;
	addToHcEndpointList(newEp->endpointType, &newEp->hcEd);
	return newEp;
}

Endpoint *doOpenEndpoint(Device *dev, UsbEndpointDescriptor *endpDesc, uint32 alignFlag) {
	if (!dev->parent)
		return NULL;

	if (endpDesc == NULL)
		return dev->endpointListStart; // default control EP was already opened
	else
		return openDeviceEndpoint(dev, endpDesc, alignFlag);		
}

int doCloseEndpoint(Endpoint *ep) {
	Device *dev = ep->correspDevice;

	if (dev->endpointListStart != ep)
		return removeEndpointFromDevice(dev, ep);
	else
		return 0;
}

void *doGetDeviceStaticDescriptor(int devId, void *data, uint8 type) {
	UsbDeviceDescriptor *descBuf;
	Device *dev = fetchDeviceById(devId);
	if (!dev)
		return NULL;

	if (data)
		descBuf = (UsbDeviceDescriptor *) ((uint8 *)data + ((UsbDeviceDescriptor*)data)->bLength);
	else
		descBuf = (UsbDeviceDescriptor *) dev->staticDeviceDescPtr;

	if (type == 0)
		return descBuf;

	while (((uint8*)descBuf < (uint8*)dev->staticDeviceDescEndPtr) && (descBuf->bLength >= 2)) {
		if (descBuf->bDescriptorType == type)
			return descBuf;
		descBuf = (UsbDeviceDescriptor *) ((uint8 *)descBuf + descBuf->bLength);
	}
	return NULL;
}

void handleRhsc(void) {
	uint32 portNum = 0;
	Device *port = memPool.deviceTreeRoot->childListStart;

	while (port) {
		uint32 status = memPool.ohciRegs->HcRhPortStatus[portNum];
		memPool.ohciRegs->HcRhPortStatus[portNum] = C_PORT_FLAGS; // reset all flags
		if (status & BIT(PORT_CONNECTION)) {
			if ((port->deviceStatus != DEVICE_NOTCONNECTED) && (status & BIT(C_PORT_CONNECTION)))
				flushPort(port);

			if (port->deviceStatus == DEVICE_NOTCONNECTED) { 
				port->deviceStatus = DEVICE_CONNECTED;
				addTimerCallback(&port->timer, (TimerCallback)hubResetDevice, port, 500);
			} else if (port->deviceStatus == DEVICE_RESETPENDING) {
				if (!(status & BIT(PORT_RESET))) {
					port->deviceStatus = DEVICE_RESETCOMPLETE;
					port->isLowSpeedDevice = (status >> PORT_LOW_SPEED) & 1;
					Endpoint *ep = openDeviceEndpoint(port, NULL, 0);
					if (ep)
						hubTimedSetFuncAddress(port);
				}
			}
		} else
			flushPort(port);
		port = port->next;
		portNum++;
	}
}

void hcdProcessIntr(void) {
	uint32 intrFlags;
	
	intrFlags = memPool.ohciRegs->HcInterruptStatus & memPool.ohciRegs->HcInterruptEnable;

	if (intrFlags & OHCI_INT_SO) {
		dbg_printf("HC: Scheduling overrun\n");
		memPool.ohciRegs->HcInterruptStatus |= OHCI_INT_SO;
		intrFlags &= ~OHCI_INT_SO;
	}

	HcTD *doneQueue = (HcTD*)((uint32)memPool.hcHCCA->DoneHead & ~0xF);
	if (doneQueue) {
		memPool.hcHCCA->DoneHead = NULL;
		memPool.ohciRegs->HcInterruptStatus |= OHCI_INT_WDH;

		// reverse queue
		HcTD *prev = NULL;
		do {
			HcTD *tmp = doneQueue;
			doneQueue = tmp->next;
			tmp->next = prev;
			prev = tmp;
		} while (doneQueue);

		do {
			HcTD *tmp = prev->next;
			if ((prev >= memPool.hcTdBuf) && (prev < memPool.hcTdBufEnd))
				processDoneQueue_GenTd(prev);
			else if ((prev >= (HcTD *)memPool.hcIsoTdBuf) && (prev < (HcTD *)memPool.hcIsoTdBufEnd))
				processDoneQueue_IsoTd((HcIsoTD *)prev);
			prev = tmp;
		} while (prev);

		intrFlags &= ~OHCI_INT_WDH;
	}

	if (intrFlags & OHCI_INT_SF) {
		memPool.ohciRegs->HcInterruptStatus |= OHCI_INT_SF;
		handleTimerList();
		intrFlags &= ~OHCI_INT_SF;
	}

	if (intrFlags & OHCI_INT_UE) {
		printf("HC: Unrecoverable error\n");
		memPool.ohciRegs->HcInterruptStatus |= OHCI_INT_UE;
		intrFlags &= ~OHCI_INT_UE;
	}

	if (intrFlags & OHCI_INT_RHSC) {
		dbg_printf("RHSC\n");
		memPool.ohciRegs->HcInterruptStatus |= OHCI_INT_RHSC;
		handleRhsc();
		intrFlags &= ~OHCI_INT_RHSC;
	}

	intrFlags &= ~OHCI_INT_MIE;
	if (intrFlags) {
		dbg_printf("Disable intr: %d\n", intrFlags);
		memPool.ohciRegs->HcInterruptDisable = intrFlags;
	}
}

void hcdIrqThread(void *arg) {
	u32 eventRes;
	while (1) {
		WaitEventFlag(hcdIrqEvent, 1, WEF_CLEAR | WEF_OR, &eventRes);

		usbdLock();
		hcdProcessIntr();
		EnableIntr(IOP_IRQ_USB);
		usbdUnlock();
	}
}

int usbdIntrHandler(void *arg) {
	iSetEventFlag((int)arg, 1);
	return 0;
}

int initHardware(void) {
	dbg_printf("Host Controller...\n");
	memPool.ohciRegs->HcInterruptDisable = ~0;
	memPool.ohciRegs->HcCommandStatus = OHCI_COM_HCR;
	memPool.ohciRegs->HcControl = 0;
	while (memPool.ohciRegs->HcCommandStatus & OHCI_COM_HCR) {
		// add timeout stuff
	}
	dbg_printf("HC reset done\n");
	*(volatile uint32*)0xBF801570 |= 0x800 << 16;
	*(volatile uint32*)0xBF801680 = 1;

	return 0;
}

int initHcdStructs(void) {
	int		i;
	HcCA	*hcCommArea;
	memPool.ohciRegs = (volatile OhciRegs*)OHCI_REG_BASE;

	initHardware();

	dbg_printf("Structs...\n");

	memPool.hcHCCA = NULL;
	memPool.hcIsoTdBuf			= (HcIsoTD *)	((uint8*)memPool.hcHCCA + sizeof(HcCA));
	memPool.hcIsoTdBufEnd		=				 memPool.hcIsoTdBuf + usbConfig.maxIsoTransfDesc;
	memPool.hcTdBuf				= (HcTD *)		 memPool.hcIsoTdBufEnd;
	memPool.hcTdBufEnd			=				 memPool.hcTdBuf + usbConfig.maxTransfDesc;
	memPool.hcEdBuf				= (HcED *)		 memPool.hcTdBufEnd;
	memPool.endpointBuf			= (Endpoint *)	(memPool.hcEdBuf + 0x42);
	memPool.deviceTreeBuf		= (Device *)	(memPool.endpointBuf + usbConfig.maxEndpoints);
	memPool.ioReqBufPtr			= (IoRequest *)	(memPool.deviceTreeBuf + usbConfig.maxDevices);
	memPool.hcIsoTdToIoReqLUT	= (IoRequest **)(memPool.ioReqBufPtr + usbConfig.maxIoReqs);
	memPool.hcTdToIoReqLUT		= (IoRequest **)(memPool.hcIsoTdToIoReqLUT + usbConfig.maxIsoTransfDesc);

	uint8 *devDescBuf			= (uint8*)		(memPool.hcTdToIoReqLUT + usbConfig.maxTransfDesc);
	uint32 memSize = ((uint32)devDescBuf) + usbConfig.maxDevices * usbConfig.maxStaticDescSize;

	uint8 *memBuf	= AllocSysMemory(ALLOC_FIRST, memSize, 0);
	memset(memBuf, 0, memSize);

	hcCommArea					= (HcCA *)		memBuf;
	memPool.hcHCCA				= (HcCA *)		((((uint32)memBuf + (uint32)memPool.hcHCCA) & 0x1FFFFFFF) | 0xA0000000);
	memPool.hcIsoTdBuf			= (HcIsoTD *)	((uint32)memBuf + (uint32)memPool.hcIsoTdBuf);
	memPool.hcIsoTdBufEnd		= (HcIsoTD *)	((uint32)memBuf + (uint32)memPool.hcIsoTdBufEnd);
	memPool.hcTdBuf				= (HcTD *)		((uint32)memBuf + (uint32)memPool.hcTdBuf);
	memPool.hcTdBufEnd			= (HcTD *)		((uint32)memBuf + (uint32)memPool.hcTdBufEnd);
	memPool.hcEdBuf				= (HcED *)		((uint32)memBuf + (uint32)memPool.hcEdBuf);
	memPool.endpointBuf			= (Endpoint *)	((uint32)memBuf + (uint32)memPool.endpointBuf);
	memPool.deviceTreeBuf		= (Device *)	((uint32)memBuf + (uint32)memPool.deviceTreeBuf);
	memPool.ioReqBufPtr			= (IoRequest *) ((uint32)memBuf + (uint32)memPool.ioReqBufPtr);
	memPool.hcIsoTdToIoReqLUT	= (IoRequest **)((uint32)memBuf + (uint32)memPool.hcIsoTdToIoReqLUT);
	memPool.hcTdToIoReqLUT		= (IoRequest **)((uint32)memBuf + (uint32)memPool.hcTdToIoReqLUT);

	devDescBuf					= (uint8 *)		((uint32)memBuf + (uint32)devDescBuf);

	Endpoint *ep = memPool.endpointBuf;

	for (i = 0; i < usbConfig.maxEndpoints; i++) {
		ep->id = i;

		ep->next = NULL;
		ep->prev = memPool.freeEpListEnd;
		if (memPool.freeEpListEnd)
			memPool.freeEpListEnd->next = ep;
		else
			memPool.freeEpListStart = ep;
		memPool.freeEpListEnd = ep;
		ep++;
	}

	memPool.tdQueueStart[0] = memPool.tdQueueStart[1] = NULL;
	memPool.tdQueueEnd[0] = memPool.tdQueueEnd[1] = NULL;

	Device *dev = memPool.deviceTreeBuf;
	for (i = 0; i < usbConfig.maxDevices; i++) {
		dev->functionAddress = i;
		dev->id = i & 0xFF;

		dev->next = NULL;
		dev->prev = memPool.freeDeviceListEnd;
		if (memPool.freeDeviceListEnd)
			memPool.freeDeviceListEnd->next = dev;
		else
			memPool.freeDeviceListStart = dev;
		memPool.freeDeviceListEnd = dev;

		dev->staticDeviceDescPtr = devDescBuf;
		dev++;
		devDescBuf += usbConfig.maxStaticDescSize;
	}

	memPool.deviceTreeRoot = attachChildDevice(NULL, 0); // virtual root
	attachChildDevice(memPool.deviceTreeRoot, 1); // root hub port 0
	attachChildDevice(memPool.deviceTreeRoot, 2); // root hub port 1

	IoRequest *req = memPool.ioReqBufPtr;
	for (i = 0; i < usbConfig.maxIoReqs; i++) {
		req->next = NULL;
		req->prev = memPool.freeIoReqListEnd;
		if (memPool.freeIoReqListEnd)
			memPool.freeIoReqListEnd->next = req;
		else
			memPool.freeIoReqList = req;
		memPool.freeIoReqListEnd = req;
		req++;
	}

	HcTD *hcTd = memPool.freeHcTdList = memPool.hcTdBuf;
	for (i = 0; i < usbConfig.maxTransfDesc - 1; i++) {
		hcTd->next = hcTd + 1;
		hcTd++;
	}
	hcTd->next = NULL;

	HcIsoTD *isoTd = memPool.freeHcIsoTdList = memPool.hcIsoTdBuf;
	for (i = 0; i < usbConfig.maxIsoTransfDesc - 1; i++) {
		isoTd->next = isoTd + 1;
		isoTd++;
	}
	isoTd->next = NULL;

	// build tree for interrupt table
	HcED *ed = memPool.hcEdBuf;
	for (i = 0; i < 0x3F; i++) {
		ed->hcArea = HCED_SKIP;
		if (i == 0)
			ed->next = NULL;
		else
			ed->next = memPool.hcEdBuf + ((i - 1) >> 1);			

		int intrId = i - 31;
		if (intrId >= 0) {
			intrId = ((intrId & 1) << 4) | ((intrId & 2) << 2) | (intrId & 4) | ((intrId & 8) >> 2) | ((intrId & 0x10) >> 4);
			hcCommArea->InterruptTable[intrId] = ed;
		}
		ed++;
	}

	ed->hcArea = HCED_SKIP;
	memPool.ohciRegs->HcControlHeadEd = ed;
	ed++;
	
	ed->hcArea = HCED_SKIP;
	memPool.ohciRegs->HcBulkHeadEd = ed;
	ed++;

	ed->hcArea = HCED_SKIP;
	memPool.hcEdBuf->next = ed; // the isochronous endpoint

	memPool.ohciRegs->HcHCCA = hcCommArea;
	memPool.ohciRegs->HcFmInterval = 0x27782EDF;
	memPool.ohciRegs->HcPeriodicStart = 0x2A2F;
	memPool.ohciRegs->HcInterruptEnable = OHCI_INT_MIE | OHCI_INT_RHSC | OHCI_INT_UE | OHCI_INT_WDH | OHCI_INT_SO;
	memPool.ohciRegs->HcControl |= OHCI_CTR_USB_OPERATIONAL | OHCI_CTR_PLE | OHCI_CTR_IE | OHCI_CTR_CLE | OHCI_CTR_BLE | 3;
	return 0;
}

int hcdInit(void) {
	int irqRes;
	iop_event_t event;
	iop_thread_t thread;

	int hcdTid;

	dbg_printf("Threads and events...\n");
	event.attr = event.option = event.bits = 0;
	hcdIrqEvent = CreateEventFlag(&event);

	dbg_printf("Intr handler...\n");
	DisableIntr(IOP_IRQ_USB, &irqRes);
	if (RegisterIntrHandler(IOP_IRQ_USB, 1, usbdIntrHandler, (void *)hcdIrqEvent) != 0) {
		if (irqRes == IOP_IRQ_USB)
			EnableIntr(IOP_IRQ_USB);
		return 1;
	}

	dbg_printf("HCD thread...\n");
	thread.attr = TH_C;
	thread.option = 0;
	thread.thread = hcdIrqThread;
	thread.stacksize = 0x4000;
	thread.priority = usbConfig.hcdThreadPrio;
	hcdTid = CreateThread(&thread);
	StartThread(hcdTid, (void *)hcdIrqEvent);

	dbg_printf("Callback thread...\n");
	initCallbackThread();

	dbg_printf("HCD init...\n");
	initHcdStructs();
    
	dbg_printf("Hub driver...\n");
	initHubDriver();

	dbg_printf("Enabling interrupts...\n");
	EnableIntr(IOP_IRQ_USB);

	return 0;
}

