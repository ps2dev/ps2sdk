/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <irx_imports.h>

// Text section hash:
// a9e8a86ab5829f75b845b3e232aca4c9
// Known titles:
// NM00012
// NM00016
// NM00020
// NM00025
// Path strings: /home/ha/setrtc/iop/

typedef union acValue_stru
{
	sceCdCLOCK rtc;
	u8 id[8];
} acValue;

typedef struct acData_stru
{
	int ret;
	acValue data;
} acData;

static acData buffer;
static SifRpcDataQueue_t qd;
static SifRpcServerData_t sd;

static int start_thread(void (*func)(void *arg), int size, int attr, int prio, void *argp)
{
	iop_thread_t thread_param;
	int thid;
	int r;

	thread_param.attr = attr;
	thread_param.thread = func;
	thread_param.priority = prio;
	thread_param.stacksize = size;
	thread_param.option = 0;
	thid = CreateThread(&thread_param);
	if ( thid <= 0 )
	{
		printf("CreateThread (%d)\n", thid);
		return -1;
	}
	r = StartThread(thid, argp);
	if ( r )
	{
		printf("StartThread (%d)\n", r);
		DeleteThread(thid);
		return -1;
	}
	return 0;
}

static void *sce_actest(unsigned int fno, void *data, int size)
{
	int ret;
	acData *datat;

	(void)size;
	datat = (acData *)data;
	switch ( fno )
	{
		case 1:
		{
			ret = sceCdReadClock(&(datat->data.rtc));
			break;
		}
		case 2:
		{
			ret = sceCdWriteClock(&(datat->data.rtc));
			break;
		}
		case 3:
		{
			ret = sceCdRI(datat->data.id, (u32 *)&(datat->ret));
			break;
		}
		case 4:
		{
			ret = sceCdWI(datat->data.id, (u32 *)&(datat->ret));
			break;
		}
		default:
		{
			printf("sce_hddtest: unrecognized code %x\n", fno);
			ret = -1;
			break;
		}
	}
	*(u32 *)data = ret;
	return data;
}

void sce_hddtest_loop(void *arg)
{
	(void)arg;
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd, 0xFFFF, (SifRpcFunc_t)sce_actest, &buffer, 0, 0, &qd);
	sceSifRpcLoop(&qd);
}

int _start(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return start_thread(sce_hddtest_loop, 0x1000, 0x2000000, 32, 0) < 0;
}
