/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Timer prototypes
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <tamtypes.h>

// EE Timers
#define T0_COUNT ((volatile unsigned int *)0x10000000)
#define T0_MODE  ((volatile unsigned int *)0x10000010)
#define T0_COMP  ((volatile unsigned int *)0x10000020)
#define T0_HOLD  ((volatile unsigned int *)0x10000030)

#define T1_COUNT ((volatile unsigned int *)0x10000800)
#define T1_MODE  ((volatile unsigned int *)0x10000810)
#define T1_COMP  ((volatile unsigned int *)0x10000820)
#define T1_HOLD  ((volatile unsigned int *)0x10000830)

// Note! T2 and T3 don't have a Tn_HOLD register!
// ----------------------------------------------
#define T2_COUNT ((volatile unsigned int *)0x10001000)
#define T2_MODE  ((volatile unsigned int *)0x10001010)
#define T2_COMP  ((volatile unsigned int *)0x10001020)

#define T3_COUNT ((volatile unsigned int *)0x10001800)
#define T3_MODE  ((volatile unsigned int *)0x10001810)
#define T3_COMP  ((volatile unsigned int *)0x10001820)

// Pointers to the kernel segment (uncached)
#define K_T2_COUNT  ((volatile unsigned int *)0xB0001000)
#define K_T2_MODE   ((volatile unsigned int *)0xB0001010)
#define K_T2_COMP   ((volatile unsigned int *)0xB0001020)

#define Tn_MODE(CLKS, GATE, GATS, GATM, ZRET, CUE, CMPE, OVFE, EQUF, OVFF) \
    (u32)((u32)(CLKS) | ((u32)(GATE) << 2) |                               \
          ((u32)(GATS) << 3) | ((u32)(GATM) << 4) |                        \
          ((u32)(ZRET) << 6) | ((u32)(CUE) << 7) |                         \
          ((u32)(CMPE) << 8) | ((u32)(OVFE) << 9) |                        \
          ((u32)(EQUF) << 10) | ((u32)(OVFF) << 11))

// Available rates
#define kBUSCLK         (147456000)
#define kBUSCLKBY16     (kBUSCLK / 16)
#define kBUSCLKBY256    (kBUSCLK / 256)
#define kHBLNK_NTSC     (15734)
#define kHBLNK_PAL      (15625)
#define kHBLNK_DTV480p  (31469)
#define kHBLNK_DTV1080i (33750)

typedef u64 (*timer_alarm_handler_t)(s32 id, u64 scheduled_time, u64 actual_time, void *arg, void *pc_value);

#ifdef __cplusplus
extern "C" {
#endif

extern s32 InitTimer(s32 in_mode);
extern s32 EndTimer(void);
extern s32 GetTimerPreScaleFactor(void);
extern s32 StartTimerSystemTime(void);
extern s32 StopTimerSystemTime(void);
extern void SetNextComp(u64 time);
extern u64 iGetTimerSystemTime(void);
extern u64 GetTimerSystemTime(void);
extern s32 iAllocTimerCounter(void);
extern s32 AllocTimerCounter(void);
extern s32 iFreeTimerCounter(s32 id);
extern s32 FreeTimerCounter(s32 id);
extern s32 iGetTimerUsedUnusedCounters(u32 *used_counters, u32 *unused_counters);
extern s32 GetTimerUsedUnusedCounters(u32 *used_counters, u32 *unused_counters);
extern s32 iStartTimerCounter(s32 id);
extern s32 StartTimerCounter(s32 id);
extern s32 iStopTimerCounter(s32 id);
extern s32 StopTimerCounter(s32 id);
extern u64 SetTimerCount(s32 id, u64 timer_count);
extern u64 iGetTimerBaseTime(s32 id);
extern u64 GetTimerBaseTime(s32 id);
extern u64 iGetTimerCount(s32 id);
extern u64 GetTimerCount(s32 id);
extern s32 iSetTimerHandler(s32 id, u64 scheduled_time, timer_alarm_handler_t callback_handler, void *arg);
extern s32 SetTimerHandler(s32 id, u64 scheduled_time, timer_alarm_handler_t callback_handler, void *arg);
extern void TimerBusClock2USec(u64 clocks, u32 *seconds_result, u32 *microseconds_result);
extern u64 TimerUSec2BusClock(u32 seconds, u32 microseconds);
extern float TimerBusClock2Freq(s64 clocks);
extern u64 TimerFreq2BusClock(float timer_frequency);
extern u32 cpu_ticks(void);

// Additional conversion functions, from time to bus cycles
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

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H__ */
