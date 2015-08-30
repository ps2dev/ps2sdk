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
static int PowerOffSema = -1;

extern int _iop_reboot_count;
static SifRpcClientData_t cd0;
static int powerOffThreadId = -1;

static void PowerOffThread(void *dat)
{
	while(1)
	{
		WaitSema(PowerOffSema);

		printf("EE: poweroff thread woken up\n");

		if (poweroff_cb)
			poweroff_cb(poweroff_data);
	}
}

static void _poff_intr_callback(void *pkt, void *param)
{
	iSignalSema(PowerOffSema);
}

int poweroffInit(void)
{
	ee_thread_t thread;
	ee_sema_t sema;
	int res;
	static int _init_count = -1;
	int *autoShutdown;
	unsigned char buffer[sizeof(*autoShutdown) + DMA_ALIGN_SIZE];

	autoShutdown = DMA_ALIGN(buffer);

	if(_init_count == _iop_reboot_count)
		return 0;
	_init_count = _iop_reboot_count;

	while(((res = SifBindRpc(&cd0, PWROFF_IRX, 0)) >= 0) && (cd0.server == NULL))
		nopdelay();

	// Terminate and delete any previously created threads
	if (powerOffThreadId >= 0) {
		TerminateThread(powerOffThreadId);
		DeleteThread(powerOffThreadId);
		powerOffThreadId = -1;
	}

	// Delete any previously created semaphores
	if (PowerOffSema >= 0)
	{
		DeleteSema(PowerOffSema);
		PowerOffSema = -1;
	}

	sema.init_count = 0;
	sema.max_count = 1;
	sema.option = 0;
	PowerOffSema = CreateSema(&sema);

	thread.initial_priority = POWEROFF_THREAD_PRIORITY;
	thread.stack_size = sizeof(poffThreadStack);
	thread.gp_reg = &_gp;
	thread.func = PowerOffThread;
	thread.stack = (void *)poffThreadStack;
	powerOffThreadId = CreateThread(&thread);
	StartThread(powerOffThreadId, NULL);

	DI();
	SifAddCmdHandler(POFF_SIF_CMD, _poff_intr_callback, NULL);
	EI();

	*autoShutdown = 0;
	SifCallRpc(&cd0, PWROFF_ENABLE_AUTO_SHUTOFF, 0, NULL, 0, autoShutdown, sizeof(*autoShutdown), NULL, NULL);

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

	SignalSema(PowerOffSema);

	SifCallRpc(&cd0, PWROFF_SHUTDOWN, 0, NULL, 0, NULL, 0, NULL, NULL);
}

void poweroffChangeThreadPriority(int priority)
{
	ChangeThreadPriority(powerOffThreadId, priority);
}
