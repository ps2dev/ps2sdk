/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __TIMER_ALARM_H__
#define __TIMER_ALARM_H__

#include <tamtypes.h>
#include <timer.h>

#ifdef __cplusplus
extern "C" {
#endif

extern u64 AlarmHandler(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *last_pc);
extern s32 iSetTimerAlarm(u64 clock_cycles, timer_alarm_handler_t callback_handler, void *arg);
extern s32 SetTimerAlarm(u64 clock_cycles, timer_alarm_handler_t callback_handler, void *arg);
extern s32 iReleaseTimerAlarm(s32 id);
extern s32 ReleaseTimerAlarm(s32 id);

#ifdef __cplusplus
};
#endif

#endif

