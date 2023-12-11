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
#include <kernel_util.h>
#include <time.h>
#include <limits.h>

#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "lwip/pbuf.h"
#include "arch/sys_arch.h"

#include "ps2ip_internal.h"

static arch_message msg_pool[SYS_MAX_MESSAGES];
static arch_message *free_head;

/* Function prototypes */
static inline arch_message *alloc_msg(void);
static void free_msg(arch_message *msg);
static int WaitSemaTimeout(int sema, unsigned int msec);

static int MsgCountSema;

extern void *_gp;

static inline arch_message *try_alloc_msg(void)
{
	arch_message *message;

	if(PollSema(MsgCountSema)==MsgCountSema)
	{
		DI();

		message = free_head;
		free_head = free_head->next;

		EI();
	}else message=NULL;

	return message;
}

static inline arch_message *alloc_msg(void)
{
	arch_message *message;

	WaitSema(MsgCountSema);
	DI();

	message = free_head;
	free_head = free_head->next;

	EI();

	return message;
}

static void free_msg(arch_message *msg)
{
	DI();

	msg->next = free_head;
	free_head = msg;

	EI();
	SignalSema(MsgCountSema);
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

	(void)name;

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

	(void)size;

	*mbox=SYS_MBOX_NULL;

	if((MBox=malloc(sizeof(struct MboxData)))!=NULL)
	{
		//Last = first, empty mbox.
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

static void RetrieveMbxInternal(sys_mbox_t mBox, arch_message **message)
{
	arch_message *NextMessage;

	DI();

	*message=mBox->FirstMessage;	//Take first message in mbox

	//The next message is next. If there is no next message, NULL is assigned,
	NextMessage=(unsigned int)(*message)->next!=0xFFFFFFFF?(*message)->next:NULL;

	//if the mbox only had one message, then update LastMessage as well.
	if(mBox->FirstMessage == mBox->LastMessage)
		mBox->LastMessage = NULL;

	mBox->FirstMessage = NextMessage;	//The next message becomes the first message. Or NULL, if there are no next messages.

	EI();
}

static int WaitSemaTimeout(int sema, unsigned int msec)
{
	int ret;
	u64 timeoutUsec;
	u64 *timeoutPtr;

	if (msec == 0) {
		if (PollSema(sema) < 0) {
			return -1;
		}
		return sema;
	}

	timeoutPtr = NULL;

	if (msec > 0 && msec != UINT32_MAX) {
		timeoutUsec = msec * 1000;
		timeoutPtr = &timeoutUsec;
	}

	ret = WaitSemaEx(sema, 1, timeoutPtr);

	if (ret < 0)
		return -1; //Timed out.
	return sema; //Wait condition satisfied.
}

static int ReceiveMbx(arch_message **message, sys_mbox_t mBox, u32_t timeout)
{
	int result;

	if(timeout > 0) {
		result = WaitSemaTimeout(mBox->MessageCountSema, timeout);
	} else {
		result = WaitSema(mBox->MessageCountSema);
	}

	if(result == mBox->MessageCountSema)
	{
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
	unsigned int TimeElasped;
	int result;

	TimeElasped=0;
	if(block){
		int start;
		start=clock()/(CLOCKS_PER_SEC/1000);

		if((result=ReceiveMbx(&pmsg, pMBox, u32Timeout))==0){
			TimeElasped = ComputeTimeDiff(start, clock()/(CLOCKS_PER_SEC/1000));
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
	msg->next = NULL;
	if((*mbox)->FirstMessage==NULL) (*mbox)->FirstMessage=msg;	//If this is the first message, it goes at the front.
	if((*mbox)->LastMessage!=NULL) (*mbox)->LastMessage->next=msg;	//Otherwise, it becomes the next message of the last message.
	(*mbox)->LastMessage=msg;					//The message becomes the new message at the end of the queue.

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

u32_t sys_arch_sem_wait(sys_sem_t *sema, u32_t timeout)
{
	u32_t result;

	//Wait timeout msec for the Sema to receive a signal.
	if(timeout==0)
	{
		//Wait with no timeouts.
		result=(WaitSema(*sema)==*sema?0:SYS_ARCH_TIMEOUT);
	}
	else if(timeout==1)
	{
		//Poll.
		result=(PollSema(*sema)==*sema?0:SYS_ARCH_TIMEOUT);
	}
	else
	{
		//Use alarm to timeout.
		unsigned int start;
		u32_t	WaitTime;

		start=clock()/(CLOCKS_PER_SEC/1000);
		if(WaitSemaTimeout(*sema, timeout) == *sema)
		{
			WaitTime=ComputeTimeDiff(start, clock()/(CLOCKS_PER_SEC/1000));
			result=(WaitTime <= timeout ? WaitTime : timeout);
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
	arch_message *prev;
	unsigned int i;
	ee_sema_t sema;

	dbgprintf("sys_init: Initializing...\n");

	sema.attr = 0;
	sema.option = (u32)"PS2IP";
	sema.init_count = sema.max_count = SYS_MAX_MESSAGES;
	MsgCountSema=CreateSema(&sema);

	free_head = &msg_pool[0];
	prev = &msg_pool[0];

	for(i = 1; i < SYS_MAX_MESSAGES; i++)
	{
		prev->next = &msg_pool[i];
		prev = &msg_pool[i];
	}

	//NULL-terminate free message list
	prev->next = NULL;
}

u32_t sys_now(void)
{
	return(clock()/(CLOCKS_PER_SEC/1000));
}

sys_prot_t sys_arch_protect(void)
{
	return DIntr();
}

void sys_arch_unprotect(sys_prot_t level)
{
	if(level)
		EIntr();
}

void *ps2ip_calloc64(size_t n, size_t size)
{
	void *ptr = NULL;
	size_t sz = n * size;

	if ((ptr = memalign(64, sz)) == NULL)
		return ptr;

	memset(ptr, 0, sz);
	return ptr;
}
