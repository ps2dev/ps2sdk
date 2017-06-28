/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE kernel update for thread functions
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

#ifdef F__thread_internals

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
