/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <timer.h>
#include <limits.h>

#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "lwip/pbuf.h"
#include "arch/sys_arch.h"

#include "ps2ip_internal.h"

static arch_message msg_pool[SYS_MAX_MESSAGES];

/* Function prototypes */
static inline arch_message *alloc_msg(void);
static void free_msg(arch_message *msg);

static int MsgCountSema;
static int ProtLevel;

extern void *_gp;

static inline arch_message *try_alloc_msg(void)
{
	unsigned int i;
	arch_message *message;

	if(PollSema(MsgCountSema)==MsgCountSema){
		DI();

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

		EI();
	}else message=NULL;

	return message;
}

static inline arch_message *alloc_msg(void)
{
	unsigned int i;
	arch_message *message;

	WaitSema(MsgCountSema);
	DI();

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

	EI();

	return message;
}

static void free_msg(arch_message *msg)
{
	DI();
	msg->next = NULL;
	msg->sys_msg = NULL;
	EI();
	SignalSema(MsgCountSema);
}

static void TimeoutHandler(s32 alarm_id, u16 time, void *pvArg){
	iReleaseWaitThread((int)pvArg);
}

static inline u32_t ComputeTimeDiff(u32 start, u32 end)
{
	u32 NumTicksElasped=(end<start)?UINT_MAX-start+end:start-end;

	return(NumTicksElasped/295000);
}

//Create a new thread.
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	ee_thread_t thp;
	int tid, rv;

	thp.attr = 0;
	thp.option = 0;
	thp.func = thread;
	thp.stack_size = stacksize;
	thp.stack = malloc(stacksize);
	thp.initial_priority = prio;
	thp.gp_reg = &_gp;

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

	dbgprintf("sys_thread_new(): %d\n", tid);

	return((sys_thread_t)tid);
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	struct MboxData *MBox;
	ee_sema_t sema;

	*mbox=SYS_MBOX_NULL;

	if((MBox=malloc(sizeof(struct MboxData)))!=NULL)
	{
		MBox->LastMessage=MBox->FirstMessage=NULL;

		sema.attr = 0;
		sema.option = (u32)"PS2IP-msgcount";
		sema.init_count=0;
		sema.max_count=SYS_MAX_MESSAGES;
		if((MBox->MessageCountSema=CreateSema(&sema))<0)
		{
			printf("sys_mbox_new: CreateMbx failed. Code: %d\n", MBox->MessageCountSema);
			free(MBox);
			return ERR_MEM;
		}
	}
	else{
		printf("sys_mbox_new: Out of memory.\n");
		return ERR_MEM;
	}

	dbgprintf("sys_mbox_new(): sema %d\n", MBox->MessageCountSema);

	*mbox=MBox;

	return ERR_OK;
}

//Delete the messagebox, (*pMBox).
void sys_mbox_free(sys_mbox_t *pMBox)
{
	arch_message *Message, *NextMessage;

	/* Free all messages that were not freed yet. */
	Message=(*pMBox)->FirstMessage;
	while(Message!=NULL)
	{
		NextMessage=Message->next;
		free_msg(Message);
		Message=NextMessage;
	}

	/* Delete all allocated resources for this message box and mark the message box as invalid. */
	DeleteSema((*pMBox)->MessageCountSema);
	free((*pMBox));
	(*pMBox)=SYS_MBOX_NULL;
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
	return((*mbox)!=SYS_MBOX_NULL);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	*mbox=SYS_MBOX_NULL;
}

extern unsigned short int hsyncTicksPerMSec;

static inline unsigned int mSec2HSyncTicks(unsigned int msec)
{
	return msec*hsyncTicksPerMSec;
}

static void RetrieveMbxInternal(sys_mbox_t mBox, arch_message **message)
{
	arch_message *NextMessage;

	DI();

	*message=mBox->FirstMessage;
	NextMessage=(unsigned int)(*message)->next!=0xFFFFFFFF?(*message)->next:NULL;
	if(mBox->FirstMessage==mBox->LastMessage)
		mBox->LastMessage=NextMessage;
	mBox->FirstMessage=NextMessage;

	EI();
}

static int ReceiveMbx(arch_message **message, sys_mbox_t mBox, u32_t timeout)
{
	int result, AlarmID;

	if(timeout > 0)
    		AlarmID=SetAlarm(mSec2HSyncTicks(timeout), &TimeoutHandler, (void*)GetThreadId());
	else
		AlarmID=-1;

	if(WaitSema(mBox->MessageCountSema)==mBox->MessageCountSema)
	{
		if(AlarmID>=0) ReleaseAlarm(AlarmID);

		RetrieveMbxInternal(mBox, message);
		result=0;
	}
	else result=-1;

	return result;
}

static int PollMbx(arch_message **message, sys_mbox_t mBox)
{
	int result;

	if(PollSema(mBox->MessageCountSema)==mBox->MessageCountSema)
	{
		RetrieveMbxInternal(mBox, message);
		result=0;
	}
	else result=-1;

	return result;
}

