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

#if 0
static UsbdMemoryPool_t *memPool_unused = NULL;
#endif

static void *hcdMemoryBuffer;

void hcdProcessIntr(void)
{
	volatile OhciRegs *ohciRegs;
	volatile HcCA *hcHCCA;
	int intrFlags;
	UsbdHcTD_t *doneQueue;
	UsbdHcTD_t *prev;
	UsbdHcTD_t *next_tmp1;
	UsbdHcTD_t *next_tmp2;

	ohciRegs = memPool->m_ohciRegs;
	memPool->m_interruptCounters[0] += 1;
	hcHCCA = memPool->m_hcHCCA;
	intrFlags = ohciRegs->HcInterruptStatus & ohciRegs->HcInterruptEnable;
	if ( (intrFlags & OHCI_INT_SO) != 0 )
	{
		dbg_printf("HC: Scheduling overrun\n");
		ohciRegs->HcInterruptStatus = OHCI_INT_SO;
		intrFlags &= ~OHCI_INT_SO;
		memPool->m_interruptCounters[1] += 1;
	}
	doneQueue = (UsbdHcTD_t *)((uiptr)hcHCCA->DoneHead & ~0xF);
	prev = NULL;
	if ( doneQueue )
	{
		hcHCCA->DoneHead = NULL;
		ohciRegs->HcInterruptStatus = OHCI_INT_WDH;

		// reverse queue
		while ( doneQueue )
		{
			next_tmp1 = doneQueue;
			doneQueue = doneQueue->m_next;
			next_tmp1->m_next = prev;
			prev = next_tmp1;
		}
		for ( ; prev; prev = next_tmp2 )
		{
			next_tmp2 = prev->m_next;
			if ( prev < memPool->m_hcTdBuf || prev >= memPool->m_hcTdBufEnd )
			{
				if ( (UsbdHcIsoTD_t *)prev >= memPool->m_hcIsoTdBuf && (UsbdHcIsoTD_t *)prev < memPool->m_hcIsoTdBufEnd )
					processDoneQueue_IsoTd((UsbdHcIsoTD_t *)prev);
			}
			else
			{
				processDoneQueue_GenTd(prev);
			}
		}
		intrFlags &= ~OHCI_INT_WDH;
		memPool->m_interruptCounters[2] += 1;
	}
	if ( (intrFlags & OHCI_INT_SF) != 0 )
	{
		ohciRegs->HcInterruptStatus = OHCI_INT_SF;
		handleTimerList();
		intrFlags &= ~OHCI_INT_SF;
		memPool->m_interruptCounters[3] += 1;
	}
	if ( (intrFlags & OHCI_INT_RD) != 0 )
	{
		ohciRegs->HcInterruptStatus = OHCI_INT_RD;
		intrFlags &= ~OHCI_INT_RD;
		memPool->m_interruptCounters[4] += 1;
	}
	if ( (intrFlags & OHCI_INT_UE) != 0 )
	{
		dbg_printf("HC: Unrecoverable error\n");
		ohciRegs->HcInterruptStatus = OHCI_INT_UE;
		intrFlags &= ~OHCI_INT_UE;
		memPool->m_interruptCounters[5] += 1;
	}
	if ( (intrFlags & OHCI_INT_FNO) != 0 )
	{
		ohciRegs->HcInterruptStatus = OHCI_INT_FNO;
		intrFlags &= ~OHCI_INT_FNO;
		memPool->m_interruptCounters[6] += 1;
	}
	if ( (intrFlags & OHCI_INT_RHSC) != 0 )
	{
		dbg_printf("RHSC\n");
		ohciRegs->HcInterruptStatus = OHCI_INT_RHSC;
		handleRhsc();
		intrFlags &= ~OHCI_INT_RHSC;
		memPool->m_interruptCounters[7] += 1;
	}
	if ( (intrFlags & OHCI_INT_OC) != 0 )
	{
		ohciRegs->HcInterruptStatus = OHCI_INT_OC;
		intrFlags &= ~OHCI_INT_OC;
		memPool->m_interruptCounters[8] += 1;
	}
	intrFlags &= ~OHCI_INT_MIE;
	if ( intrFlags )
	{
		dbg_printf("Disable intr: %d\n", intrFlags);
		ohciRegs->HcInterruptDisable = intrFlags;
	}
}

