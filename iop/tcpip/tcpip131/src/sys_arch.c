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
#include "lwip/pbuf.h"
#include "arch/sys_arch.h"

#if		defined(DEBUG)
#define	dbgprintf(args...)	printf(args)
#else
#define	dbgprintf(args...)	((void)0)
#endif

#define	SYS_THREAD_PRIO_BASE		0x22

static int sema_count = 0;
static int mbox_count = 0;
static int thread_count = 0;

typedef struct Timeout	Timeout;

struct Timeout
{
	struct sys_timeouts	Timeouts;
	int						iTID;
	Timeout*					pNext;
};


#define	SYS_TIMEOUT_MAX	10

static Timeout		aTimeouts[SYS_TIMEOUT_MAX];
static Timeout*	pFreeTimeouts;
static Timeout*	pActiveTimeouts;

// this is probably excessive but since each "message" is only 8 bytes...
// I have no idea how many messages would be queued at once...
#define SYS_MAX_MESSAGES    512

// 4096 bytes
arch_message msg_pool[SYS_MAX_MESSAGES];

arch_message *alloc_msg(void)
{
    int i;
    int oldIntr;

    CpuSuspendIntr(&oldIntr);

    for(i = 0; i < SYS_MAX_MESSAGES; i++)
    {
        if((msg_pool[i].next == NULL) && (msg_pool[i].sys_msg == NULL))
        {
            msg_pool[i].next = (arch_message *) 0xFFFFFFFF;
            msg_pool[i].sys_msg = (void *) 0xFFFFFFFF;
            CpuResumeIntr(oldIntr);
            return(&msg_pool[i]);
        }
    }

    CpuResumeIntr(oldIntr);
    return(NULL);
}

void free_msg(arch_message *msg)
{
    int oldIntr;

    CpuSuspendIntr(&oldIntr);
    msg->next = NULL;
    msg->sys_msg = NULL;
    CpuResumeIntr(oldIntr);
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
	int					iSec;
	int					iUSec;
	int					iDiff;

	Diff.lo=pEnd->lo-pStart->lo;
	Diff.hi=pEnd->hi-pStart->hi;

	SysClock2USec(&Diff, (u32 *)&iSec, (u32 *)&iUSec);
	iDiff=(iSec*1000)+(iUSec/1000);

	return	iDiff != 0 ? iDiff : 1;
}

//Create a new thread.
sys_thread_t sys_thread_new(char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio)
{
	iop_thread_t thp;
	int tid, rv;

    thp.attr = TH_C;
    thp.option = 0;
    thp.thread = thread;
//    thp.stacksize = 0x900; // why this magic number??
    thp.stacksize = stacksize;
    thp.priority = prio + SYS_THREAD_PRIO_BASE;

	dbgprintf("sys_thread_new: Thread new (TID: %d)\n",GetThreadId());

	if((tid = CreateThread(&thp)) < 0)
    {
		dbgprintf("sys_thread_new: CreateThread failed, EC: %d\n", tid);
		return	-1;
	}

	if((rv = StartThread(tid, arg)) < 0)
	{
		dbgprintf("sys_thread_new: StartThread failed, EC: %d\n", rv);
	    DeleteThread(tid);
	    return(-1);
	}

	thread_count++;

	return((sys_thread_t) tid);
}

sys_mbox_t
sys_mbox_new(int size)
{
    iop_mbx_t mbp;
    int mbid;

	dbgprintf("sys_mbox_new: Create MBox (TID: %d)\n",GetThreadId());

    mbp.attr = 0;
    mbp.option = 0;

    if((mbid = CreateMbx(&mbp)) < 0)
    {
		printf("sys_mbox_new: CreateMbx failed, EC: %d\n", mbid);
		return	-1;
    }

	mbox_count++;
	return((sys_mbox_t) mbid);
}


//Delete the messagebox, pMBox.

void
sys_mbox_free(sys_mbox_t pMBox)
{
	dbgprintf("sys_mbox_free: Free MBox (TID: %d)\n",GetThreadId());

	if(pMBox <= 0) { return; }

    // should refer status and see if mbox is empty, if not should give an error...

    DeleteMbx(pMBox);

	mbox_count--;
}

void PostInputMSG(sys_mbox_t mbid, arch_message *msg)
{
	//This function should only be invoked by ps2ip_input. It'll be invoked from an interrupt-context and the pMBox is non-full.
    iSendMbx(mbid, (iop_message_t *) msg);
}


err_t _sys_mbox_post(sys_mbox_t mbid, void *sys_msg, int block)
{
	arch_message *msg;
	//This function should only be invoked by ps2ip_input. It'll be invoked from an interrupt-context and the pMBox is non-full.

    // FIXME: Not sure if this is the best thing to do, will this lock up the only thread which will free up a message??
    while(((msg = alloc_msg()) == NULL) && block)
    {
        DelayThread(100);
    }

    if(msg == NULL)
    {
        return(ERR_MEM);
    }

    msg->sys_msg = sys_msg;
    SendMbx(mbid, (iop_message_t *) msg);

    return(ERR_OK);
}

