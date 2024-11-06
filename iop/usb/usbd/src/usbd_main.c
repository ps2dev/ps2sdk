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

#ifdef _IOP
IRX_ID("USB_driver", 2, 4);
#endif
// Based on the module from SCE SDK 3.1.0.

static int usbdModuleUnload(void);

extern struct irx_export_table _exp_usbd;

static UsbdArgOption_t SupportedArgs[] = {
	{"dev=", &usbConfig.m_maxDevices, NULL},
	{"ed=", &usbConfig.m_maxEndpoints, NULL},
	{"gtd=", &usbConfig.m_maxTransfDesc, NULL},
	{"itd=", &usbConfig.m_maxIsoTransfDesc, NULL},
	{"ioreq=", &usbConfig.m_maxIoReqs, NULL},
	{"conf=", &usbConfig.m_maxStaticDescSize, NULL},
	{"hub=", &usbConfig.m_maxHubDevices, NULL},
	{"port=", &usbConfig.m_maxPortsPerHub, NULL},
	{"thpri=", &usbConfig.m_hcdThreadPrio, &usbConfig.m_cbThreadPrio},
	{"reportd=", &usbConfig.m_curDescNum, NULL},
	{NULL, NULL, NULL}};

UsbdMemoryPool_t *memPool = NULL;
UsbdKernelResources_t usbKernelResources = {-1, -1, -1, -1, -1};
UsbdIoRequest_t *cbListStart = NULL;
UsbdIoRequest_t *cbListEnd = NULL;
#ifndef MINI_DRIVER
UsbdConfig_t usbConfig = {
	0x20,
	0x40,
	0x80,
	0x80,
	0x100,
	0x200,
	0x8,
	0x8,
	0x0,
	0x1E,
	0x24,
	0x0,
};
#else
UsbdConfig_t usbConfig = {
	0x10,
	0x20,
	0x40,
	0x40,
	0x100,
	0x200,
	0x4,
	0x4,
	0x0,
	0x1E,
	0x24,
	0x0,
};
#endif
sceUsbdLddOps *drvListStart = NULL;
sceUsbdLddOps *drvListEnd = NULL;
sceUsbdLddOps *drvAutoLoader = NULL;

static int usbdIntrHandler(void *arg)
{
	const UsbdKernelResources_t *kernelResources;

	kernelResources = (UsbdKernelResources_t *)arg;
	iSetEventFlag(kernelResources->m_hcdIrqEvent, 1u);
	return 0;
}

static void hcdIrqThread(void *arg)
{
	u32 efres;

	(void)arg;
	while ( 1 )
	{
		WaitEventFlag(usbKernelResources.m_hcdIrqEvent, 1u, WEF_OR | WEF_CLEAR, &efres);
		usbdLock();
		hcdProcessIntr();
		EnableIntr(IOP_IRQ_USB);
		PostIntrEnableFunction();
		usbdUnlock();
	}
}

static void callbackThreadFunc(void *arg)
{
	UsbdIoRequest_t *req;
	UsbdIoRequest_t reqCopy;
	u32 efres;
	int state;

	(void)arg;
	while ( 1 )
	{
		WaitEventFlag(usbKernelResources.m_callbackEvent, 1u, WEF_OR | WEF_CLEAR, &efres);
		while ( 1 )
		{
			CpuSuspendIntr(&state);
			req = cbListStart;
			if ( req )
			{
				if ( req->m_next )
					req->m_next->m_prev = req->m_prev;
				else
					cbListEnd = req->m_prev;
				if ( req->m_prev )
					req->m_prev->m_next = req->m_next;
				else
					cbListStart = req->m_next;
			}
			CpuResumeIntr(state);
			if ( !req )
				break;
			bcopy(req, &reqCopy, sizeof(UsbdIoRequest_t));
			usbdLock();
			freeIoRequest(req);
			usbdUnlock();
			if ( reqCopy.m_userCallbackProc )
			{
#if USE_GP_REGISTER
				SetGP(reqCopy.m_gpSeg);
#endif
				if ( reqCopy.m_req.bNumPackets )
					reqCopy.m_userCallbackProcMultiIsochronous(reqCopy.m_resultCode, &reqCopy.m_req, reqCopy.m_userCallbackArg);
				else
					reqCopy.m_userCallbackProcRegular(reqCopy.m_resultCode, reqCopy.m_transferedBytes, reqCopy.m_userCallbackArg);
#if USE_GP_REGISTER
				SetGP(_gp);
#endif
			}
		}
	}
}

void usbdReboot(int ac)
{
	if ( (unsigned int)ac < 2 )
		usbdRebootInner();
}

static void ParseOptionInput(const UsbdArgOption_t *option, const char *arguments)
{
	int value_1;
	const char *p_1;
	int value_2;
	const char *p_2;

	value_1 = 0;
	for ( p_1 = arguments; *p_1 && *p_1 != ','; p_1 += 1 )
	{
		if ( (unsigned int)(*p_1 - '0') >= 9 )
			return;
		value_1 = 10 * value_1 + (*p_1 - '0');
	}
	if ( option->value2 ? (*p_1 != ',') : (*p_1 == ',') )
		return;
	if ( arguments < p_1 )
		*option->value = value_1;
	if ( !option->value2 )
	{
		return;
	}
	value_2 = 0;
	for ( p_2 = p_1 + 1; *p_2; p_2 += 1 )
	{
		if ( (unsigned int)(*p_2 - '0') >= 9 )
			return;
		value_2 = 10 * value_2 + (*p_2 - '0');
	}
	if ( p_1 + 1 < p_2 )
		*option->value2 = value_2;
}

