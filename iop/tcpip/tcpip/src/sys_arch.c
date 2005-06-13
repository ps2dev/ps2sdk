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


struct sys_mbox_msg
{
	struct sys_mbox_msg*		pNext;
	void*							pvMSG;
};

#define	SYS_THREAD_PRIO_BASE		0x22
#define	SYS_MBOX_SIZE				64

struct sys_mbox
{
	u16_t			u16First;
	u16_t			u16Last;
	void*			apvMSG[SYS_MBOX_SIZE];
	sys_sem_t	Mail;
	sys_sem_t	Mutex;
	int			iWaitPost;
	int			iWaitFetch;
};


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
static u16			u16SemaCNT;
static u16			u16MBoxCNT;
static u16			u16ThreadCNT;


sys_thread_t
sys_thread_new(void (*pFunction)(void* pvArg),void* pvArg,int iPrio)
{

	//Create a new thread.

	iop_thread_t	Info={TH_C,0,pFunction,0x900,iPrio+SYS_THREAD_PRIO_BASE};
	int				iThreadID;

	dbgprintf("sys_thread_new: Thread new (TID: %d)\n",GetThreadId());

	iThreadID=CreateThread(&Info);
	if	(iThreadID<0)
   {
		printf("sys_thread_new: CreateThread failed, EC: %d\n",iThreadID);
		return	-1;
	}

	StartThread(iThreadID,pvArg);
	++u16ThreadCNT;

	return	iThreadID;
}


sys_mbox_t
sys_mbox_new(void)
{

	//Create a new messagebox.

	sys_mbox_t	pMBox;

	dbgprintf("sys_mbox_new: Create MBox (TID: %d).\n",GetThreadId());

	pMBox=(sys_mbox_t)AllocSysMemory(0,sizeof(struct sys_mbox),0);
	if	(pMBox==NULL)
	{
		printf("sys_mbox_new: Out of memory\n");
		return	NULL;
	}
	pMBox->u16First=pMBox->u16Last=0;
	pMBox->Mail=sys_sem_new(0);
	pMBox->Mutex=sys_sem_new(1);
	pMBox->iWaitPost=pMBox->iWaitFetch=0;
	++u16MBoxCNT;
	return	pMBox;
}


void
sys_mbox_free(sys_mbox_t pMBox)
{

	//Delete the messagebox, pMBox.

	dbgprintf("sys_mbox_free: Free MBox (TID: %d)\n",GetThreadId());

	if	(pMBox==NULL)
	{
		return;
	}

	sys_arch_sem_wait(pMBox->Mutex,0);

	sys_sem_free(pMBox->Mail);
	sys_sem_free(pMBox->Mutex);
	pMBox->Mail=pMBox->Mutex=SYS_SEM_NULL;
	FreeSysMemory(pMBox);
	--u16MBoxCNT;
}


static u16_t
GenNextMBoxIndex(u16_t u16Index)
{
	return	(u16Index+1)%SYS_MBOX_SIZE;
}


int
IsMessageBoxFull(sys_mbox_t pMBox)
{
	return	GenNextMBoxIndex(pMBox->u16Last)==pMBox->u16First;
}


int
IsMessageBoxEmpty(sys_mbox_t pMBox)
{
	return	pMBox->u16Last==pMBox->u16First;
}


void
PostInputMSG(sys_mbox_t pMBox,void* pvMSG)
{

	//This function should only be invoked by ps2ip_input. It'll be invoked from an interrupt-context and the pMBox is non-full.
	//Start with storing the message last in the message-queue.

	pMBox->apvMSG[pMBox->u16Last]=pvMSG;
	pMBox->u16Last=GenNextMBoxIndex(pMBox->u16Last);

	//Is there a thread waiting for the arrival of a message in the mbox?

	if	(pMBox->iWaitFetch>0)
	{

		//Yes, signal the Mail-semaphore that a message has arrived.

		iSignalSema(pMBox->Mail);
	}
}


void
sys_mbox_post(sys_mbox_t pMBox,void* pvMSG)
{

	//Post the message, pvMSG, in the messagebox, pMBox.

	sys_prot_t	Flags;

	dbgprintf("sys_mbox_post: MBox Post (TID: %d)\n",GetThreadId());

	if	(pMBox==NULL)
	{
		return;
	}

	Flags=sys_arch_protect();

	//Is the mbox full?

	while	(IsMessageBoxFull(pMBox))
	{

		//Yes, wait for the mbox to become non-full.

		u32_t		u32WaitTime;

		dbgprintf("sys_mbox_post: MBox full\n");

		//Increase the waitpost-count to indicate that there is a thread waiting to for the mbox to become non-full.

		++pMBox->iWaitPost;
		sys_arch_unprotect(Flags);

		//Wait for a signal indicating that the mbox has become non-full.

		u32WaitTime=sys_arch_sem_wait(pMBox->Mail,0);

		//Decrease the waitpost-count, since we aren't waiting anymore.

		Flags=sys_arch_protect();
		--pMBox->iWaitPost;

		//Has the mbox become non-full or did sys_arch_sem_wait fail?

		if	(u32WaitTime==SYS_ARCH_TIMEOUT) 
		{

			//sys_arch_sem_wait failed, exit!

			sys_arch_unprotect(Flags);
			return;
		}

		//We've received a signal that the mbox has become non-full.
	}

	//Now, the mbox isn't full, store the message last in the mbox.

	pMBox->apvMSG[pMBox->u16Last]=pvMSG;
	pMBox->u16Last=GenNextMBoxIndex(pMBox->u16Last);

	//Is there a thread waiting for the arrival of a message in the mbox?

	if	(pMBox->iWaitFetch>0)
	{

		//Yes, send it a signal that one has arrived.

		sys_sem_signal(pMBox->Mail);
	}

	sys_arch_unprotect(Flags);
}


