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

#define DEBUG

#ifdef DEBUG
#include "thsemap.h"
#endif

#define INT_CDROM	0x02
#define TYPE_C		1
#define CDVDreg_PWOFF	(*(volatile unsigned char*)0xBF402008)

#define POFF_SIF_CMD	20

IRX_ID("Poweroff_Handler", 1, 1);

//---------------------------------------------------------------------

typedef void (*pwoffcb)(void*);

int  _start(int, char**);

//---------------------------------------------------------------------
typedef int (*intrhandler)(void*);

intrhandler	old=0;

struct handlerTableEntry{
	intrhandler	handler;
	void		*param;
};

//---------------------------------------------------------------------

char cmdData[16];
int pid;
#ifdef DEBUG
int poffSema;
#endif

int myCdHandler(void *param)
{

	if (((CDVDreg_PWOFF & 1)==0) && (CDVDreg_PWOFF & 4)) 
	{
		isceSifSendCmd(POFF_SIF_CMD, cmdData, 16, NULL, NULL, 0);
#ifdef DEBUG
		iSignalSema(poffSema);
#endif
	}

	return old(param);
}

//---------------------------------------------------------------------
//-----------------------------------------------------------entrypoint
//---------------------------------------------------------------------

#ifdef DEBUG
void pDebugThread(void *arg)
{
	while(1)
	{
		WaitSema(poffSema);
		printf("Poweroff!!!!\n");
	}
}
#endif

int _start(int argc, char* argv[])
{
	register struct handlerTableEntry *handlers=(struct handlerTableEntry*)0x480;//iopmem
#ifdef DEBUG
	iop_thread_t mythread;
	iop_sema_t sem_info;
	int i;
#endif

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

#ifdef DEBUG
	mythread.attr = 0x02000000;
	mythread.option = 0;
	mythread.thread = pDebugThread;
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
#endif

	return 0;
}
