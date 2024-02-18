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
 * Some routines to do some timer work
 */

#include <kernel.h>
#include <timer.h>
#include <string.h>

#define TIMER_MODE_START 0x00000001
#define TIMER_MODE_HANDLER 0x00000002

typedef struct counter_struct_
{
    struct counter_struct_ *timer_next;
    struct counter_struct_ *timer_previous;
    vu32 timer_key;
    u32 timer_mode;
    u64 timer_base_time;
    u64 timer_base_count;
    u64 timer_schedule;
    timer_alarm_handler_t callback_handler;
    void *gp_value;
    void *callback_handler_arg;
    u32 padding[3];
} counter_struct_t __attribute__((aligned(64)));

typedef struct timer_ee_global_struct_
{
    vu64 timer_handled_count;
    s32 intc_handler;
    vu32 timer_counter_total;
    vu32 timer_counter_used;
    counter_struct_t *timer_counter_buf_free;
    counter_struct_t *timer_counter_buf_alarm;
    vs32 current_handling_timer_id;
} timer_ee_global_struct;

#define PTR_TO_TIMER_ID(ptr_) ((s32)((((uiptr)(ptr_)) << 4) | ((ptr_)->timer_key)))
#define TIMER_ID_TO_PTR(id_) ((counter_struct_t *)((((uiptr)(id_)) >> 10) << 6))
#define TIMER_ID_IS_VALID(id_) ((TIMER_ID_TO_PTR(id_) != NULL) && ((siptr)(id_) >= 0) && (((uiptr)(id_) & 0x3FF) == ((TIMER_ID_TO_PTR(id_))->timer_key)))

// The following is defined in timer_alarm.h
extern void ForTimer_InitAlarm(void);

extern void SetT2(volatile void *ptr, u32 val);
extern void SetT2_COUNT(u32 val);
extern void SetT2_MODE(u32 val);
extern void SetT2_COMP(u32 val);
extern void InsertAlarm_ForTimer(counter_struct_t *timer_link);
extern counter_struct_t *UnlinkAlarm_ForTimer(counter_struct_t *timer_unlink);
extern s32 TimerHandler_callback(s32 cause, void *arg, void *addr);

#define COUNTER_COUNT 128

#ifdef F_timer_data
timer_ee_global_struct g_Timer = {
    .timer_handled_count = 0,
    .intc_handler = -1,
    .timer_counter_total = 1,
    .timer_counter_used = 0,
    .timer_counter_buf_free = NULL,
    .timer_counter_buf_alarm = NULL,
    .current_handling_timer_id = -1,
};
counter_struct_t g_CounterBuf[COUNTER_COUNT] __attribute__((aligned(64)));
#else
extern timer_ee_global_struct g_Timer;
extern counter_struct_t g_CounterBuf[COUNTER_COUNT] __attribute__((aligned(64)));
#endif

#ifdef F_SetT2
void SetT2(volatile void *ptr, u32 val)
{
    ee_kmode_enter();
    *((volatile u32 *)ptr) = val;
    ee_kmode_exit();
}
#endif

#ifdef F_SetT2_COUNT
void SetT2_COUNT(u32 val)
{
    SetT2((volatile void *)K_T2_COUNT, val);
}
#endif

#ifdef F_SetT2_MODE
void SetT2_MODE(u32 val)
{
    SetT2((volatile void *)K_T2_MODE, val);
}
#endif

#ifdef F_SetT2_COMP
void SetT2_COMP(u32 val)
{
    SetT2((volatile void *)K_T2_COMP, val);
}
#endif