void sys_mbox_post(sys_mbox_t mbid, void *sys_msg)
{
    _sys_mbox_post(mbid, sys_msg, 1);
}

err_t sys_mbox_trypost(sys_mbox_t mbid, void *sys_msg)
{
    return _sys_mbox_post(mbid, sys_msg, 0);
}

u32_t
_sys_arch_mbox_fetch(sys_mbox_t pMBox,void** ppvMSG,u32_t u32Timeout, int block)
{
    void *pmsg;
    u32 u32Time = 0;
	iop_sys_clock_t	ClockTicks;
	iop_sys_clock_t	Start;
	iop_sys_clock_t	End;

    if(PollMbx(&pmsg, pMBox) != 0)
    {
        if(!block) { return(SYS_MBOX_EMPTY); }

    	int iPID=GetThreadId();

        if(u32Timeout > 0)
        {
    		GetSystemTime(&Start);
    		USec2SysClock(u32Timeout * 1000, &ClockTicks);
    		SetAlarm(&ClockTicks, TimeoutHandler, (void*)iPID);
    	}

        if(ReceiveMbx(&pmsg, pMBox) != 0) { return(SYS_ARCH_TIMEOUT); }

        if(u32Timeout > 0)
        {
    		CancelAlarm(TimeoutHandler,(void*)iPID);
    		GetSystemTime(&End);

    		u32Time = ComputeTimeDiff(&Start,&End);
    	}
    }

    if(ppvMSG) { *ppvMSG = ((arch_message *) pmsg)->sys_msg; }

    free_msg((arch_message *) pmsg);

	//Return the number of msec waited.
	return	u32Time;
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t pMBox,void** ppvMSG,u32_t u32Timeout)
{
    return _sys_arch_mbox_fetch(pMBox, ppvMSG, u32Timeout, 1);
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t pMBox,void** ppvMSG)
{
    return _sys_arch_mbox_fetch(pMBox, ppvMSG, 0, 0);
}

sys_sem_t
sys_sem_new(u8_t u8Count)
{

	//Create a new semaphore.

	iop_sema_t	Sema={1,1,u8Count,1};
	int			iSema;

	dbgprintf("sys_sem_new: CreateSema (TID: %d, CNT: %d)\n",GetThreadId(),u8Count);

	iSema=CreateSema(&Sema);
	if(iSema<=0)
	{
		printf("sys_sem_new: CreateSema failed, EC: %d\n",iSema);
		return	SYS_SEM_NULL;
	}

	++sema_count;

	return	iSema;
}


u32_t
sys_arch_sem_wait(sys_sem_t Sema,u32_t u32Timeout)
{

	//Wait u32Timeout msec for the Sema to receive a signal.

	dbgprintf("sys_arch_sem_wait: Sema: %d, Timeout: %x (TID: %d)\n",Sema,u32Timeout,GetThreadId());

	if(u32Timeout==0)
	{

		//Wait with no timeouts.

		return	WaitSema(Sema)==0 ? 0:SYS_ARCH_TIMEOUT;
	}
	else if(u32Timeout==1)
	{

		//Poll.

		return	PollSema(Sema)==0 ? 0:SYS_ARCH_TIMEOUT;
	}
	else
	{

		//Use alarm to timeout.

		iop_sys_clock_t	ClockTicks;
		iop_sys_clock_t	Start;
		iop_sys_clock_t	End;
		int					iPID=GetThreadId();
		u32_t					u32WaitTime;

		GetSystemTime(&Start);
		USec2SysClock(u32Timeout*1000,&ClockTicks);
		SetAlarm(&ClockTicks,TimeoutHandler,(void*)iPID);

		if(WaitSema(Sema)!=0)
		{
			return	SYS_ARCH_TIMEOUT;
		}
		CancelAlarm(TimeoutHandler,(void*)iPID);
		GetSystemTime(&End);

		u32WaitTime=ComputeTimeDiff(&Start,&End);
		return	u32WaitTime<=u32Timeout ? u32WaitTime:u32Timeout;
	}
}


void
sys_sem_signal(sys_sem_t Sema)
{
	dbgprintf("sys_sem_signal: Sema: %d (TID: %d)\n",Sema,GetThreadId());

	SignalSema(Sema);
}


void
sys_sem_free(sys_sem_t Sema)
{
	dbgprintf("sys_sem_free: Sema: %d (TID: %d)\n",Sema,GetThreadId());

	if(Sema==SYS_SEM_NULL)
	{
		printf("sys_sem_free: Trying to delete illegal sema: %d\n",Sema);
		return;
	}
	DeleteSema(Sema);
	--sema_count;
}