u32_t
sys_arch_mbox_fetch(sys_mbox_t pMBox,void** ppvMSG,u32_t u32Timeout)
{

	//Retrieve the first message in the messagebox, pMBox.

	sys_prot_t	Flags;
	u32_t			u32Time=0;

	if	(pMBox==NULL)
	{
		if	(ppvMSG!=NULL)
		{
			*ppvMSG=NULL;
		}
		return	SYS_ARCH_TIMEOUT;
	}

	dbgprintf("sys_arch_mbox_fetch: MBox fetch (TID: %d, MTX: %x)\n",GetThreadId(),pMBox->Mutex);

	Flags=sys_arch_protect();

	//Is the mbox empty?

	while	(IsMessageBoxEmpty(pMBox))
	{
		u32_t		u32WaitTime;

		dbgprintf("sys_mbox_post: MBox empty\n");

		//Yes, increase the waitfetch-count to indicate that there is a thread waiting to for the mbox to receive a message.

		++pMBox->iWaitFetch;
		sys_arch_unprotect(Flags);

		//Wait for a message to arrive.

		u32WaitTime=sys_arch_sem_wait(pMBox->Mail,u32Timeout);

		//Decrease the waitfetch-count since we aren't waiting anymore.

		Flags=sys_arch_protect();
		--pMBox->iWaitFetch;

		//Has the mbox received a new message or did we time out?

		if	(u32WaitTime==SYS_ARCH_TIMEOUT) 
		{

			//The sys_arch_sem_wait timed out. Return SYS_ARCH_TIMEOUT to indicate that a timeout occured.

			sys_arch_unprotect(Flags);
			return	SYS_ARCH_TIMEOUT;
		}

		//We've received a signal that a message has arrived. Add the time we spent waiting to the total time we've spent trying to
		//fetch this message.

		u32Time+=u32WaitTime;

		//In the unlikely event the mbox has become empty again, decrease the timeout time with the time we spend waiting.

		u32Timeout-=u32WaitTime;
	}

	//Now, there is atleast one message in the mbox, retrieve the first message!

	if	(ppvMSG!=NULL)
	{
		LWIP_DEBUGF(SYS_DEBUG,("sys_arch_mbox_fetch: MBox: %p MSG: %p\n",pMBox,*ppvMSG));
		*ppvMSG=pMBox->apvMSG[pMBox->u16First];
	}
	pMBox->u16First=GenNextMBoxIndex(pMBox->u16First);

	//Is there a thread waiting for the mbox to become non-full?

	if	(pMBox->iWaitPost>0)
	{

		//Yes, send it a signal that the mbox is non-full.

		sys_sem_signal(pMBox->Mail);
	}    

	sys_arch_unprotect(Flags);

	//Return the number of msec waited.

	return	u32Time;
}     


sys_sem_t
sys_sem_new(u8_t u8Count)
{

	//Create a new semaphore.

	iop_sema_t	Sema={1,1,u8Count,1};
	int			iSema;

	dbgprintf("sys_sem_new: CreateSema (TID: %d, CNT: %d)\n",GetThreadId(),u8Count);

	iSema=CreateSema(&Sema);
	if	(iSema<=0)
	{
		printf("sys_sem_new: CreateSema failed, EC: %d\n",iSema);
		return	SYS_SEM_NULL;
	}

	++u16SemaCNT;

	return	iSema;
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

	return	iDiff!=0 ? iDiff:1;
}


u32_t
sys_arch_sem_wait(sys_sem_t Sema,u32_t u32Timeout)
{

	//Wait u32Timeout msec for the Sema to receive a signal.

	dbgprintf("sys_arch_sem_wait: Sema: %d, Timeout: %x (TID: %d)\n",Sema,u32Timeout,GetThreadId());

	if	(u32Timeout==0)
	{ 

		//Wait with no timeouts.

		return	WaitSema(Sema)==0 ? 0:SYS_ARCH_TIMEOUT;
	}
	else if	(u32Timeout==1)
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

		if	(WaitSema(Sema)!=0)
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

	if	(Sema==SYS_SEM_NULL)
	{
		printf("sys_sem_free: Trying to delete illegal sema: %d\n",Sema);
		return;
	}
	DeleteSema(Sema);
	--u16SemaCNT;
}


void
sys_init(void)
{
	int			iA;
	Timeout**	ppTimeout=&pFreeTimeouts;

	dbgprintf("sys_init: Initializing (TID: %d)\n",GetThreadId());

	for	(iA=0;iA<SYS_TIMEOUT_MAX;++iA)
	{
		Timeout*		pTimeout=&aTimeouts[iA];
		
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
		struct sys_timeout*	pTimeout=(*ppTimeout)->Timeouts.next;

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

static int
CountTimeouts(Timeout* pTimeout)
{
	int	iCNT=0;

	while	(pTimeout!=NULL)
	{
		++iCNT;
		pTimeout=pTimeout->pNext;
	}
	return	iCNT;
}


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
	printf("u16SemaCNT: %d\n",u16SemaCNT);
	printf("u16MBoxCNT: %d\n",u16MBoxCNT);
	printf("u16ThreadCNT: %d\n",u16ThreadCNT);
	printf("FreeTimeouts: %d (%d)\n",CountTimeouts(pFreeTimeouts),SYS_TIMEOUT_MAX);
	printf("ActiveTimeouts: %d (%d)\n",CountTimeouts(pActiveTimeouts),SYS_TIMEOUT_MAX);
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