#ifdef F_InitTimer
__attribute__((weak)) s32 InitTimer(s32 in_mode)
{
    s32 handler;
    u32 oldintr;
    u32 mode;

    if (g_Timer.intc_handler >= 0)
    {
        return 0x80008001; // EINIT
    }
    g_Timer.timer_handled_count = 0;
    g_Timer.timer_counter_used = 0;
    memset(g_CounterBuf, 0, sizeof(g_CounterBuf));
    g_Timer.timer_counter_buf_free = &g_CounterBuf[0];
    for (u32 i = 0; i < ((sizeof(g_CounterBuf) / sizeof(g_CounterBuf[0])) - 1); i += 1)
    {
        g_CounterBuf[i].timer_next = &g_CounterBuf[i + 1];
    }
    g_CounterBuf[(sizeof(g_CounterBuf) / sizeof(g_CounterBuf[0])) - 1].timer_next = NULL;
    ForTimer_InitAlarm();
    handler = AddIntcHandler2(INTC_TIM2, TimerHandler_callback, 0, NULL);
    if (handler < 0)
    {
        return 0x80009021; // EINT_HANDLER
    }
    g_Timer.intc_handler = handler;
    oldintr = DIntr();
    mode = ((*T2_MODE) & (~0x3)) | in_mode;
    mode |= (1 << 9) | (1 << 8);
    if ((mode & (1 << 7)) == 0)
    {
        mode |= (1 << 7);
        mode |= (1 << 11) | (1 << 10);
        SetT2_COUNT(0);
        SetT2_COMP(0xFFFF);
    }
    SetT2_MODE(mode);
    EnableIntc(INTC_TIM2);
    if (oldintr != 0)
    {
        EIntr();
    }
    return 0;
}
#endif

#ifdef F_EndTimer
__attribute__((weak)) s32 EndTimer(void)
{
    u32 oldintr;

    if (g_Timer.intc_handler < 0)
    {
        return 0x80008001; // EINIT
    }
    if (g_Timer.timer_counter_used != 0)
    {
        return 0x80000010; // EBUSY
    }
    oldintr = DIntr();
    if (RemoveIntcHandler(INTC_TIM2, g_Timer.intc_handler) == 0)
    {
        DisableIntc(INTC_TIM2);
        SetT2_MODE((1 << 11) | (1 << 10));
        SetT2_COUNT(0);
    }
    g_Timer.timer_handled_count = 0;
    g_Timer.intc_handler = -1;
    if (oldintr != 0)
    {
        EIntr();
    }
    return 0;
}
#endif

#ifdef F_GetTimerPreScaleFactor
s32 GetTimerPreScaleFactor(void)
{
    if (g_Timer.intc_handler < 0)
    {
        return 0x80008001; // EINIT
    }
    return (*T2_MODE) & 3;
}
#endif

#ifdef F_StartTimerSystemTime
/** Init the timer
 *
 *
 * Start the interruption handler for getting the system time
 * This function should be called at beginning of your program
 */
__attribute__((weak)) s32 StartTimerSystemTime(void)
{
    u32 oldintr;

    oldintr = DIntr();
    if (((*T2_MODE) & (1 << 7)) != 0)
    {
        if (oldintr != 0)
        {
            EIntr();
        }
        return 1;
    }
    else
    {
        SetT2_MODE(((*T2_MODE) & (~((1 << 11) | (1 << 10)))) | (1 << 7));
        SetNextComp(iGetTimerSystemTime());
        if (oldintr != 0)
        {
            EIntr();
        }
        return 0;
    }
}
#endif

#ifdef F_StopTimerSystemTime
/** Finish the timer
 *
 *
 * Stops the interruption handler for getting the  system time
 * This function should be called when ending your program
 */
__attribute__((weak)) s32 StopTimerSystemTime(void)
{
    u32 oldintr;

    oldintr = DIntr();
    if (((*T2_MODE) & (1 << 7)) != 0)
    {
        SetT2_MODE((*T2_MODE) & (~((1 << 11) | (1 << 10))));
        if (oldintr != 0)
        {
            EIntr();
        }
        return 1;
    }
    else
    {
        if (oldintr != 0)
        {
            EIntr();
        }
        return 0;
    }
}
#endif

