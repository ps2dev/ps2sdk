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
#include "pwroff_rpc.h"

//#define DEBUG

#define INT_CDROM	0x02
#define TYPE_C		1
#define CDVDreg_PWOFF	(*(volatile unsigned char*)0xBF402008)

#define MAX_CALLBACKS	8

IRX_ID("Poweroff_Handler", 1, 1);

extern struct irx_export_table _exp_poweroff;

//---------------------------------------------------------------------

int  _start(int, char**);
static void Shutdown();
static void SendCmd(void* data);

//---------------------------------------------------------------------
typedef int (*intrhandler)(void*);

intrhandler	oldCdHandler=0;

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
static pwoffcb poweroff_button_cb = 0;
static void *poweroff_button_data = 0;
static struct t_SifRpcDataQueue qd;
static struct t_SifRpcServerData sd0;

static int myCdHandler(void *param)
{
	if (((CDVDreg_PWOFF & 1)==0) && (CDVDreg_PWOFF & 4))
	{
		/* can't seem to register a sif cmd callback in ps2link so... */
		/* Clear interrupt bit */
		CDVDreg_PWOFF = 4;
#ifdef DEBUG
		printf("Poweroff!!!! %08x\n", CDVDreg_PWOFF);
#endif
		if (poweroff_button_cb)
			poweroff_button_cb(poweroff_button_data);
	}

	return oldCdHandler(param);
}

static void Shutdown(void* data)
{
#ifdef DEBUG
	printf("Shutdown\n");
#endif
	int i;
	/* Do callbacks in reverse order */
	for(i = MAX_CALLBACKS-1; i >= 0; i--)
	{
		if(CallbackTable[i].cb)
		{
			CallbackTable[i].cb(CallbackTable[i].data);
		}
	}

	// Turn off PS2
	*((unsigned char *)0xBF402017) = 0;
	*((unsigned char *)0xBF402016) = 0xF;
}

static void SendCmd(void* data)
{
	isceSifSendCmd(POFF_SIF_CMD, cmdData, 16, NULL, NULL, 0);
}

//---------------------------------------------------------------------
//-----------------------------------------------------------entrypoint
//---------------------------------------------------------------------

void SetPowerButtonHandler(pwoffcb func, void* param)
{
	poweroff_button_cb = func;
	poweroff_button_data = param;
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

void PoweroffShutdown()
{
	Shutdown(0);
}

void* poweroff_rpc_server(int fno, void *data, int size)
{
	switch(fno) {
	case PWROFF_SHUTDOWN:
		Shutdown(0);
		break;
		
	case PWROFF_ENABLE_AUTO_SHUTOFF:
		{
			int* sbuff = data;
			if (sbuff[0])
				SetPowerButtonHandler(Shutdown, 0);
			else
				SetPowerButtonHandler(SendCmd, 0);
			sbuff[0] = 1;
			return sbuff;
		}
	}
	return NULL;
}

void poweroff_rpc_Thread(void* param)
{
	SifInitRpc(0);
	
	SifSetRpcQueue(&qd, GetThreadId());
	SifRegisterRpc(&sd0, PWROFF_IRX, poweroff_rpc_server, cmdData, 0, 0, &qd);
	SifRpcLoop(&qd);
}

int _start(int argc, char* argv[])
{
	register struct handlerTableEntry *handlers=(struct handlerTableEntry*)0x480;//iopmem
	iop_thread_t mythread;
	int i;

	if(RegisterLibraryEntries(&_exp_poweroff) != 0)
	{
		printf("Poweroff already registered\n");
		return 1;
	}
	
	SetPowerButtonHandler(Shutdown, 0);

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

	oldCdHandler=(intrhandler)((int)handlers[INT_CDROM].handler & ~3);
	handlers[INT_CDROM].handler=(intrhandler)((int)myCdHandler | TYPE_C);

	memset(CallbackTable, 0, sizeof(struct CallbackEntry) * MAX_CALLBACKS);

	mythread.attr = 0x02000000;
	mythread.option = 0;
	mythread.thread = poweroff_rpc_Thread;
	mythread.stacksize = 0x1000;
	mythread.priority = 0x27;

	int pid = CreateThread(&mythread);

	if (pid > 0) {
		if ((i=StartThread(pid, NULL)) < 0) {
			printf("StartThread failed (%d)\n", i);
			return 1;
		}
	}
	else {
		printf("CreateThread failed (%d)\n", pid);
		return 1;
	}

	printf("Poweroff installed\n");
	return 0;
}