static u32_t sys_arch_mbox_fetch_internal(sys_mbox_t pMBox, void** ppvMSG, u32_t u32Timeout, char block)
{
	arch_message *pmsg;
	unsigned int TimeElasped, start;
	int result;

	TimeElasped=0;
	if(block){
		start=cpu_ticks();

		if((result=ReceiveMbx(&pmsg, pMBox, u32Timeout))==0){
			TimeElasped = ComputeTimeDiff(start, cpu_ticks());
		}
		else{
			return SYS_ARCH_TIMEOUT;
		}
	}
	else{
		TimeElasped=((result=PollMbx(&pmsg, pMBox))!=0)?SYS_MBOX_EMPTY:0;
	}

	if(result==0){
		if(ppvMSG!=NULL) *ppvMSG = pmsg->sys_msg;
		free_msg(pmsg);
	}

	//Return the number of msec waited.
	return TimeElasped;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *pMBox, void** ppvMSG, u32_t u32Timeout)
{
	return sys_arch_mbox_fetch_internal(*pMBox, ppvMSG, u32Timeout, 1);
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *pMBox, void** ppvMSG)
{
	return sys_arch_mbox_fetch_internal(*pMBox, ppvMSG, 0, 0);
}

static void SendMbx(sys_mbox_t *mbox, arch_message *msg, void *sys_msg){
	DI();

	/* Store the message and update the message chain for this message box. */
	msg->sys_msg = sys_msg;
	if((*mbox)->FirstMessage==NULL) (*mbox)->FirstMessage=msg;
	if((*mbox)->LastMessage!=NULL) (*mbox)->LastMessage->next=msg;
	(*mbox)->LastMessage=msg;

	EI();
	SignalSema((*mbox)->MessageCountSema);
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *sys_msg)
{
	arch_message *msg;
	err_t result;

	/* Attempt to allocate one more message. */
	if((msg=try_alloc_msg())!=NULL){
		SendMbx(mbox, msg, sys_msg);

		result=ERR_OK;
	}
	else result=ERR_MEM;

	return result;
}

void sys_mbox_post(sys_mbox_t *mbox, void *sys_msg)
{
	SendMbx(mbox, alloc_msg(), sys_msg);
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	//Create a new semaphore.
	ee_sema_t sema;

	sema.init_count = count;
	sema.max_count = 1;
	sema.attr = 0;
	sema.option = (u32)"PS2IP";

	if((*sem=CreateSema(&sema))<0)
	{
		printf("sys_sem_new: CreateSema failed, EC: %d\n", *sem);
		return ERR_MEM;
	}

	dbgprintf("sys_sem_new: CreateSema (CNT: %d) %d\n", count, *sem);

	return ERR_OK;
}

u32_t sys_arch_sem_wait(sys_sem_t *Sema, u32_t u32Timeout)
{
	u32_t result;

	//Wait u32Timeout msec for the Sema to receive a signal.
	if(u32Timeout==0)
	{
		//Wait with no timeouts.
		result=(WaitSema(*Sema)==*Sema?0:SYS_ARCH_TIMEOUT);
	}
	else if(u32Timeout==1)
	{
		//Poll.
		result=(PollSema(*Sema)==*Sema?0:SYS_ARCH_TIMEOUT);
	}
	else
	{
		//Use alarm to timeout.
		unsigned int start;
		int	AlarmID;
		u32_t	WaitTime;

		start=cpu_ticks();
		AlarmID=SetAlarm(mSec2HSyncTicks(u32Timeout), &TimeoutHandler, (void*)GetThreadId());

		if(WaitSema(*Sema)==*Sema)
		{
			ReleaseAlarm(AlarmID);

			WaitTime=ComputeTimeDiff(start, cpu_ticks());
			result=(WaitTime<=u32Timeout?WaitTime:u32Timeout);
		}
		else result=SYS_ARCH_TIMEOUT;
	}

	return result;
}

void sys_sem_signal(sys_sem_t *Sema)
{
	SignalSema(*Sema);
}

void sys_sem_free(sys_sem_t *Sema)
{
	DeleteSema(*Sema);
}

int sys_sem_valid(sys_sem_t *sem){
	return(*sem>=0);
}

void sys_sem_set_invalid(sys_sem_t *sem){
	*sem=SYS_SEM_NULL;
}

void sys_init(void)
{
	unsigned int i;
	ee_sema_t sema;

	dbgprintf("sys_init: Initializing...\n");

	sema.attr = 0;
	sema.option = (u32)"PS2IP";
	sema.init_count = sema.max_count = SYS_MAX_MESSAGES;
	MsgCountSema=CreateSema(&sema);

	for(i = 0; i < SYS_MAX_MESSAGES; i++)
	{
		msg_pool[i].next = NULL;
		msg_pool[i].sys_msg = NULL;
	}

	ProtLevel = 0;
}

u32_t sys_now(void)
{
	return(cpu_ticks()/295000);
}

sys_prot_t sys_arch_protect(void)
{
	sys_prot_t OldLevel;

	DI();

	OldLevel = ProtLevel;
	ProtLevel++;

	return OldLevel;
}

void sys_arch_unprotect(sys_prot_t level)
{
	ProtLevel = level;
	if(ProtLevel == 0)
		EI();
}
