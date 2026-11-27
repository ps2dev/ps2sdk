/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Some routines to do some thread delay work
 */

#include <kernel.h>
#include <timer.h>
#include <timer_alarm.h>
#include <delaythread.h>
#include <mipscopaccess.h>

#ifdef F_DelayThread
static u64 DelayThreadWakeup_callback(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
	(void)alarm_id;
	(void)scheduled_time;
	(void)actual_time;
	(void)pc_value;

	iWakeupThread((s32)arg);
	ExitHandler();
	return 0;
}

s32 DelayThread(s32 microseconds)
{
	u32 eie;
	volatile s32 thread_id;
	s32 timer_alarm_id;

	eie = get_mips_cop_reg(0, COP0_REG_Status);
	if ((eie & 0x10000) == 0)
	{
		return 0x80008008; // ECPUDI
	}
	thread_id = GetThreadId();
	timer_alarm_id = SetTimerAlarm(TimerUSec2BusClock(0, microseconds), DelayThreadWakeup_callback, (void *)thread_id);
	if (timer_alarm_id < 0)
	{
		thread_id = -1;
		return timer_alarm_id;
	}
	SleepThread();
	thread_id = -1;
	return 0;
}
#endif