void PostIntrEnableFunction(void)
{
	volatile int lw_busy;

	lw_busy = 0;
	memPool->m_ohciRegs->HcInterruptDisable = OHCI_INT_MIE;
	do
	{
		u32 val;
		val = *((volatile u32 *)0xBFC00000);
		__asm__ __volatile__(" " : "=r"(val));
		lw_busy -= 1;
	} while ( lw_busy > 0 );
	memPool->m_ohciRegs->HcInterruptEnable = OHCI_INT_MIE;
}

static int initHardware(volatile OhciRegs *ohciRegs)
{
	int i;

	dbg_printf("Host Controller...\n");
	ohciRegs->HcInterruptDisable = ~0;
	ohciRegs->HcControl &= ~0x3Cu;
	DelayThread(2000);
	ohciRegs->HcCommandStatus = OHCI_COM_HCR;
	ohciRegs->HcControl = 0;
	for ( i = 1; i < 1000; i += 1 )
	{
		volatile int lw_busy;

		if ( (ohciRegs->HcCommandStatus & OHCI_COM_HCR) == 0 )
		{
			dbg_printf("HC reset done\n");
			return 0;
		}

		lw_busy = 0;
		do
		{
			u32 val;
			val = *((volatile u32 *)0xBFC00000);
			__asm__ __volatile__(" " : "=r"(val));
			lw_busy -= 1;
		} while ( lw_busy > 0 );
	}
	return -1;
}