#ifdef F_SetNextComp
void SetNextComp(u64 time_now)
{
    u64 a0, a1;
    counter_struct_t *timer_current;

    if (g_Timer.current_handling_timer_id >= 0)
    {
        return;
    }
    timer_current = g_Timer.timer_counter_buf_alarm;
    if (timer_current == NULL)
    {
        SetT2_COMP(0);
        SetT2_MODE((*T2_MODE) & (~(1 << 11)));
        return;
    }
    a0 = timer_current->timer_schedule + timer_current->timer_base_time - timer_current->timer_base_count;
    timer_current = timer_current->timer_next;
    while (timer_current != NULL)
    {
        a1 = timer_current->timer_schedule + timer_current->timer_base_time - timer_current->timer_base_count;
        if (a1 < (a0 + 0x733))
        {
            a0 = a1;
        }
        else
        {
            break;
        }
        timer_current = timer_current->timer_next;
    }
    if (a0 < (time_now + 0x733))
    {
        SetT2_COMP((*T2_COUNT) + (0x733 >> (((*T2_MODE) & 3) << 2)));
        SetT2_MODE((*T2_MODE) & (~(1 << 11)));
    }
    else
    {
        SetT2_MODE((*T2_MODE) & (~(1 << 11)));
        SetT2_COMP(a0 >> (((*T2_MODE) & 3) << 2));
    }
}
#endif

#ifdef F_InsertAlarm_ForTimer
void InsertAlarm_ForTimer(counter_struct_t *timer_link)
{
    counter_struct_t *timer_current;
    counter_struct_t *timer_next;
    u64 abs_cmp;

    abs_cmp = timer_link->timer_schedule + timer_link->timer_base_time - timer_link->timer_base_count;
    timer_current = NULL;
    timer_next = g_Timer.timer_counter_buf_alarm;
    while (timer_next != NULL)
    {
        u64 target_cmp = timer_next->timer_schedule + timer_next->timer_base_time - timer_next->timer_base_count;
        if (abs_cmp < target_cmp)
        {
            break;
        }
        timer_current = timer_next;
        timer_next = timer_next->timer_next;
    }
    timer_link->timer_previous = timer_current;
    timer_link->timer_next = timer_next;
    if (timer_next != NULL)
    {
        timer_next->timer_previous = timer_link;
    }
    if (timer_current != NULL)
    {
        timer_current->timer_next = timer_link;
    }
    else
    {
        g_Timer.timer_counter_buf_alarm = timer_link;
    }
}
#endif

#ifdef F_UnlinkAlarm_ForTimer
counter_struct_t *UnlinkAlarm_ForTimer(counter_struct_t *timer_unlink)
{
    counter_struct_t *timer_next;

    timer_next = timer_unlink->timer_next;
    if (timer_unlink->timer_previous)
    {
        timer_unlink->timer_previous->timer_next = timer_next;
    }
    else
    {
        g_Timer.timer_counter_buf_alarm = timer_unlink->timer_next;
    }
    if (timer_next != NULL)
    {
        timer_next->timer_previous = timer_unlink->timer_previous;
    }
    timer_unlink->timer_previous = NULL;
    return timer_next;
}
#endif

#ifdef F_TimerHandler_callback
static inline u64 ApplyOverflow(void)
{
    u64 timer_system_time_now;
    u32 mode;
    u32 low;

    low = *T2_COUNT;
    mode = *T2_MODE;
    if ((mode & (1 << 11)) != 0)
    {
        g_Timer.timer_handled_count += 1;
        SetT2_MODE(mode & (~(1 << 10)));
        low = *T2_COUNT;
    }
    timer_system_time_now = (g_Timer.timer_handled_count << 16) | low;
    timer_system_time_now = timer_system_time_now << ((mode & 3) << 2);
    return timer_system_time_now;
}

s32 TimerHandler_callback(s32 cause, void *arg, void *addr)
{
    counter_struct_t *timer_current;
    counter_struct_t *timer_next;
    u64 target_cmp;
    u64 abs_cmp;
    u32 timer_id;
    u64 timer_schedule_next;

    if (((*T2_MODE) & (1 << 10)) != 0)
    {
        timer_current = g_Timer.timer_counter_buf_alarm;
        while (timer_current != NULL)
        {
            target_cmp = timer_current->timer_schedule + timer_current->timer_base_time - timer_current->timer_base_count;
            abs_cmp = ApplyOverflow();
            if (abs_cmp < target_cmp)
            {
                break;
            }
            timer_next = UnlinkAlarm_ForTimer(timer_current);
            timer_id = PTR_TO_TIMER_ID(timer_current);
            g_Timer.current_handling_timer_id = timer_id;
            SetGP(timer_current->gp_value);
            timer_schedule_next = timer_current->callback_handler(
                timer_id,
                timer_current->timer_schedule,
                (abs_cmp + timer_current->timer_base_count) - timer_current->timer_base_time,
                timer_current->callback_handler_arg,
                addr);
            if (timer_schedule_next == 0)
            {
                timer_current->timer_mode &= ~TIMER_MODE_HANDLER;
            }
            else if (timer_schedule_next == (u64)-1)
            {
                timer_current->timer_next = g_Timer.timer_counter_buf_free;
                g_Timer.timer_counter_buf_free = timer_current;
                timer_current->timer_key = 0;
                timer_current->timer_mode = 0;
                g_Timer.timer_counter_used -= 1;
            }
            else
            {
                if (timer_schedule_next < 0x3999)
                {
                    timer_schedule_next = 0x3999;
                }
                timer_current->timer_schedule += timer_schedule_next;
                InsertAlarm_ForTimer(timer_current);
            }
            timer_current = timer_next;
        }
    }
    g_Timer.current_handling_timer_id = -1;
    SetNextComp(ApplyOverflow());
    ApplyOverflow();
    ExitHandler();
    return 0;
}
#endif

