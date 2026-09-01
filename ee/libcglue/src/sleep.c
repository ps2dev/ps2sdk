/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * sleep implementation
 */

#include <kernel.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <timer_alarm.h>

#ifdef F_nanosleep
static u64 nanosleep_wakeup_callback(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
	(void)alarm_id;
	(void)scheduled_time;
	(void)actual_time;
	(void)pc_value;

	iWakeupThread((s32)arg);
	ExitHandler();
	return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
	u32 eie;
	volatile s32 thread_id;
	s32 timer_alarm_id;

	eie = get_mips_cop_reg(0, COP0_REG_Status);
	if ((eie & 0x10000) == 0)
	{
		errno = ENOSYS;  // Functionality not available
        return -1;
	}
	thread_id = GetThreadId();
	timer_alarm_id = SetTimerAlarm(Sec2TimerBusClock(req->tv_sec) + NSec2TimerBusClock(req->tv_nsec), nanosleep_wakeup_callback, (void *)thread_id);
	if (timer_alarm_id < 0)
	{
		thread_id = -1;
		errno = EAGAIN;  // Resource temporarily unavailable
        return -1;
	}
	SleepThread();
	thread_id = -1;
    if (rem != NULL)
    {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

	return 0;
}
#endif
