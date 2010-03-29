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

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopheap.h>
#include <malloc.h>
#include <pwroff_rpc.h>

// PS2DRV includes
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fileXio_rpc.h"
#include "errno.h"

extern void *_gp;

static poweroff_callback poweroff_cb = NULL;
static void *poweroff_data = NULL;

static u8 poffThreadStack[512 * 16] __attribute__ ((aligned(16)));
static int PowerOffSema;

extern int _iop_reboot_count;
static SifRpcClientData_t cd0;

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

int poweroffInit()
{
	int res;
	static int _init_count = -1;

	if(_init_count == _iop_reboot_count)
		return 0;
	_init_count = _iop_reboot_count;

	while(((res = SifBindRpc(&cd0, PWROFF_IRX, 0)) >= 0) && (cd0.server == NULL))
		nopdelay();

	ee_thread_t thread;
	ee_thread_status_t thisThread;

	ee_sema_t sema;
	int tid;

	sema.init_count = 0;
	sema.max_count = 1;
	sema.option = 0;
	PowerOffSema = CreateSema(&sema);

	ReferThreadStatus(GetThreadId(), &thisThread);
	if (thisThread.current_priority == 0) {
		ChangeThreadPriority(GetThreadId(), 51);
		thread.initial_priority = 50;
	} else
		thread.initial_priority = thisThread.current_priority - 1;

	thread.stack_size = 512 * 16;
	thread.gp_reg = &_gp;
	thread.func = PowerOffThread;
	thread.stack = (void *)poffThreadStack;
	tid = CreateThread(&thread);
	StartThread(tid, NULL);
	
	DIntr();
	SifAddCmdHandler(POFF_SIF_CMD, _poff_intr_callback, NULL);
	EIntr();

	int autoShutdown = 0;
	SifCallRpc(&cd0, PWROFF_ENABLE_AUTO_SHUTOFF, 0, NULL, 0, &autoShutdown, sizeof(autoShutdown), 0, 0);
	
	return res;
}

void poweroffSetCallback(poweroff_callback cb, void *arg)
{
	poweroffInit();

	poweroff_cb = cb;
	poweroff_data = arg;
}

void poweroffShutdown()
{
	poweroffInit();
	
	SifCallRpc(&cd0, PWROFF_SHUTDOWN, 0, NULL, 0, NULL, 0, 0, 0);
}
