/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <kernel.h>

#include "ExecPS2.h"

static int *p_ThreadID=(int*)0x800125EC;	//Pointer to a variable containing the running thread's ID.
static int *p_ThreadStatus=(int*)0x800125F4;	//Pointer to a variable containing the running thread's status.
static int (*p_CancelWakeupThread)(int ThreadID)=(void*)0x80004970;
static int (*p_ChangeThreadPriority)(int ThreadID, int priority)=(void*)0x80004288;
static int (*p_InitPgifHandler2)(void)=(void*)0x800021b0;
static int (*p_InitSemaphores)(void)=(void*)0x80004e68;
static int (*p_DeleteThread)(int thread_id)=(void*)0x80003f00;
static int (*p_TerminateThread)(int ThreadID)=(void*)0x80003e00;
static struct TCB *p_TCBs=(struct TCB *)0x80017400;
static void (*p_InitializeINTC)(int interrupts)=(void*)0x8000b8d0;
static void (*p_InitializeTIMER)(void)=(void*)0x8000b900;
static void (*p_InitializeFPU)(void)=(void*)0x8000b7a8;
static void (*p_InitializeScratchPad)(void)=(void*)0x8000b840;
static int (*p_ResetEE)(int flags)=(void*)0x8000ad68;
static void (*p_InitializeGS)(void)=(void*)0x8000aa60;
static void (*p_SetGSCrt)(unsigned short int interlace, unsigned short int mode, unsigned short int ffmd)=(void*)0x8000a060;

#ifndef REUSE_EXECPS2

static void (*p_FlushDCache)(void)=(void*)0x80002a80;
static void (*p_FlushICache)(void)=(void*)0x80002ac0;
static char *(*p_eestrcpy)(char* dst, const char* src)=(void*)0x80005560;
//static void *p_unk80012600=(void*)0x80012600;	//Not sure what this is, as it isn't used by the patch.
static char *p_ArgsBuffer=(char*)0x80012608;

#else

static void* (*p_ExecPS2)(void *entry, void *gp, int argc, char *argv[])=(void*)0x800057E8;	//Not part of the SONY patch. See below.

#endif

//Taken from PCSX2's FPS2BIOS
struct TCB { //internal struct
	struct TCB	*next; //+00
	struct TCB	*prev; //+04
	int 		status;//+08
	void		(*entry)(void*); //+0C
	void		*stack_res;		//+10 initial $sp
	void		*gpReg; //+14
	short		currentPriority; //+18
	short		initPriority; //+1A
	int		waitSema, //+1C waitType?
                semaId, //+20
                wakeupCount, //+24
                attr, //+28
                option; //+2C
	void		(*entry_)(void*); //+30
	int		argc; //+34
	char		*argstring;//+38
	void		*stack;//+3C
	int		stackSize; //+40
	int		(*root)(); //+44
	void*		heap_base; //+48
};

static inline void SoftPeripheralEEReset(void){
	*(volatile unsigned int *)0x1000f000=4;
	while((*(volatile unsigned int *)0x1000f000&4)==0){};
	*(volatile unsigned int *)0x1000f000=4;

	p_InitializeGS();
	p_SetGSCrt(1, 2, 1);	//Interlaced, NTSC, field mode.
	p_InitializeINTC(0xdffd);
	p_InitializeTIMER();
	p_ResetEE(0x7F);
	p_InitializeFPU();
	p_InitializeScratchPad();
}

void *ExecPS2Patch(void *EntryPoint, void *gp, int argc, char *argv[]){
	int i, CurrentThreadID;
	struct TCB *tcb;
#ifndef REUSE_EXECPS2
	char *ArgsPtr;
#endif

	CurrentThreadID=*p_ThreadID;
	p_CancelWakeupThread(CurrentThreadID);
	p_ChangeThreadPriority(CurrentThreadID, 0);

	//Like IOP kernels, the first thread is the idle thread.
	for(i=1,tcb=&p_TCBs[1]; i<MAX_THREADS; i++,tcb++){
		if(tcb->status!=0 && i!=CurrentThreadID){
			if(tcb->status!=THS_DORMANT) p_TerminateThread(i);

			p_DeleteThread(i);
		}
	}

	p_InitSemaphores();
	p_InitPgifHandler2();

	*p_ThreadStatus=0;
	SoftPeripheralEEReset();

#ifdef REUSE_EXECPS2
	//Unlike the Sony patch, why don't we just reuse the original function in the kernel?
	return p_ExecPS2(EntryPoint, gp, argc, argv);
#else
	for(i=0,ArgsPtr=p_ArgsBuffer; i<argc; i++){
		ArgsPtr=p_eestrcpy(ArgsPtr, argv[i]);
	}

	tcb=&p_TCBs[CurrentThreadID];
	tcb->argstring=p_ArgsBuffer;
	tcb->argc=argc;
	tcb->entry=EntryPoint;
	tcb->entry_=EntryPoint;
	tcb->gpReg=gp;
	tcb->initPriority=0;
	tcb->currentPriority=0;
	tcb->wakeupCount=0;
	tcb->waitSema=0;
	tcb->semaId=0;

	p_FlushICache();
	p_FlushDCache();

	return EntryPoint;
#endif
}