int initHcdStructs(void)
{
	int memSize;
	void *memBuf_1;
	HcCA *hcCommArea;
	UsbdHcIsoTD_t *hcIsoTdBuf;
	UsbdHcTD_t *hcTdBuf;
	int i;
	UsbdHcED_t *hcEdBufForEndpoint;
	UsbdHcED_t *hcEdBuf;
	UsbdEndpoint_t *endpointBuf;
	UsbdDevice_t *deviceTreeBuf;
	UsbdIoRequest_t *ioReqBuf;
	UsbdIoRequest_t **hcIsoTdToIoReqLUT;
	UsbdIoRequest_t **hcTdToIoReqLUT;
	UsbdIoRequest_t **devDescBuf;
	volatile OhciRegs *ohciRegs;

	ohciRegs = (volatile OhciRegs *)OHCI_REG_BASE;
	if ( initHardware(ohciRegs) < 0 )
		return -1;
	*(vu32 *)0xBF801570 |= 0x8000000u;
	*(vu32 *)0xBF801680 = 1;
	dbg_printf("Structs...\n");
	memSize = 0 + 28 + sizeof(HcCA) + sizeof(UsbdHcIsoTD_t) * usbConfig.m_maxIsoTransfDesc
					+ sizeof(UsbdHcTD_t) * usbConfig.m_maxTransfDesc + sizeof(UsbdHcED_t) * usbConfig.m_maxEndpoints
					+ sizeof(UsbdHcED_t) * 66 + sizeof(UsbdEndpoint_t) * usbConfig.m_maxEndpoints
					+ sizeof(UsbdDevice_t) * usbConfig.m_maxDevices + sizeof(UsbdIoRequest_t) * usbConfig.m_maxIoReqs
					+ sizeof(UsbdIoRequest_t *) * usbConfig.m_maxIsoTransfDesc
					+ sizeof(UsbdIoRequest_t *) * usbConfig.m_maxTransfDesc
					+ usbConfig.m_maxStaticDescSize * usbConfig.m_maxDevices;
	memBuf_1 = AllocSysMemoryWrap(memSize);
	if ( !memBuf_1 )
		return -1;
	if ( ((uiptr)memBuf_1) & 0xFF )
	{
		FreeSysMemoryWrap(memBuf_1);
		return -1;
	}
	hcdMemoryBuffer = memBuf_1;
	hcCommArea = (HcCA *)memBuf_1;
	bzero(memBuf_1, memSize);
	hcIsoTdBuf = (UsbdHcIsoTD_t *)(((uiptr)memBuf_1 + (28 + sizeof(HcCA) - 1)) & 0xFFFFFFE0);
#if 0
	*(UsbdConfig_t **)((u8 *)hcIsoTdBuf - 28) = &usbConfig;
#endif
	hcTdBuf = (UsbdHcTD_t *)&hcIsoTdBuf[usbConfig.m_maxIsoTransfDesc];
	hcEdBufForEndpoint = (UsbdHcED_t *)&hcTdBuf[usbConfig.m_maxTransfDesc];
	hcEdBuf = (UsbdHcED_t *)&hcEdBufForEndpoint[usbConfig.m_maxEndpoints];
	memPool = (UsbdMemoryPool_t *)&hcEdBuf[66];
#if 0
	memPool_unused = memPool;
#endif
	endpointBuf = (UsbdEndpoint_t *)&memPool[1];
	deviceTreeBuf = (UsbdDevice_t *)&endpointBuf[usbConfig.m_maxEndpoints];
	ioReqBuf = (UsbdIoRequest_t *)&deviceTreeBuf[usbConfig.m_maxDevices];
	hcIsoTdToIoReqLUT = (UsbdIoRequest_t **)&ioReqBuf[usbConfig.m_maxIoReqs];
	hcTdToIoReqLUT = &hcIsoTdToIoReqLUT[usbConfig.m_maxIsoTransfDesc];
	devDescBuf = &hcTdToIoReqLUT[usbConfig.m_maxTransfDesc];
	usbConfig.m_allocatedSize_unused += memSize;
	memPool->m_ohciRegs = ohciRegs;
	memPool->m_hcEdBuf = hcEdBuf;
	memPool->m_hcIsoTdToIoReqLUT = hcIsoTdToIoReqLUT;
	memPool->m_hcTdToIoReqLUT = hcTdToIoReqLUT;
	memPool->m_endpointBuf = endpointBuf;
	memPool->m_hcHCCA = (volatile HcCA *)(((uiptr)memBuf_1 & 0x1FFFFFFF) | 0xA0000000);
	for ( i = 0; i < usbConfig.m_maxEndpoints; i += 1 )
	{
		endpointBuf[i].m_id = i;
		endpointBuf[i].m_hcEd = &hcEdBufForEndpoint[i];
		endpointBuf[i].m_prev = memPool->m_freeEpListEnd;
		if ( memPool->m_freeEpListEnd )
			memPool->m_freeEpListEnd->m_next = &endpointBuf[i];
		else
			memPool->m_freeEpListStart = &endpointBuf[i];
		endpointBuf[i].m_next = NULL;
		memPool->m_freeEpListEnd = &endpointBuf[i];
	}
	memPool->m_tdQueueEnd = NULL;
	memPool->m_tdQueueStart = NULL;
	memPool->m_deviceTreeBuf = deviceTreeBuf;
	for ( i = 0; i < usbConfig.m_maxDevices; i += 1 )
	{
		deviceTreeBuf[i].m_functionAddress = i;
		deviceTreeBuf[i].m_id = (u8)i;
		deviceTreeBuf[i].m_staticDeviceDescPtr = (u8 *)devDescBuf + (usbConfig.m_maxStaticDescSize * i);
		deviceTreeBuf[i].m_prev = memPool->m_freeDeviceListEnd;
		if ( memPool->m_freeDeviceListEnd )
			memPool->m_freeDeviceListEnd->m_next = &deviceTreeBuf[i];
		else
			memPool->m_freeDeviceListStart = &deviceTreeBuf[i];
		deviceTreeBuf[i].m_next = NULL;
		memPool->m_freeDeviceListEnd = &deviceTreeBuf[i];
	}
	memPool->m_deviceTreeRoot = attachChildDevice(NULL, 0);  // virtual root
	memPool->m_deviceTreeRoot->m_magicPowerValue = 2;
	attachChildDevice(memPool->m_deviceTreeRoot, 1u);  // root hub port 0
	attachChildDevice(memPool->m_deviceTreeRoot, 2u);  // root hub port 1
	memPool->m_ioReqBufPtr = ioReqBuf;
	for ( i = 0; i < usbConfig.m_maxIoReqs; i += 1 )
	{
		ioReqBuf[i].m_id = i;
		ioReqBuf[i].m_prev = memPool->m_freeIoReqListEnd;
		if ( memPool->m_freeIoReqListEnd )
			memPool->m_freeIoReqListEnd->m_next = &ioReqBuf[i];
		else
			memPool->m_freeIoReqList = &ioReqBuf[i];
		ioReqBuf[i].m_next = NULL;
		memPool->m_freeIoReqListEnd = &ioReqBuf[i];
	}
	memPool->m_freeHcTdList = hcTdBuf;
	memPool->m_hcTdBuf = hcTdBuf;
	memPool->m_hcTdBufEnd = &hcTdBuf[usbConfig.m_maxTransfDesc];
	for ( i = 0; i < usbConfig.m_maxTransfDesc - 1; i += 1 )
	{
		hcTdBuf[i].m_next = &hcTdBuf[i + 1];
	}
	memPool->m_freeHcIsoTdList = hcIsoTdBuf;
	memPool->m_hcIsoTdBuf = hcIsoTdBuf;
	memPool->m_hcIsoTdBufEnd = &hcIsoTdBuf[usbConfig.m_maxIsoTransfDesc];
	for ( i = 0; i < usbConfig.m_maxIsoTransfDesc - 1; i += 1 )
	{
		hcIsoTdBuf[i].m_next = &hcIsoTdBuf[i + 1];
	}
	// build tree for interrupt table
	for ( i = 0; i < 63; i += 1 )
	{
		int intrId;
		hcEdBuf[i].m_hcArea.asu32 = HCED_SKIP;
		hcEdBuf[i].m_next = (i > 0) ? &hcEdBuf[(i - 1) >> 1] : NULL;
		intrId = i - 31;
		if ( intrId >= 0 )
		{
			intrId = ((intrId & 1) << 4) + ((intrId & 2) << 2) + (intrId & 4) + ((intrId & 8) >> 2) + ((intrId & 0x10) >> 4);
			hcCommArea->InterruptTable[intrId] = &hcEdBuf[i];
		}
	}
	hcEdBuf[TYPE_CONTROL].m_hcArea.asu32 = HCED_SKIP;
	ohciRegs->HcControlHeadEd = &hcEdBuf[TYPE_CONTROL];
	hcEdBuf[TYPE_BULK].m_hcArea.asu32 = HCED_SKIP;
	ohciRegs->HcBulkHeadEd = &hcEdBuf[TYPE_BULK];
	hcEdBuf[TYPE_ISOCHRON].m_hcArea.asu32 = HCED_SKIP;
	hcEdBuf[0].m_next = &hcEdBuf[TYPE_ISOCHRON];  // the isochronous endpoint
	ohciRegs->HcHCCA = hcCommArea;
	ohciRegs->HcFmInterval = 0x27782EDF;
	ohciRegs->HcPeriodicStart = 0x2A2F;
	ohciRegs->HcInterruptEnable = OHCI_INT_SO | OHCI_INT_WDH | OHCI_INT_UE | OHCI_INT_FNO | OHCI_INT_RHSC | OHCI_INT_MIE;
	ohciRegs->HcControl |=
		OHCI_CTR_CBSR | OHCI_CTR_PLE | OHCI_CTR_IE | OHCI_CTR_CLE | OHCI_CTR_BLE | OHCI_CTR_USB_OPERATIONAL;
	return 0;
}

void deinitHcd(void)
{
	UsbdDevice_t *i;
	UsbdReportDescriptor_t *hidDescriptorStart;
	UsbdReportDescriptor_t *next;

	initHardware((OhciRegs *)OHCI_REG_BASE);
	for ( i = memPool->m_freeDeviceListStart; i; i = i->m_next )
	{
		for ( hidDescriptorStart = i->m_reportDescriptorStart, next = hidDescriptorStart; next; hidDescriptorStart = next )
		{
			next = hidDescriptorStart->m_next;
			FreeSysMemoryWrap(hidDescriptorStart);
		}
	}
	FreeSysMemoryWrap(hcdMemoryBuffer);
}
