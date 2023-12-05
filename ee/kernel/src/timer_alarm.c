/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <tamtypes.h>
#include <kernel.h>
#include <timer.h>
#include <timer_alarm.h>

#ifdef TIME_USE_T0
#define INTC_TIM       INTC_TIM0
#define T_COUNT        T0_COUNT
#define T_MODE         T0_MODE
#define T_COMP         T0_COMP
#else
#define INTC_TIM       INTC_TIM1
#define T_COUNT        T1_COUNT
#define T_MODE         T1_MODE
#define T_COMP         T1_COMP
#endif


#ifdef F_TimerAlarmInternals
// Event queue sorted from next to last.
struct timer_alarm_t *__event_queue = NULL;
int __alarm_timer_intc_id = -1;
#else
extern struct timer_alarm_t *__event_queue;
extern int __alarm_timer_intc_id;
#endif

#ifdef F___Timer2Resched
void __Timer2Resched(struct timer_alarm_t *alarm) {
    if (!alarm) {
        // Disable the counter, no alarms are queued
        *T_MODE = 0x0000;
    } else {
        // Setup timer for the next event.
        s64 wait_time = (s64)alarm->scheduled_time - (s64)(iGetTimerSystemTime());
        u64 alarm_clocks = wait_time <= 0 ? 1 : wait_time;

        if (alarm_clocks < 65536) {   // [0..2^16)
            // Using /1 clock mode for better precision
            *T_COUNT = 0;
            *T_COMP = (u16)alarm_clocks;
            *T_MODE = Tn_MODE(0, 0, 0, 0, 0, 1, 1, 0, 1, 1);
        }
        else if (alarm_clocks < 1048576) {  // [2^16..2^20)
            // Using /16 mode
            *T_COUNT = 0;
            *T_COMP = (u16)(alarm_clocks >> 4);
            *T_MODE = Tn_MODE(1, 0, 0, 0, 0, 1, 1, 0, 1, 1);
        }
        else if (alarm_clocks < 16777216) {  // [2^20..2^24)
            // Using /256 mode
            *T_COUNT = 0;
            *T_COMP = (u16)(alarm_clocks >> 8);
            *T_MODE = Tn_MODE(2, 0, 0, 0, 0, 1, 1, 0, 1, 1);
        }
        else {
            // Requires several interrups, there's no slower clock mode
            // Using counter overflow mode
            *T_COUNT = 0;
            *T_MODE = Tn_MODE(2, 0, 0, 0, 0, 1, 0, 1, 1, 1);
        }
    }
}
#else
extern void __Timer2Resched(struct timer_alarm_t *alarm);
#endif

#ifdef F_InitTimerAlarm
static int timOverflow(int ca) {
    (void)ca;

    // Check if we actually triggered an alarm
    if (__event_queue) {
        if (GetTimerSystemTime() > (__event_queue->scheduled_time)) {
            // Remove the top of the queue
            struct timer_alarm_t *gone = __event_queue;
            __event_queue = __event_queue->next;
            if (__event_queue)
                __event_queue->prev = NULL;
            gone->prev = NULL;
            gone->next = NULL;
            gone->scheduled_time = 0;
            __Timer2Resched(__event_queue);

            if (gone->callback)
                gone->callback(gone, gone->usr_arg);
        }
        else  // Timer might require several IRQs (for timers over ~100ms)
            __Timer2Resched(__event_queue);
    }

    // Clear both compare and overflow flags (since we can use either)
    *T_MODE |= (3 << 10);

    ExitHandler();
    return -1;
}

__attribute__((weak))
void InitTimerAlarm() {
    if (__alarm_timer_intc_id < 0)
        __alarm_timer_intc_id = AddIntcHandler(INTC_TIM, timOverflow, 0);

    *T_MODE = 0x0000;
    EnableIntc(INTC_TIM);
}
#endif

#ifdef F_DeinitTimerAlarm
__attribute__((weak))
void DeinitTimerAlarm() {
    DisableIntc(INTC_TIM);
    if (__alarm_timer_intc_id >= 0)
        RemoveIntcHandler(INTC_TIM, __alarm_timer_intc_id);
    __alarm_timer_intc_id = -1;
}
#endif

#ifdef F_InitializeTimerAlarm
/** Initializes a timer alarm struct so that it can be used.
 *
 * @param alarm Alarm object to initialize.
 *
 * Initializes the alarm object to be used. The object must be kept
 * around as long as it is being used.
 */
void InitializeTimerAlarm(struct timer_alarm_t *alarm) {
    // Just return an empty struct, that's not in the event queue
    alarm->prev = NULL;
    alarm->next = NULL;
    alarm->scheduled_time = 0;
    alarm->timer_cycles = 0;
    alarm->callback = NULL;
    alarm->usr_arg = NULL;
}
#endif

