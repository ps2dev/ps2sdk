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
 * Some routines to do some timer alarm work
 */

#include <kernel.h>
#include <timer.h>
#include <timer_alarm.h>

typedef struct alarm_struct_
{
    struct alarm_struct_ *alarm_next;
    vs32 timer_counter_id;
    timer_alarm_handler_t callback_handler;
    void *callback_handler_arg;
} alarm_struct_t __attribute__((aligned(16)));

#define PTR_TO_ALARM_ID(ptr_, cid_) ((s32)((((uiptr)(ptr_)) << 4) | ((cid_) & 0xFE) | 1))
#define ALARM_ID_TO_PTR(id_) ((alarm_struct_t *)((((uiptr)(id_)) >> 8) << 4))
#define ALARM_ID_IS_VALID(id_) ((ALARM_ID_TO_PTR(id_) != NULL) && ((siptr)(id_) >= 0) && ((((((uiptr)id_) & 0xFF) == (((ALARM_ID_TO_PTR(id_))->timer_counter_id) & 0xFF)))))

#define ALARM_COUNT 64

#ifdef F_alarm_data
alarm_struct_t g_AlarmBuf[ALARM_COUNT] __attribute__((aligned(16)));
alarm_struct_t *g_pFreeAlarm = NULL;
#else
extern alarm_struct_t g_AlarmBuf[ALARM_COUNT] __attribute__((aligned(16)));
extern alarm_struct_t *g_pFreeAlarm;
#endif

#ifdef F_ForTimer_InitAlarm
__attribute__((weak)) void ForTimer_InitAlarm(void)
{
    g_pFreeAlarm = &g_AlarmBuf[0];
    for (u32 i = 0; i < (ALARM_COUNT - 1); i += 1)
    {
        g_AlarmBuf[i].alarm_next = &g_AlarmBuf[i + 1];
    }
    g_AlarmBuf[ALARM_COUNT - 1].alarm_next = NULL;
}
#endif

static inline alarm_struct_t *ForTimer_AllocAlarm(void)
{
    alarm_struct_t *alarm_current;
    alarm_current = g_pFreeAlarm;
    if (alarm_current != NULL)
    {
        g_pFreeAlarm = alarm_current->alarm_next;
    }
    return alarm_current;
}

static inline void ForTimer_FreeAlarm(alarm_struct_t *alarm_current)
{
    alarm_current->alarm_next = g_pFreeAlarm;
    alarm_current->timer_counter_id = 0;
    g_pFreeAlarm = alarm_current;
}

#ifdef F_AlarmHandler
u64 AlarmHandler(s32 alarm_id, u64 scheduled_time, u64 actual_time, void *arg, void *last_pc)
{
    u64 result;
    alarm_struct_t *alarm_current;

    alarm_current = (alarm_struct_t *)arg;
    result = alarm_current->callback_handler(
        PTR_TO_ALARM_ID(alarm_current, alarm_id),
        scheduled_time,
        actual_time,
        alarm_current->callback_handler_arg, last_pc);
    if (result == 0)
    {
        ForTimer_FreeAlarm(alarm_current);
        return -1;
    }
    return result;
}
#endif

#ifdef F_iSetTimerAlarm
s32 iSetTimerAlarm(u64 clock_cycles, timer_alarm_handler_t callback_handler, void *arg)
{
    s32 timer_counter_id;
    alarm_struct_t *alarm_current;

    if (callback_handler == NULL)
    {
        return 0x80000016; // EINVAL
    }
    alarm_current = ForTimer_AllocAlarm();
    if (alarm_current == NULL)
    {
        return 0x80008005; // ETIMER
    }
    timer_counter_id = iAllocTimerCounter();
    if (timer_counter_id < 0)
    {
        ForTimer_FreeAlarm(alarm_current);
        return timer_counter_id;
    }
    alarm_current->timer_counter_id = timer_counter_id;
    alarm_current->callback_handler = callback_handler;
    alarm_current->callback_handler_arg = arg;
    iSetTimerHandler(timer_counter_id, clock_cycles, AlarmHandler, alarm_current);
    iStartTimerCounter(timer_counter_id);
    return PTR_TO_ALARM_ID(alarm_current, timer_counter_id);
}
#endif

#ifdef F_SetTimerAlarm
/** Sets up alarm paramenters.
 *
 * @param clock_cycles Number of bus cycles when the alarm will trigger
 * @param callback_handler Function pointer to call when the alarm triggers (in IRQ)
 * @param arg Argument to pass to callback_handler
 *
 * Starts the alarm. If the alarm was already started, it does nothing.
 */
s32 SetTimerAlarm(u64 clock_cycles, timer_alarm_handler_t callback_handler, void *arg)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iSetTimerAlarm(clock_cycles, callback_handler, arg);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iReleaseTimerAlarm
s32 iReleaseTimerAlarm(s32 id)
{
    alarm_struct_t *alarm_current;
    s32 ret;

    alarm_current = ALARM_ID_TO_PTR(id);
    if (!ALARM_ID_IS_VALID(id))
    {
        return 0x80008002; // EID
    }
    ret = iFreeTimerCounter(alarm_current->timer_counter_id);
    if (ret == 0)
    {
        ForTimer_FreeAlarm(alarm_current);
    }
    return ret;
}
#endif

#ifdef F_ReleaseTimerAlarm
s32 ReleaseTimerAlarm(s32 id)
{
    alarm_struct_t *alarm_current;
    u32 oldintr;

    alarm_current = ALARM_ID_TO_PTR(id);
    oldintr = DIntr();
    if (!ALARM_ID_IS_VALID(id))
    {
        if (oldintr != 0)
        {
            EIntr();
        }
        return 0x80008002; // EID
    }
    FreeTimerCounter(alarm_current->timer_counter_id);
    // Note: the result of FreeTimerCounter is not checked, so can be called within own timer handler
    ForTimer_FreeAlarm(alarm_current);
    if (oldintr != 0)
    {
        EIntr();
    }
    return 0;
}
#endif