void
sys_init(void)
{
	int i;
	Timeout**	ppTimeout=&pFreeTimeouts;

	dbgprintf("sys_init: Initializing (TID: %d)\n",GetThreadId());

	for(i = 0; i < SYS_MAX_MESSAGES; i++)
	{
	    msg_pool[i].next = NULL;
	    msg_pool[i].sys_msg = NULL;
	}


	for	(i = 0; i < SYS_TIMEOUT_MAX; i++)
	{
		Timeout*		pTimeout=&aTimeouts[i];

		*ppTimeout=pTimeout;
		ppTimeout=&pTimeout->pNext;
	}

	*ppTimeout=NULL;
}


static Timeout**
FindTimeout(int iThreadID)
{

	//Find the Timeout for the thread-id, iThreadID.

	Timeout**	ppTimeout=&pActiveTimeouts;

	while	(*ppTimeout!=NULL)
	{
		if	((*ppTimeout)->iTID==iThreadID)
		{

			//Found it.

			return	ppTimeout;
		}
		ppTimeout=&(*ppTimeout)->pNext;
	}

	//Didn't find it.

	return	ppTimeout;
}


static Timeout**
AllocTimeout(void)
{

	//Allocate a Timeout-struct. Is there any left in the free-list?

	Timeout**	ppTimeout;

	if	(pFreeTimeouts!=NULL)
	{

		//Yes, use the first entry in the free-list.

		return	&pFreeTimeouts;
	}

	//There are no free entries. Then we'll return the LRU-entry, which is the last entry in the active-list.

	ppTimeout=&pActiveTimeouts;
	while	((*ppTimeout)->pNext!=NULL)
	{
		ppTimeout=&(*ppTimeout)->pNext;
	}

	//Before we return the LRU-entry, remove/free the timeout-list.

	while	((*ppTimeout)->Timeouts.next!=NULL)
	{
		struct sys_timeo*	pTimeout=(*ppTimeout)->Timeouts.next;

		(*ppTimeout)->Timeouts.next=pTimeout->next;
		memp_free(MEMP_SYS_TIMEOUT,pTimeout);
	}
	return	ppTimeout;
}


struct sys_timeouts*
sys_arch_timeouts(void)
{

	//Return the timeout-list for this thread.

	int			iThreadID=GetThreadId();
	Timeout**	ppTimeout;
	Timeout*		pTimeout;
	sys_prot_t	Flags=sys_arch_protect();

	//Does it exist an entry for this thread?

	ppTimeout=FindTimeout(iThreadID);
	if	(*ppTimeout==NULL)
	{

		//No, allocate an entry for this thread.

		ppTimeout=AllocTimeout();
		(*ppTimeout)->iTID=iThreadID;
	}

	//The active entries are listed in MRU order. The entry for this thread is the MRU and therefore should be first in the
	//active-list.

	pTimeout=*ppTimeout;
	*ppTimeout=pTimeout->pNext;
	pTimeout->pNext=pActiveTimeouts;
	pActiveTimeouts=pTimeout;

	sys_arch_unprotect(Flags);

	//Return the timeout-list.

	return	&pTimeout->Timeouts;
}

sys_prot_t
sys_arch_protect(void)
{
	sys_prot_t	Flags;

	CpuSuspendIntr(&Flags);
	return	Flags;
}


void
sys_arch_unprotect(sys_prot_t Flags)
{
	CpuResumeIntr(Flags);
}


#if		defined(DEBUG)

void
DumpMBox(sys_mbox_t pMBox)
{
	printf("u16First: %d\n",pMBox->u16First);
	printf("u16Last: %d\n",pMBox->u16Last);
	printf("Mail: %d\n",pMBox->Mail);
	printf("Mutex: %d\n",pMBox->Mutex);
	printf("iWaitPost: %d\n",pMBox->iWaitPost);
	printf("iWaitFetch: %d\n",pMBox->iWaitFetch);
}


void
DumpSysStats(void)
{
	printf("sema_count: %d\n",sema_count);
	printf("mbox_count: %d\n",mbox_count);
	printf("thread_count: %d\n",thread_count);
	printf("Interrupt-context: %s\n",QueryIntrContext()==1 ? "True":"False");
}


static char const*
GetPBufType(int iFlag)
{
	switch	(iFlag)
	{
	case	PBUF_FLAG_RAM:
		return	"PBUF_FLAG_RAM";
	case	PBUF_FLAG_ROM:
		return	"PBUF_FLAG_ROM";
	case	PBUF_FLAG_POOL:
		return	"PBUF_FLAG_POOL";
	case	PBUF_FLAG_REF:
		return	"PBUF_FLAG_REF";
	default:
		return	"Unknown";
	}
}


void
DumpPBuf(struct pbuf* pBuf)
{
	printf("pbuf: %p\n",pBuf);
	printf("pbuf->next: %p\n",pBuf->next);
	printf("pbuf->payload: %p\n",pBuf->payload);
	printf("pbuf->tot_len: %d\n",pBuf->tot_len);
	printf("pbuf->len: %d\n",pBuf->len);
	printf("pbuf->flags: %d (%s)\n",pBuf->flags,GetPBufType(pBuf->flags));
	printf("pbuf->ref: %d\n",pBuf->ref);
}

#endif	//defined(DEBUG)
