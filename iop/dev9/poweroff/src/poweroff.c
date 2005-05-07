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

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "loadcore.h"
#include "sysmem.h"
#include "stdio.h"
#include "sysclib.h"
#include "sifcmd.h"
#include "thbase.h"
#include "intrman.h"
#include "loadcore.h"
#include "thsemap.h"
#include "poweroff.h"

//#define DEBUG


#define INT_CDROM	0x02
#define TYPE_C		1
#define CDVDreg_PWOFF	(*(volatile unsigned char*)0xBF402008)

#define POFF_SIF_CMD	20
#define MAX_CALLBACKS	4

IRX_ID("Poweroff_Handler", 1, 1);

extern struct irx_export_table _exp_poweroff;

//---------------------------------------------------------------------

int  _start(int, char**);


//---------------------------------------------------------------------
typedef int (*intrhandler)(void*);

intrhandler	old=0;

struct handlerTableEntry{
	intrhandler	handler;
	void		*param;
};

struct CallbackEntry
{
	pwoffcb cb;
	void *data;
} CallbackTable[MAX_CALLBACKS];

//---------------------------------------------------------------------

static char cmdData[16];
static int pid;
static int poffSema;

static int myCdHandler(void *param)
{

	if (((CDVDreg_PWOFF & 1)==0) && (CDVDreg_PWOFF & 4)) 
	{
		/* can't seem to register a sif cmd callback in ps2link so... */
		/* Clear interrupt bit */
		CDVDreg_PWOFF = 4;
		isceSifSendCmd(POFF_SIF_CMD, cmdData, 16, NULL, NULL, 0);
		iSignalSema(poffSema);
	}

	return old(param);
}

//---------------------------------------------------------------------
//-----------------------------------------------------------entrypoint
//---------------------------------------------------------------------

static void pCallbackThread(void *arg)
{
	int i;
	while(1)
	{
		WaitSema(poffSema);
		/* Do callbacks in reverse order */
		for(i = MAX_CALLBACKS-1; i >= 0; i--)
		{
			if(CallbackTable[i].cb)
			{
				CallbackTable[i].cb(CallbackTable[i].data);
			}
		}
#ifdef DEBUG	
		printf("Poweroff!!!! %08x\n", CDVDreg_PWOFF);
#endif
	}
}

void AddPowerOffHandler(pwoffcb func, void* param)
{
	int i;

	for(i = 0; i < MAX_CALLBACKS; i++)
	{
		if(CallbackTable[i].cb == 0)
		{
			CallbackTable[i].cb = func;
			CallbackTable[i].data = param;
#ifdef DEBUG
			printf("Added callback at position %d\n", i);
#endif
			break;
		}
	}

	if(i == MAX_CALLBACKS)
	{
		printf("Could not add poweroff callback\n");
	}
}

void RemovePowerOffHandler(pwoffcb func)
{
	int i;

	for(i = 0; i < MAX_CALLBACKS; i++)
	{
		if(CallbackTable[i].cb == func)
		{
			break;
		}
	}

	if(i < MAX_CALLBACKS)
	{
		for(; i < (MAX_CALLBACKS-1); i++)
		{
			CallbackTable[i] = CallbackTable[i+1];
		}
		memset(&CallbackTable[i], 0, sizeof(struct CallbackEntry));
	}
}

int _start(int argc, char* argv[])
{
	register struct handlerTableEntry *handlers=(struct handlerTableEntry*)0x480;//iopmem
	iop_thread_t mythread;
	iop_sema_t sem_info;
	int i;

	if(RegisterLibraryEntries(&_exp_poweroff) != 0)
	{
		printf("Poweroff already registered\n");
		return 1;
	}

	FlushDcache();
	CpuEnableIntr(0);

	if (handlers[INT_CDROM].handler==0) {
		printf("No CDROM handler. Run CDVDMAN first\n");
		return 1;
	}

	if (((int)handlers[INT_CDROM].handler & 3) != TYPE_C){
		printf("Cannot chain to non-C handler\n");
		return 1;
	}

	old=(intrhandler)((int)handlers[INT_CDROM].handler & ~3);
	handlers[INT_CDROM].handler=(intrhandler)((int)myCdHandler | TYPE_C);

	memset(CallbackTable, 0, sizeof(struct CallbackEntry) * MAX_CALLBACKS);

	mythread.attr = 0x02000000;
	mythread.option = 0;
	mythread.thread = pCallbackThread;
	mythread.stacksize = 0x1000;
	mythread.priority = 0x27;

	pid = CreateThread(&mythread);

	if (pid > 0) {
		if ((i=StartThread(pid, NULL)) < 0) {
			printf("StartThread failed (%d)\n", i);
		}
	} 
	else {
		printf("CreateThread failed (%d)\n", pid);
	}

	sem_info.attr = 1;
	sem_info.option = 1;
	sem_info.initial = 0;
	sem_info.max = 1;

	poffSema = CreateSema(&sem_info);
	if (poffSema <= 0) {
		printf( "CreateSema failed %i\n", poffSema);
		return 1;
	}

	return 0;
}
