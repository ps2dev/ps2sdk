/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libpwroff.h"

#include <errno.h>
#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>
#include <pwroff_rpc.h>

extern void *_gp;

static poweroff_callback poweroff_cb = NULL;
static void *poweroff_data = NULL;

static u8 poffThreadStack[512 * 16] __attribute__ ((aligned(16)));

extern int _iop_reboot_count;
static SifRpcClientData_t cd0;
static struct t_SifRpcDataQueue cb_queue;
static struct t_SifRpcServerData cb_srv;
static int powerOffThreadId = -1;

static void *PowerOff_ee_rpc_handler(int fnum, void *buffer, int len)
{
	switch(fnum){
		case POFF_RPC_BUTTON:
			printf("EE: power button pressed\n");

			if (poweroff_cb)
				poweroff_cb(poweroff_data);
			break;
	}

	return buffer;
}

static void PowerOffThread(void *dat)
{
	static unsigned char cb_rpc_buffer[64] __attribute__((aligned(64)));

	SifSetRpcQueue(&cb_queue, powerOffThreadId);
	SifRegisterRpc(&cb_srv, PWROFF_IRX, &PowerOff_ee_rpc_handler, cb_rpc_buffer, NULL, NULL, &cb_queue);
	SifRpcLoop(&cb_queue);
}

int poweroffInit(void)
{
	ee_thread_t thread;
	int res;
	static int _init_count = -1;

	if(_init_count == _iop_reboot_count)
		return 0;
	_init_count = _iop_reboot_count;

	while(((res = SifBindRpc(&cd0, PWROFF_IRX, 0)) < 0) || (cd0.server == NULL))
		nopdelay();

	// Terminate and delete any previously created threads
	if (powerOffThreadId >= 0) {
		TerminateThread(powerOffThreadId);
		DeleteThread(powerOffThreadId);
		SifRemoveRpc(&cb_srv, &cb_queue);
		SifRemoveRpcQueue(&cb_queue);
		powerOffThreadId = -1;
	}

	thread.initial_priority = POWEROFF_THREAD_PRIORITY;
	thread.stack_size = sizeof(poffThreadStack);
	thread.gp_reg = &_gp;
	thread.func = PowerOffThread;
	thread.stack = (void *)poffThreadStack;
	thread.option = 0;
	thread.attr = 0;
	powerOffThreadId = CreateThread(&thread);
	StartThread(powerOffThreadId, NULL);

	int autoShutdown[4] = {0};
	SifCallRpc(&cd0, PWROFF_ENABLE_AUTO_SHUTOFF, 0, NULL, 0, autoShutdown, sizeof(autoShutdown), NULL, NULL);

	return res;
}

void poweroffSetCallback(poweroff_callback cb, void *arg)
{
	poweroffInit();

	poweroff_cb = cb;
	poweroff_data = arg;
}

void poweroffShutdown(void)
{
	poweroffInit();

	SifCallRpc(&cd0, PWROFF_SHUTDOWN, 0, NULL, 0, NULL, 0, NULL, NULL);
}

void poweroffChangeThreadPriority(int priority)
{
	ChangeThreadPriority(powerOffThreadId, priority);
}
