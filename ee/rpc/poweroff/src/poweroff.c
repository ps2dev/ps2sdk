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

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopheap.h>
#include <malloc.h>

// PS2DRV includes
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fileXio_rpc.h"
#include "errno.h"

#include "libhdd.h"

#define POFF_SIF_CMD	20

extern void *_gp;

static void (*userCallbackFunc)(void *) = NULL;
static void *userCallbackArg = NULL;
static u8 poffThreadStack[512 * 16] __attribute__ ((aligned(16)));
static int PowerOffSema;

static unsigned int getStatusReg() {
    register unsigned int rv;
    asm volatile (
        "mfc0 %0, $12\n"
        "nop\n" : "=r" 
	(rv) : );
    return rv;
}

static void setStatusReg(unsigned int v) {
    asm volatile (
        "mtc0 %0, $12\n"
        "nop\n"
	: : "r" (v) );
}

static void setKernelMode() {
    setStatusReg(getStatusReg() & (~0x18));
}

static void setUserMode() {
    setStatusReg((getStatusReg() & (~0x18)) | 0x10);
}

static void PowerOffThread(void *dat)
{
	while(1)
	{
		WaitSema(PowerOffSema);

		printf("EE: poweroff thread woken up\n");

		// Execute user callback if it exists
		if(userCallbackFunc)
			userCallbackFunc(userCallbackArg);

		fileXioDevctl("pfs:", PFSCTL_CLOSE_ALL, NULL, 0, NULL, 0);
		fileXioDevctl("hdd0:", HDDCTL_FLUSH_CACHE, NULL, 0, NULL, 0);
		while(fileXioDevctl("dev9x:", DEV9CTLSHUTDOWN, NULL, 0, NULL, 0) < 0);

		// Turn off PS2
		setKernelMode();

		*((unsigned char *)0xBF402017) = 0;
		*((unsigned char *)0xBF402016) = 0xF;

		setUserMode();
	}
}

static void _poff_intr_callback(void *pkt, void *param)
{
	iSignalSema(PowerOffSema);
}

void hddSetUserPoweroffCallback(void (*user_callback)(void *arg), void *arg)
{
	userCallbackFunc = (void *)user_callback;
	userCallbackArg = arg;
}

void hddPreparePoweroff()
{
	ee_thread_t thread;
	ee_sema_t sema;
	int tid;

	sema.init_count = 0;
	sema.max_count = 1;
	sema.option = 0;
	PowerOffSema = CreateSema(&sema);

	ChangeThreadPriority(GetThreadId(), 51);
	thread.stack_size = 512 * 16;
	thread.gp_reg = &_gp;
	thread.func = PowerOffThread;
	thread.stack = (void *)poffThreadStack;
	thread.initial_priority = 50;
	tid = CreateThread(&thread);
	StartThread(tid, NULL);
	
	DIntr();
	SifAddCmdHandler(POFF_SIF_CMD, _poff_intr_callback, NULL);
	EIntr();
}

void hddPowerOff()
{
	SignalSema(PowerOffSema);
}