int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int i;
	UsbdArgOption_t *args_ptr;
	const char *pParam;
	char *pArgs;
	iop_event_t ef;
	iop_thread_t thparam;
	iop_sema_t sema;
	int intrstate;

	(void)startaddr;
	if ( ac < 0 )
		return usbdModuleUnload();
	for ( i = 1; i < ac; i += 1 )
	{
		for ( args_ptr = SupportedArgs; args_ptr->param; args_ptr += 1 )
		{
			int j;
			for ( j = 0, pParam = args_ptr->param, pArgs = av[i]; pParam[j] && pParam[j] == pArgs[j]; j += 1 )
			{
			}
			if ( !pParam[j] )
			{
				ParseOptionInput(args_ptr, pArgs);
				break;
			}
		}
	}
	dbg_printf("Intr handler...\n");
	DisableIntr(IOP_IRQ_USB, &intrstate);
	if ( RegisterIntrHandler(IOP_IRQ_USB, 1, usbdIntrHandler, &usbKernelResources) )
	{
		if ( intrstate == IOP_IRQ_USB )
		{
			EnableIntr(IOP_IRQ_USB);
			return MODULE_NO_RESIDENT_END;
		}
	}
	else
	{
		dbg_printf("library entries...\n");
		if ( !RegisterLibraryEntries(&_exp_usbd) )
		{
			dbg_printf("Threads and events...\n");

			sema.attr = SA_THPRI;
			sema.initial = 1;
			sema.max = 1;
			sema.option = 0;
			usbKernelResources.m_usbdSema = CreateSema(&sema);
			if ( usbKernelResources.m_usbdSema >= 0 )
			{
				ef.attr = EA_SINGLE;
				ef.option = 0;
				ef.bits = 0;
				usbKernelResources.m_hcdIrqEvent = CreateEventFlag(&ef);
				if ( usbKernelResources.m_hcdIrqEvent >= 0 )
				{
					dbg_printf("HCD thread...\n");
					thparam.attr = TH_C;
					thparam.thread = hcdIrqThread;
#ifndef MINI_DRIVER
					thparam.stacksize = 0x4000;  // 16KiB
#else
					thparam.stacksize = 0x0800;  //  2KiB
#endif
					thparam.option = 0;
					thparam.priority = usbConfig.m_hcdThreadPrio;
					usbKernelResources.m_hcdTid = CreateThread(&thparam);
					if ( usbKernelResources.m_hcdTid >= 0 && !StartThread(usbKernelResources.m_hcdTid, NULL) )
					{
						ef.attr = EA_SINGLE;
						ef.option = 0;
						ef.bits = 0;
						usbKernelResources.m_callbackEvent = CreateEventFlag(&ef);
						if ( usbKernelResources.m_callbackEvent >= 0 )
						{
							dbg_printf("Callback thread...\n");
							thparam.attr = TH_C;
							thparam.thread = callbackThreadFunc;
#ifndef MINI_DRIVER
							thparam.stacksize = 0x4000;  // 16KiB
#else
							thparam.stacksize = 0x0800;  //  2KiB
#endif
							thparam.option = 0;
							thparam.priority = usbConfig.m_cbThreadPrio;
							usbKernelResources.m_callbackTid = CreateThread(&thparam);
							if (
								usbKernelResources.m_callbackTid >= 0 && !StartThread(usbKernelResources.m_callbackTid, NULL)
								&& usbdInitInner() >= 0 )
							{
								dbg_printf("Enabling interrupts...\n");
								EnableIntr(IOP_IRQ_USB);
								dbg_printf("Init done\n");
#if 0
								return MODULE_REMOVABLE_END;
#else
								if ( mi && ((mi->newflags & 2) != 0) )
									mi->newflags |= 0x10;
								return MODULE_RESIDENT_END;
#endif
							}
						}
					}
				}
			}
		}
		else
		{
			dbg_printf("RegisterLibraryEntries failed\n");
		}
		if ( usbKernelResources.m_callbackTid > 0 )
			DeleteThread(usbKernelResources.m_callbackTid);
		if ( usbKernelResources.m_callbackEvent > 0 )
			DeleteEventFlag(usbKernelResources.m_callbackEvent);
		if ( usbKernelResources.m_hcdTid > 0 )
			DeleteThread(usbKernelResources.m_hcdTid);
		if ( usbKernelResources.m_hcdIrqEvent > 0 )
			DeleteEventFlag(usbKernelResources.m_hcdIrqEvent);
		if ( usbKernelResources.m_usbdSema > 0 )
			DeleteSema(usbKernelResources.m_usbdSema);
		ReleaseIntrHandler(IOP_IRQ_USB);
	}
	return MODULE_NO_RESIDENT_END;
}

static int usbdModuleUnload(void)
{
	int intrstate;

	if ( ReleaseLibraryEntries(&_exp_usbd) == 0 )
	{
		return MODULE_REMOVABLE_END;
	}
	DisableIntr(IOP_IRQ_USB, &intrstate);
	ReleaseIntrHandler(IOP_IRQ_USB);
	TerminateThread(usbKernelResources.m_hcdTid);
	TerminateThread(usbKernelResources.m_callbackTid);
	DeleteThread(usbKernelResources.m_hcdTid);
	DeleteThread(usbKernelResources.m_callbackTid);
	DeleteEventFlag(usbKernelResources.m_hcdIrqEvent);
	DeleteEventFlag(usbKernelResources.m_callbackEvent);
	DeleteSema(usbKernelResources.m_usbdSema);
	deinitHcd();
	deinitHubDriver();
	return MODULE_NO_RESIDENT_END;
}
