/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# EE kernel glue and utility routines.
*/

#include "kernel.h"

struct request{
	u8 mode;
	u8 data;
};

enum TOP_REQ{
	TOP_REQ_WAKEUP,
	TOP_REQ_ROTATE,
	TOP_REQ_SUSPEND
};

struct topArg {
	int requestOut;
	int requestIn;
	struct request request[512];
};

extern int topId;
extern int topSema;
extern struct topArg topArg;

#ifdef F___glue_internals

extern void *_gp;

u8 stack[0x400] __attribute__((aligned(16)));

int topId = 0;
int topSema = 0;
struct topArg topArg = {0};

static void topThread(void *arg)
{
	int index;

	while(1)
	{
		WaitSema(topSema);
		index = topArg.requestOut & 0x1FF;
		topArg.requestOut = index + 1;

		switch(topArg.request[index].mode)
		{
			case TOP_REQ_WAKEUP:
				WakeupThread(topArg.request[index].data);
				break;
			case TOP_REQ_ROTATE:
				RotateThreadReadyQueue(topArg.request[index].data);
				break;
			case TOP_REQ_SUSPEND:
				SuspendThread(topArg.request[index].data);
				break;
			/* default:
				Kprintf("## internal error in libkernel!\n"); */
		}
	}
}

int InitThread(void)
{
	ee_sema_t sema;
	ee_thread_t thread;

	sema.max_count = 255;
	sema.init_count = 0;
	sema.option = (u32)"KernelTopThread";
	if((topSema = CreateSema(&sema)) < 0)
		return -1;

	thread.func = &topThread;
	thread.stack = stack;
	thread.stack_size = sizeof(stack);
	thread.gp_reg = &_gp;
	thread.option = (u32)"KernelTopThread";
	thread.initial_priority = 0;
	if((topId = CreateThread(&thread)) < 0)
	{
		DeleteSema(topSema);
		return -1;
	}

	topArg.requestOut = 0;
	topArg.requestIn = 0;
	StartThread(topId, &topArg);

	ChangeThreadPriority(GetThreadId(), 1);

	return topId;
}
#endif

#ifdef F_iWakeupThread
//The original iWakeupThread cannot wake up threads in THS_RUN state.
s32 iWakeupThread(s32 thread_id)
{
	int index;

	if(_iGetThreadId() == thread_id)
	{
		if(thread_id < 256 && topId != 0)
		{
			index = topArg.requestIn & 0x1FF;
			topArg.requestIn = index + 1;
			topArg.request[index].mode = TOP_REQ_WAKEUP;
			topArg.request[index].data = thread_id;

			iSignalSema(topSema);
			return 0;
		} else
			return -1;
	} else {
		return _iWakeupThread(thread_id);
	}
}
#endif

#ifdef F_iRotateThreadReadyQueue
//The original iRotateThreadReadyQueue will not change the current thread ID.
s32 iRotateThreadReadyQueue(s32 priority)
{
	int index;

	if(priority < 128 && topId != 0)
	{
		index = topArg.requestIn & 0x1FF;
		topArg.requestIn = index + 1;
		topArg.request[index].mode = TOP_REQ_ROTATE;
		topArg.request[index].data = priority;

		iSignalSema(topSema);
		return 0;
	} else
		return -1;
}
#endif

#ifdef F_iSuspendThread
//The original iSuspendThread allows a thread to suspend itself, but won't change the current thread ID.
s32 iSuspendThread(s32 thread_id)
{
	int index;

	if(_iGetThreadId() == thread_id)
	{
		if(thread_id < 256 && topId != 0)
		{
			index = topArg.requestIn & 0x1FF;
			topArg.requestIn = index + 1;
			topArg.request[index].mode = TOP_REQ_SUSPEND;
			topArg.request[index].data = thread_id;

			iSignalSema(topSema);
			return 0;
		} else
			return -1;
	} else {
		return _iSuspendThread(thread_id);
	}
}
#endif

#ifdef F_DIntr
int DIntr()
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;
	res = eie != 0;

	if (!eie)
		return 0;

	asm (".p2align 3");
	do {
		asm volatile ("di");
		asm volatile ("sync.p");
		asm volatile ("mfc0\t%0, $12" : "=r" (eie));
		eie &= 0x10000;
	} while (eie);

	return res;
}
#endif

#ifdef F_EIntr
int EIntr()
{
	int eie;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;
	asm volatile ("ei");

	return eie != 0;
}
#endif

#ifdef F_EnableIntc
int EnableIntc(int intc)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _EnableIntc(intc);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_DisableIntc
int DisableIntc(int intc)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _DisableIntc(intc);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_EnableDmac
int EnableDmac(int dmac)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _EnableDmac(dmac);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_DisableDmac
int DisableDmac(int dmac)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _DisableDmac(dmac);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_iEnableIntc
int iEnableIntc(int intc)
{
	int res = _iEnableIntc(intc);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_iDisableIntc
int iDisableIntc(int intc)
{
	int res = _iDisableIntc(intc);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_iEnableDmac
int iEnableDmac(int dmac)
{
	int res = _iEnableDmac(dmac);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_iDisableDmac
int iDisableDmac(int dmac)
{
	int res = _iDisableDmac(dmac);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_SyncDCache
void SyncDCache(void *start, void *end)
{
	int eie;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	_SyncDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));

	if (eie)
		EI();
}
#endif

#ifdef F_iSyncDCache
void iSyncDCache(void *start, void *end)
{
	_SyncDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));
}
#endif

#ifdef F_InvalidDCache
void InvalidDCache(void *start, void *end)
{
	int eie;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	_InvalidDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));

	if (eie)
		EI();
}
#endif

#ifdef F_iInvalidDCache
void iInvalidDCache(void *start, void *end)
{
	_InvalidDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));
}
#endif
