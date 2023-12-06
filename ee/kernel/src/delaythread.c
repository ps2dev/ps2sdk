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

#ifdef F_DelayThread
static u64 DelayThreadWakeup_callback(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
	(void)alarm_id;
	(void)scheduled_time;
	(void)actual_time;
	(void)pc_value;

	iSignalSema((s32)arg);
	ExitHandler();
	return 0;
}

s32 DelayThread(s32 microseconds)
{
	u32 eie;
	s32 sema_id;
	s32 timer_alarm_id;
	ee_sema_t sema;

	__asm__ __volatile__ ("mfc0\t%0, $12" : "=r" (eie));
	if ((eie & 0x10000) == 0)
	{
		return 0x80008008; // ECPUDI
	}
	sema.max_count = 1;
	sema.option = (u32)"DelayThread";
	sema.init_count = 0;
	sema_id = CreateSema(&sema);
	if (sema_id < 0)
	{
		return 0x80008003; // ESEMAPHORE
	}
	timer_alarm_id = SetTimerAlarm(TimerUSec2BusClock(0, microseconds), DelayThreadWakeup_callback, (void *)sema_id);
	if (timer_alarm_id < 0)
	{
		DeleteSema(sema_id);
		return timer_alarm_id;
	}
	WaitSema(sema_id);
	DeleteSema(sema_id);
	return 0;
}
#endif