#ifdef F_iGetTimerSystemTime
/** Get current System Time
 *
 * This function gets the current system time
 * Can be called from an interrupt handler
 * Cannot be called from a thread
 *
 * @return current system time in BUSCLK units, 0 in case the InitTimer wasn't called at init.
 */
u64 iGetTimerSystemTime(void)
{
    u64 timer_handled_count, timer_system_time_now;
    u32 low, mode;

    low = *T2_COUNT;
    mode = *T2_MODE;
    timer_handled_count = g_Timer.timer_handled_count;
    if ((mode & (1 << 11)) != 0)
    {
        timer_handled_count += 1;
        low = *T2_COUNT;
    }
    timer_system_time_now = (timer_handled_count << 16) | low;
    timer_system_time_now = timer_system_time_now << ((mode & 3) << 2);
    return timer_system_time_now;
}
#endif

#ifdef F_GetTimerSystemTime
/** Get current System Time
 *
 * This function gets the current system time
 * Can be called from an interrupt handler
 * Can be called from a thread
 * Multithread safe
 *
 * @return current system time in BUSCLK units, 0 in case the InitTimer wasn't called at init.
 */
u64 GetTimerSystemTime(void)
{
    u32 oldintr;
    u64 ret;

    oldintr = DIntr();
    ret = iGetTimerSystemTime();
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iAllocTimerCounter
s32 iAllocTimerCounter(void)
{
    counter_struct_t *timer_current;

    timer_current = g_Timer.timer_counter_buf_free;
    if (timer_current == NULL)
    {
        return 0x80008005; // ETIMER
    }
    g_Timer.timer_counter_buf_free = timer_current->timer_next;
    g_Timer.timer_counter_used += 1;
    timer_current->timer_mode = 0;
    timer_current->callback_handler = NULL;
    timer_current->timer_base_count = 0;
    g_Timer.timer_counter_total += 1;
    timer_current->timer_key = ((g_Timer.timer_counter_total << 1) & 0x3FE) | 1;
    return PTR_TO_TIMER_ID(timer_current);
}
#endif

#ifdef F_AllocTimerCounter
s32 AllocTimerCounter(void)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iAllocTimerCounter();
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iFreeTimerCounter
s32 iFreeTimerCounter(s32 id)
{
    counter_struct_t *timer_current;

    timer_current = TIMER_ID_TO_PTR(id);
    if (!TIMER_ID_IS_VALID(id))
    {
        return 0x80008002; // EID
    }
    if (g_Timer.current_handling_timer_id == id)
    {
        return 0x80000010; // EBUSY
    }
    if ((timer_current->timer_mode & TIMER_MODE_HANDLER) != 0)
    {
        UnlinkAlarm_ForTimer(timer_current);
    }
    timer_current->timer_key = 0;
    timer_current->timer_mode = 0;
    timer_current->timer_next = g_Timer.timer_counter_buf_free;
    g_Timer.timer_counter_buf_free = timer_current;
    g_Timer.timer_counter_used -= 1;
    return 0;
}
#endif

#ifdef F_FreeTimerCounter
s32 FreeTimerCounter(s32 id)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iFreeTimerCounter(id);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iGetTimerUsedUnusedCounters
s32 iGetTimerUsedUnusedCounters(u32 *used_counters, u32 *unused_counters)
{
    if (g_Timer.intc_handler < 0)
    {
        return 0x80008001; // EINIT
    }
    if (used_counters != NULL)
    {
        *used_counters = g_Timer.timer_counter_used;
    }
    if (unused_counters != NULL)
    {
        *unused_counters = (sizeof(g_CounterBuf) / sizeof(g_CounterBuf[0])) - g_Timer.timer_counter_used;
    }
    return 0;
}
#endif

#ifdef F_GetTimerUsedUnusedCounters
s32 GetTimerUsedUnusedCounters(u32 *used_counters, u32 *unused_counters)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iGetTimerUsedUnusedCounters(used_counters, unused_counters);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iStartTimerCounter
s32 iStartTimerCounter(s32 id)
{
    counter_struct_t *timer_current;
    u64 timer_system_time_now;

    timer_current = TIMER_ID_TO_PTR(id);
    if (!TIMER_ID_IS_VALID(id))
    {
        return 0x80008002; // EID
    }
    if (g_Timer.current_handling_timer_id == id)
    {
        return 0x80000010; // EBUSY
    }
    if ((timer_current->timer_mode & TIMER_MODE_START) != 0)
    {
        return 1;
    }
    timer_system_time_now = iGetTimerSystemTime();
    timer_current->timer_base_time = timer_system_time_now;
    timer_current->timer_mode |= TIMER_MODE_START;
    if ((timer_current->timer_mode & TIMER_MODE_HANDLER) != 0)
    {
        InsertAlarm_ForTimer(timer_current);
        SetNextComp(timer_system_time_now);
    }
    return 0;
}
#endif

#ifdef F_StartTimerCounter
s32 StartTimerCounter(s32 id)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iStartTimerCounter(id);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iStopTimerCounter
s32 iStopTimerCounter(s32 id)
{
    counter_struct_t *timer_current;
    u64 timer_system_time_now;

    timer_current = TIMER_ID_TO_PTR(id);
    if (!TIMER_ID_IS_VALID(id))
    {
        return 0x80008002; // EID
    }
    if (g_Timer.current_handling_timer_id == id)
    {
        return 0x80000010; // EBUSY
    }
    if ((timer_current->timer_mode & TIMER_MODE_START) == 0)
    {
        return 0;
    }
    timer_system_time_now = iGetTimerSystemTime();
    timer_current->timer_base_count += timer_system_time_now - timer_current->timer_base_time;
    timer_current->timer_mode &= ~TIMER_MODE_START;
    if ((timer_current->timer_mode & TIMER_MODE_HANDLER) != 0)
    {
        UnlinkAlarm_ForTimer(timer_current);
        SetNextComp(timer_system_time_now);
    }
    return 1;
}
#endif

#ifdef F_StopTimerCounter
s32 StopTimerCounter(s32 id)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iStopTimerCounter(id);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_SetTimerCount
u64 SetTimerCount(s32 id, u64 timer_count)
{
    counter_struct_t *timer_current;
    u32 oldintr;
    u64 ret;
    u64 timer_system_time_now;

    timer_current = TIMER_ID_TO_PTR(id);
    oldintr = DIntr();
    if ((!TIMER_ID_IS_VALID(id)) || g_Timer.current_handling_timer_id == id)
    {
        if (oldintr != 0)
        {
            EIntr();
        }
        return -1;
    }
    ret = timer_current->timer_base_count;
    if ((timer_current->timer_mode & TIMER_MODE_START) != 0)
    {
        timer_system_time_now = iGetTimerSystemTime();
        ret += timer_system_time_now - timer_current->timer_base_time;
        timer_current->timer_base_count = timer_count;
        timer_current->timer_base_time = timer_system_time_now;
    }
    else
    {
        timer_current->timer_base_count = timer_count;
    }
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iGetTimerBaseTime
u64 iGetTimerBaseTime(s32 id)
{
    counter_struct_t *timer_current;

    timer_current = TIMER_ID_TO_PTR(id);
    if (!TIMER_ID_IS_VALID(id))
    {
        return -1;
    }
    if ((timer_current->timer_mode & TIMER_MODE_START) == 0)
    {
        return 0;
    }
    return timer_current->timer_base_time - timer_current->timer_base_count;
}
#endif

#ifdef F_GetTimerBaseTime
u64 GetTimerBaseTime(s32 id)
{
    u32 oldintr;
    u64 ret;

    oldintr = DIntr();
    ret = iGetTimerBaseTime(id);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iGetTimerCount
u64 iGetTimerCount(s32 id)
{
    u64 ret;
    counter_struct_t *timer_current;

    timer_current = TIMER_ID_TO_PTR(id);
    if (!TIMER_ID_IS_VALID(id))
    {
        return -1;
    }
    ret = timer_current->timer_base_count;
    if ((timer_current->timer_mode & TIMER_MODE_START) != 0)
    {
        ret += iGetTimerSystemTime() - timer_current->timer_base_time;
    }
    return ret;
}
#endif

#ifdef F_GetTimerCount
u64 GetTimerCount(s32 id)
{
    u32 oldintr;
    u64 ret;

    oldintr = DIntr();
    ret = iGetTimerCount(id);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_iSetTimerHandler
s32 iSetTimerHandler(s32 id, u64 scheduled_time, timer_alarm_handler_t callback_handler, void *arg)
{
    counter_struct_t *timer_current;

    timer_current = TIMER_ID_TO_PTR(id);
    if (!TIMER_ID_IS_VALID(id))
    {
        return 0x80008002; // EID
    }
    if (g_Timer.current_handling_timer_id == id)
    {
        return 0x80000010; // EBUSY
    }
    if ((timer_current->timer_mode & TIMER_MODE_HANDLER) != 0)
    {
        UnlinkAlarm_ForTimer(timer_current);
    }
    timer_current->callback_handler = callback_handler;
    if (callback_handler == NULL)
    {
        timer_current->timer_mode &= ~TIMER_MODE_HANDLER;
    }
    else
    {
        timer_current->timer_schedule = scheduled_time;
        timer_current->timer_mode |= TIMER_MODE_HANDLER;
        timer_current->gp_value = GetGP();
        timer_current->callback_handler_arg = arg;
        if ((timer_current->timer_mode & TIMER_MODE_START) != 0)
        {
            InsertAlarm_ForTimer(timer_current);
        }
    }
    SetNextComp(iGetTimerSystemTime());
    return 0;
}
#endif

#ifdef F_SetTimerHandler
s32 SetTimerHandler(s32 id, u64 scheduled_time, timer_alarm_handler_t callback_handler, void *arg)
{
    u32 oldintr;
    s32 ret;

    oldintr = DIntr();
    ret = iSetTimerHandler(id, scheduled_time, callback_handler, arg);
    if (oldintr != 0)
    {
        EIntr();
    }
    return ret;
}
#endif

#ifdef F_TimerBusClock2USec
void TimerBusClock2USec(u64 clocks, u32 *seconds_result, u32 *microseconds_result)
{
    u32 seconds;

    seconds = (u32)(clocks / kBUSCLK);
    if (seconds_result != NULL)
    {
        *seconds_result = seconds;
    }
    if (microseconds_result != NULL)
    {
        *microseconds_result = (u32)(1000 * 1000 * (clocks - (((s64)(s32)((kBUSCLK * (u64)seconds) >> 32) << 32) | (u32)(kBUSCLK * seconds))) / kBUSCLK);
    }
}
#endif

#ifdef F_TimerUSec2BusClock
u64 TimerUSec2BusClock(u32 seconds, u32 microseconds)
{
    return (seconds * kBUSCLK) + (microseconds * (u64)kBUSCLK / (1000 * 1000));
}
#endif

#ifdef F_TimerBusClock2Freq
float TimerBusClock2Freq(s64 clocks)
{
    return (float)kBUSCLK / (float)clocks;
}
#endif

#ifdef F_TimerFreq2BusClock
u64 TimerFreq2BusClock(float timer_frequency)
{
    return (u64)((float)kBUSCLK / timer_frequency);
}
#endif

#ifdef F_cpu_ticks
u32 cpu_ticks(void)
{
    u32 out;

    asm("mfc0\t%0, $9\n"
        : "=r"(out));
    return out;
}
#endif
