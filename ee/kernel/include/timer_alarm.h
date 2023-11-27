/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __TIMER_ALARM_H__
#define __TIMER_ALARM_H__

#include <tamtypes.h>
#include <timer.h>

struct timer_alarm_t;
typedef void (*timer_alarm_callback_t)(struct timer_alarm_t *alarm, void *arg);

// Defines an alarm object
// Do not manipulate fields directly, but through functions.

struct timer_alarm_t {
    u64 scheduled_time;                  // Clock tick when the alarm should be triggered
    u64 timer_cycles;                    // Alarm period in clock ticks
    struct timer_alarm_t *prev, *next;   // Previous and next alarm in the sorted event list
    timer_alarm_callback_t callback;     // User callback to call during the IRQ
    void *usr_arg;
};

// Init/Deinit functions
void InitTimerAlarm();
void DeinitTimerAlarm();

// Allocate a timer-based alarm
void InitializeTimerAlarm(struct timer_alarm_t *alarm);
// Sets up an alarm (with a user callback function and argument) that triggers some cycles in the future.
void SetTimerAlarm(struct timer_alarm_t *alarm, u64 clock_cycles, timer_alarm_callback_t user_callback, void *user_arg);
// Starts/Enables the alarm.
void iStartTimerAlarm(struct timer_alarm_t *alarm);
void StartTimerAlarm(struct timer_alarm_t *alarm);
// Stops/Disabled the alarm.
void iStopTimerAlarm(struct timer_alarm_t *alarm);
void StopTimerAlarm(struct timer_alarm_t *alarm);
// Sleeping function
void ThreadWaitClock(u64 clock_cycles);

// Conversion functions, from time to bus cycles
static inline u64 NSec2TimerBusClock(u64 usec) {
    // Approximate, avoid 64 bit division
    return ((((kBUSCLK / 1000) * 65536ULL) / 1000000) * usec) >> 16;
}

static inline u64 USec2TimerBusClock(u64 usec) {
    // Approximate, avoid 64 bit division
    return ((((kBUSCLK / 1000) * 1024) / 1000) * usec) >> 10;
}

static inline u64 MSec2TimerBusClock(u64 msec) {
    return (kBUSCLK / 1000) * msec;
}

static inline u64 Sec2TimerBusClock(u64 sec) {
    return kBUSCLK * sec;
}

static inline void TimerBusClock2USec (u64 ulClock, u32 *pSec, u32 *pUsec) {
    u64 sec = ulClock/kBUSCLK;
    *pSec = (u32)sec;
    *pUsec = (1000000 * (ulClock - (sec * kBUSCLK))) / kBUSCLK;
}

static inline u64 TimerUSec2BusClock (u32 sec, u32 usec) {
    return Sec2TimerBusClock(sec) + USec2TimerBusClock(usec);
}

#endif

