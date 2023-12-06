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
static u64 WaitSemaEx_callback(s32 id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
    (void)id;
    (void)scheduled_time;
    (void)actual_time;
    (void)pc_value;
    iReleaseWaitThread((s32)arg);
    return 0;
}

/** Semaphore wait function similar to newer platforms */
s32 WaitSemaEx(s32 semaid, int signal, u64 *timeout)
{
    int ret;
    int timerid;

    timerid = -1;

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
        timerid = SetTimerAlarm(USec2TimerBusClock(*timeout), &WaitSemaEx_callback, (void *)GetThreadId());
    }

    ret = WaitSema(semaid);

    if (ret < 0)
    {
        return ret;
    }

    if (timerid >= 0)
    {
        ReleaseTimerAlarm(timerid);
    }

    return semaid;
}
#endif
