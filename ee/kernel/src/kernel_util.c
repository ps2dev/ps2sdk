/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <kernel.h>
#include <timer_alarm.h>
#include <kernel_util.h>

#ifdef F_WaitSemaEx
static void WaitSemaEx_callback(struct timer_alarm_t *alarm, void *arg)
{
	iReleaseWaitThread((s32)arg);
}

/** Semaphore wait function similar to newer platforms */
s32 WaitSemaEx(s32 semaid, int signal, u64 *timeout)
{
    int ret;
    struct timer_alarm_t alarm;

    // TODO: other values NYI
    if (signal != 1)
    {
    	return -1;
    }

    if (timeout != NULL && *timeout == 0)
    {
    	ret = PollSema(semaid);
        if (ret < 0)
        {
            return ret;
        }
        return semaid;
    }

    if (timeout != NULL)
    {
		InitializeTimerAlarm(&alarm);
        SetTimerAlarm(&alarm, USec2TimerBusClock(*timeout), &WaitSemaEx_callback, (void *)GetThreadId());
    }

    ret = WaitSema(semaid);

    if (timeout != NULL)
    {
    	StopTimerAlarm(&alarm);
    }
    
    if (ret < 0)
    {
        return ret;
    }
    return semaid;
}
#endif
