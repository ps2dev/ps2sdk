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

int flag = 0, flag2 = 0;

void usercb(struct timer_alarm_t *alarm, void *arg) {
    *(int*)arg = 1;
    // The alarm does not trigger again unless we re-enable it manually
    iStartTimerAlarm(alarm);
}

int main(int argc, char *argv[])
{
    struct timer_alarm_t alarm, alarm2;
    InitializeTimerAlarm(&alarm);
    InitializeTimerAlarm(&alarm2);
    SetTimerAlarm(&alarm, Sec2TimerBusClock(2), &usercb, &flag);
    SetTimerAlarm(&alarm2, MSec2TimerBusClock(1414), &usercb, &flag2);
    StartTimerAlarm(&alarm);
    StartTimerAlarm(&alarm2);

    while (1) {
        if (flag) {
            printf("Hello there, 2 second tick!\n");
            flag = 0;
        }
        if (flag2) {
            printf("Hello there, sqrt(2) second tick!\n");
            flag2 = 0;
        }
    }

    return 0;
}