#ifdef F_iStopTimerAlarm
/** Same as StopTimerAlarm but for IRQ handlers
 *
 */
void iStopTimerAlarm(struct timer_alarm_t *alarm) {
    // Bail if the alarm is not set.
    if (!alarm->scheduled_time)
        return;

    // Check if the event is the next event in the queue.
    if (__event_queue == alarm) {
        // Remove it from the front of the queue
        if (alarm->next)
            alarm->next->prev = NULL;
        __event_queue = alarm->next;

        // Reschedule next event
        __Timer2Resched(__event_queue);
    }
    else {
        // Simply remove it, since it is not yet scheduled.
        alarm->prev->next = alarm->next;
        if (alarm->next)
            alarm->next->prev = alarm->prev;
    }

    // Cleanup the structure
    alarm->next = NULL;
    alarm->prev = NULL;
    alarm->scheduled_time = 0;
}
#endif

#ifdef F_StopTimerAlarm
/** Stop an alarm
 *
 * @param alarm Alarm object to be stopped.
 *
 * Stops the alarm. If the alarm was not armed it does nothing.
 * When an alarm triggers it is automatically disabled, so user must
 * call iStartTimerAlarm to re-enable it.
 */
void StopTimerAlarm(struct timer_alarm_t *alarm) {
    u32 oldintr = DIntr();
    iStopTimerAlarm(alarm);
    if (oldintr)
        EIntr();
}
#endif

#ifdef F_iStartTimerAlarm
/** Same as StartTimerAlarm but for IRQ handlers.
 *
 */
void iStartTimerAlarm(struct timer_alarm_t *alarm) {
    // Bail if the alarm is already set!
    if (alarm->scheduled_time)
        return;

    // Calculate clock scheduling time
    u64 sched_time = iGetTimerSystemTime() + alarm->timer_cycles;
    alarm->scheduled_time = sched_time;

    if (!__event_queue) {
        // No alarm was set
        __event_queue = alarm;
        alarm->next = alarm->prev = NULL;
        __Timer2Resched(alarm);
    }
    else if (__event_queue->scheduled_time > alarm->scheduled_time) {
        // This alarm is the next alarm to trigger
        alarm->next = __event_queue;
        alarm->prev = NULL;
        __event_queue->prev = alarm;
        __event_queue = alarm;
        __Timer2Resched(alarm);
    }
    else {
        // Just queue it at the right place (keep the list sorted)
        struct timer_alarm_t *it = __event_queue;
        for (; it != NULL; it = it->next) {
            if (!it->next || it->next->scheduled_time > alarm->scheduled_time) {
                alarm->next = it->next;
                alarm->prev = it;
                if (it->next)
                    it->next->prev = alarm;
                it->next = alarm;
                break;
            }
        }
        // No need to update the alarm at all
    }
}
#endif

#ifdef F_StartTimerAlarm
/** Starts (arms) an alarm.
 *
 * @param alarm Alarm object to be started.
 *
 * Starts the alarm. If the alarm was already started, it does nothing.
 */
void StartTimerAlarm(struct timer_alarm_t *alarm) {
    u32 oldintr = DIntr();
    iStartTimerAlarm(alarm);
    if (oldintr)
        EIntr();
}
#endif

#ifdef F_SetTimerAlarm
/** Sets up alarm paramenters.
 *
 * @param alarm Alarm object to be updated
 * @param clock_cycles Number of bus cycles when the alarm will trigger
 * @param user_callback Function pointer to call when the alarm triggers (in IRQ)
 * @param user_arg Argument to pass to user_callback
 *
 * Starts the alarm. If the alarm was already started, it does nothing.
 */
void SetTimerAlarm(struct timer_alarm_t *alarm, u64 clock_cycles, timer_alarm_callback_t user_callback, void *user_arg) {
    // Do not set alarms that are armed.
    if (alarm->scheduled_time)
        return;

    alarm->timer_cycles = clock_cycles;
    alarm->callback = user_callback;
    alarm->usr_arg = user_arg;
}
#endif

#ifdef F_ThreadWaitClock
static void _wake_sema_cb(struct timer_alarm_t *alarm, void *userptr) {
    (void)alarm;

    s32 sema_id = *(s32*)userptr;
    iSignalSema(sema_id);
}

void ThreadWaitClock(u64 clock_cycles) {
    // Setup a semaphore and alarm. The alarm will unblock the semaphore.
    ee_sema_t sema = {.max_count = 1, .init_count = 0, .option = 0};
    s32 sema_id = CreateSema(&sema);
    struct timer_alarm_t alarm;
    InitializeTimerAlarm(&alarm);
    SetTimerAlarm(&alarm, clock_cycles, _wake_sema_cb, &sema_id);
    StartTimerAlarm(&alarm);

    WaitSema(sema_id);

    DeleteSema(sema_id);
}
#endif

