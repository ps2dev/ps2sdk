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

struct timer_alarm_t alarm, alarm2;
int flag = 0, flag2 = 0;

void usercb1(struct timer_alarm_t *trigalarm, void *arg) {
    *(int*)arg = 1;
    iStartTimerAlarm(trigalarm);
}

void usercb2(struct timer_alarm_t *trigalarm, void *arg) {
    *(int*)arg = 1;
    iStartTimerAlarm(trigalarm);
    // Disable the 2s alarm
    iStopTimerAlarm(&alarm);
}

int main(int argc, char *argv[])
{
    InitializeTimerAlarm(&alarm);
    InitializeTimerAlarm(&alarm2);
    SetTimerAlarm(&alarm, Sec2TimerBusClock(2), &usercb1, &flag);
    SetTimerAlarm(&alarm2, Sec2TimerBusClock(5), &usercb2, &flag2);
    StartTimerAlarm(&alarm);
    StartTimerAlarm(&alarm2);

    while (1) {
        if (flag) {
            printf("2s alarm triggered!\n");
            flag = 0;
        }
        if (flag2) {
            printf("5s alarm triggered!\n");
            flag2 = 0;
        }
    }

    return 0;
}
