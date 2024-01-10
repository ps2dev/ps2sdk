/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <kernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <timer.h>
#include <timer_alarm.h>

int flag = 2, flag2 = 2;

static u64 usercb(s32 id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value)
{
    *(int*)arg = 1;
    return 0;
}

int main(int argc, char *argv[])
{
    while (1) {
        if (flag) {
            if (flag != 2)
            {
                printf("Hello there, 2 second tick!\n");
            }
            flag = 0;
            SetTimerAlarm(Sec2TimerBusClock(2), &usercb, &flag);
        }
        if (flag2) {
            if (flag2 != 2)
            {
                printf("Hello there, sqrt(2) second tick!\n");
            }
            flag2 = 0;
            SetTimerAlarm(MSec2TimerBusClock(1414), &usercb, &flag2);
        }
    }

    return 0;
}
