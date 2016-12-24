/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: sys_arch.c 1347 2006-08-10 20:10:10Z Herben $
*/

#include <types.h>
#include <stdio.h>
#include <sysmem.h>
#include <thsemap.h>
#include <thbase.h>
#include <thmsgbx.h>
#include <sysclib.h>
#include <intrman.h>

#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "lwip/timeouts.h"
#include "lwip/pbuf.h"
#include "arch/sys_arch.h"

#include "ps2ip_internal.h"

// this is probably excessive but since each <message" is only 8 bytes...
// I have no idea how many messages would be queued at once...
#define SYS_MAX_MESSAGES	512

// 4096 bytes
static arch_message msg_pool[SYS_MAX_MESSAGES];

/* Function prototypes */
static arch_message *alloc_msg(void);
static void free_msg(arch_message *msg);

static int MsgCountSema;

static arch_message *try_alloc_msg(void)
{
	unsigned int i;
	arch_message *message;
	int OldState;

	if(PollSema(MsgCountSema)==0){
		CpuSuspendIntr(&OldState);

		for(i=0,message=NULL; i<SYS_MAX_MESSAGES; i++)
		{
			if((msg_pool[i].next == NULL) && (msg_pool[i].sys_msg == NULL))
			{
				msg_pool[i].next = (arch_message *) 0xFFFFFFFF;
				msg_pool[i].sys_msg = (void *) 0xFFFFFFFF;
				message=&msg_pool[i];
				break;
			}
		}

		CpuResumeIntr(OldState);
	}else message=NULL;

	return message;
}

static arch_message *alloc_msg(void)
{
	unsigned int i;
	arch_message *message;
	int OldState;

	WaitSema(MsgCountSema);

	CpuSuspendIntr(&OldState);

	for(i=0,message=NULL; i<SYS_MAX_MESSAGES; i++)
	{
		if((msg_pool[i].next == NULL) && (msg_pool[i].sys_msg == NULL))
		{
			msg_pool[i].next = (arch_message *) 0xFFFFFFFF;
			msg_pool[i].sys_msg = (void *) 0xFFFFFFFF;
			message=&msg_pool[i];
			break;
		}
	}

	CpuResumeIntr(OldState);

	return message;
}

static void free_msg(arch_message *msg)
{
	int oldIntr;

	CpuSuspendIntr(&oldIntr);
	msg->next = NULL;
	msg->sys_msg = NULL;
	CpuResumeIntr(oldIntr);
	SignalSema(MsgCountSema);
}

static unsigned int
TimeoutHandler(void* pvArg)
{
	iReleaseWaitThread((int)pvArg);
	return	0;
}

static u32_t
ComputeTimeDiff(iop_sys_clock_t* pStart,iop_sys_clock_t* pEnd)
{
	iop_sys_clock_t	Diff;
	u32 iSec, iUSec, iDiff;

	Diff.lo=pEnd->lo-pStart->lo;
	Diff.hi=pEnd->hi-pStart->hi - (pStart->lo>pEnd->lo);

	SysClock2USec(&Diff, &iSec, &iUSec);
	iDiff=(iSec*1000)+(iUSec/1000);

	return((iDiff!=0)?iDiff:1);
}

//Create a new thread.
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	iop_thread_t thp;
	int tid, rv;

	thp.attr = TH_C;
	thp.option = 0;
	thp.thread = thread;
	thp.stacksize = stacksize;
	thp.priority = prio;

	dbgprintf("sys_thread_new: Thread new (TID: %d)\n", GetThreadId());

	if((tid = CreateThread(&thp)) < 0)
	{
		dbgprintf("sys_thread_new: CreateThread failed, EC: %d\n", tid);
		return ERR_MEM;
	}

	if((rv = StartThread(tid, arg)) < 0)
	{
		dbgprintf("sys_thread_new: StartThread failed, EC: %d\n", rv);
		DeleteThread(tid);
		return ERR_MEM;
	}

	return((sys_thread_t) tid);
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	iop_mbx_t mbp;

	dbgprintf("sys_mbox_new: Create MBox (TID: %d)\n", GetThreadId());

	mbp.attr = mbp.option = 0;

	if((*mbox = CreateMbx(&mbp)) < 0)
	{
	    	printf("sys_mbox_new: CreateMbx failed, EC: %d\n", *mbox);
		return ERR_MEM;
	}

	return ERR_OK;
}

//Delete the messagebox, pMBox.
void sys_mbox_free(sys_mbox_t *mbox)
{
	dbgprintf("sys_mbox_free: Free MBox (TID: %d)\n", GetThreadId());
	DeleteMbx(*mbox);
}

int sys_mbox_valid(sys_mbox_t *mbox){
	return(*mbox>=0);
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg){
	arch_message *MsgPkt;
	err_t result;

	if((MsgPkt = try_alloc_msg()) != NULL){
		MsgPkt->sys_msg = msg;
		SendMbx(*mbox, (iop_message_t *)MsgPkt);
		result=ERR_OK;
	}
	else result=ERR_MEM;

	return result;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	arch_message *MsgPkt;
	MsgPkt=alloc_msg();
	MsgPkt->sys_msg = msg;
	SendMbx(*mbox, (iop_message_t *)MsgPkt);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox){
	*mbox=SYS_MBOX_NULL;
}

static u32_t sys_arch_mbox_fetch_internal(sys_mbox_t pMBox, void** ppvMSG, u32_t TimeElaspedout, char block)
{
	void *pmsg;
	u32_t TimeElasped = 0;
	iop_sys_clock_t	ClockTicks;
	iop_sys_clock_t	Start;
	iop_sys_clock_t	End;
	int result, iPID;

	if(block){
		iPID=GetThreadId();

		if(TimeElaspedout > 0)
		{
			GetSystemTime(&Start);
			USec2SysClock(TimeElaspedout * 1000, &ClockTicks);
			SetAlarm(&ClockTicks, &TimeoutHandler, (void*)iPID);
		}

		if((result=ReceiveMbx(&pmsg, pMBox))!= 0) { return(SYS_ARCH_TIMEOUT); }

		if(TimeElaspedout > 0)
		{
			CancelAlarm(TimeoutHandler,(void*)iPID);
			GetSystemTime(&End);

			TimeElasped = ComputeTimeDiff(&Start, &End);
		}
	}
	else{
		TimeElasped=((result=PollMbx(&pmsg, pMBox))!=0)?SYS_MBOX_EMPTY:0;
	}

	if(result==0){
		*ppvMSG = ((arch_message *)pmsg)->sys_msg;
		free_msg((arch_message *) pmsg);
	}

	//Return the number of msec waited.
	return	TimeElasped;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	return(sys_arch_mbox_fetch_internal(*mbox, msg, timeout, 1));
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg){
	return(sys_arch_mbox_fetch_internal(*mbox, msg, 0, 0));
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	//Create a new semaphore.
	iop_sema_t Sema={1, 1, count, 1};
	err_t result;

	dbgprintf("sys_sem_new: CreateSema (TID: %d, CNT: %d)\n", GetThreadId(), count);

	if((*sem=CreateSema(&Sema))<0)
	{
		printf("sys_sem_new: CreateSema failed, EC: %d\n", *sem);
		result=ERR_MEM;
	}
	else result=ERR_OK;

	return result;
}

int sys_sem_valid(sys_sem_t *sem){
	return(*sem>=0);
}

void sys_sem_set_invalid(sys_sem_t *sem){
	*sem=SYS_SEM_NULL;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	//Wait TimeElaspedout msec for the Sema to receive a signal.
	dbgprintf("sys_arch_sem_wait: Sema: %d, Timeout: %x (TID: %d)\n", *sem, timeout, GetThreadId());
	u32_t result;

	if(timeout==0)
	{
		//Wait with no timeouts.
		result=(WaitSema(*sem)==0)?0:SYS_ARCH_TIMEOUT;
	}
	else if(timeout==1)
	{
		//Poll.
		result=(PollSema(*sem)==0)?0:SYS_ARCH_TIMEOUT;
	}
	else
	{
		//Use alarm to timeout.
		iop_sys_clock_t	ClockTicks;
		iop_sys_clock_t	Start;
		iop_sys_clock_t	End;
		int	iPID=GetThreadId();
		u32_t	u32WaitTime;

		GetSystemTime(&Start);
		USec2SysClock(timeout*1000,&ClockTicks);
		SetAlarm(&ClockTicks, &TimeoutHandler, (void*)iPID);

		if(WaitSema(*sem)==0){
			CancelAlarm(TimeoutHandler,(void*)iPID);
			GetSystemTime(&End);

			u32WaitTime=ComputeTimeDiff(&Start, &End);
			result=(u32WaitTime<=timeout)?u32WaitTime:timeout;
		}else result=SYS_ARCH_TIMEOUT;
	}

	return result;
}

void sys_sem_signal(sys_sem_t *sem)
{
	dbgprintf("sys_sem_signal: Sema: %d (TID: %d)\n", *sem, GetThreadId());
	SignalSema(*sem);
}

void sys_sem_free(sys_sem_t *sem)
{
	dbgprintf("sys_sem_free: (TID: %d)\n", GetThreadId());
	DeleteSema(*sem);
}

void sys_init(void)
{
	iop_sema_t sema;

	dbgprintf("sys_init: Initializing (TID: %d)\n", GetThreadId());

	memset(msg_pool, 0, sizeof(msg_pool));

	sema.initial=sema.max=SYS_MAX_MESSAGES;
	sema.attr=sema.option=0;
	MsgCountSema=CreateSema(&sema);
}

sys_prot_t sys_arch_protect(void)
{
	sys_prot_t Flags;

	CpuSuspendIntr(&Flags);
	return	Flags;
}

void sys_arch_unprotect(sys_prot_t Flags)
{
	CpuResumeIntr(Flags);
}

void *malloc(unsigned int size)
{
	int flags;
	void *ptr;

	CpuSuspendIntr(&flags);
	ptr=AllocSysMemory(ALLOC_LAST, size, NULL);
	CpuResumeIntr(flags);

	return ptr;
}

void free(void *ptr)
{
	int flags;

	CpuSuspendIntr(&flags);
	FreeSysMemory(ptr);
	CpuResumeIntr(flags);
}

u32_t sys_now(void)
{
	iop_sys_clock_t	timenow;
	u32 seconds, usecs;

	GetSystemTime(&timenow);
	SysClock2USec(&timenow, &seconds, &usecs);

	return(seconds*1000 + usecs / 1000);
}
