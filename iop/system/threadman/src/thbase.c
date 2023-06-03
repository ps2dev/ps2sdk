#include "thbase.h"
#include "loadcore.h"
#include "sysclib.h"
#include "sysmem.h"
#include "kerr.h"
#include "intrman.h"
#include "thsemap.h"
#include "xthbase.h"
#include "xtimrman.h"

#include "thcommon.h"

static void clock_mul(iop_sys_clock_t *dst, iop_sys_clock_t *src, u32 mul);
static void clock_div(iop_sys_clock_t *dst, iop_sys_clock_t *src, u32 d, u32 *r);

static struct thread *refer_thread(int thid, int current);

static void thread_get_run_stats(struct thread *thread, iop_thread_run_status_t *stat);
static void thread_get_status(struct thread *thread, iop_thread_info_t *info);

struct thread_context *GetThreadCtx()
{
    return &thctx;
}

int CreateThread(iop_thread_t *thparam)
{
    struct thread *thread;
    void *stack;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thparam->attr & ~(TH_ASM | TH_C | TH_UMODE | TH_NO_FILLSTACK | TH_CLEAR_STACK)) {
        return KE_ILLEGAL_ATTR;
    }

    if (thparam->priority > 126) {
        return KE_ILLEGAL_PRIORITY;
    }

    if ((u32)thparam->thread & 3) {
        return KE_ILLEGAL_ENTRY;
    }

    if (thparam->stacksize < 0x130) {
        return KE_ILLEGAL_STACK_SIZE;
    }

    CpuSuspendIntr(&state);

    thread = heap_alloc(TAG_THREAD, sizeof(*thread));
    if (!thread) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    thparam->stacksize = ALIGN_256(thparam->stacksize);
    stack              = AllocSysMemory(1, thparam->stacksize, 0);
    if (!stack) {
        heap_free(&thread->tag);
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    thread->tag.id        = ++thctx.thread_id;
    thread->entry         = thparam->thread;
    thread->stack_size    = thparam->stacksize;
    thread->stack_top     = stack;
    thread->init_priority = thparam->priority;
    thread->attr          = thparam->attr;
    thread->option        = thparam->option;
    thread->status        = THS_DORMANT;

    asm __volatile__("sw $gp, 0(%0)\n"
                     : "=r"(thread->gp)::);

    list_insert(&thctx.thread_list, &thread->thread_list);

    if ((thread->attr & TH_NO_FILLSTACK) == 0) {
        // why -0x30?
        memset(thread->stack_top, 0xff, thread->stack_size - 0x30);
    }

    CpuResumeIntr(state);

    return MAKE_HANDLE(thread);
}

int DeleteThread(int thid)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    CpuSuspendIntr(&state);

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD) || thread == thctx.idle_thread) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_THID;
    }

    if (thread->status != THS_DORMANT) {
        CpuResumeIntr(state);
        return KE_NOT_DORMANT;
    }

    if (thread->attr & TH_CLEAR_STACK) {
        memset(thread->stack_top, 0, thread->stack_size);
    }

    FreeSysMemory(thread->stack_top);
    list_remove(&thread->queue);
    list_remove(&thread->thread_list);
    heap_free(&thread->tag);

    CpuResumeIntr(state);

    return KE_OK;
}

int StartThread(int thid, void *arg)
{
    struct thread *thread;
    u32 reg_offset;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    CpuSuspendIntr(&state);

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_THID;
    }

    if (thread->status != THS_DORMANT) {
        CpuResumeIntr(state);
        return KE_NOT_DORMANT;
    }

    // zero out register state
    reg_offset         = ALIGN(thread->stack_size) - RESERVED_REGCTX_SIZE;
    thread->saved_regs = thread->stack_top + reg_offset;
    memset(thread->saved_regs, 0, RESERVED_REGCTX_SIZE);

    thread->saved_regs->a0 = (u32)arg;

    return thread_init_and_start(thread, state);
}

int StartThreadArgs(int thid, int args, void *argp)
{
    struct thread *thread;
    u32 arg_offset, reg_offset;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    CpuSuspendIntr(&state);

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_THID;
    }

    if (thread->status != THS_DORMANT) {
        CpuResumeIntr(state);
        return KE_NOT_DORMANT;
    }

    // stash the args at the bottom of stack
    arg_offset = ALIGN(thread->stack_size) - ALIGN(args);
    if (args > 0 && argp) {
        memcpy(thread->stack_top + arg_offset, argp, args);
    }

    // BUG: memset was done before setting saved_regs in the struct
    // would probably derefence null on a newly created thread
    // memset(thread->saved_regs, 0, RESERVED_REGCTX_SIZE);

    reg_offset         = arg_offset - RESERVED_REGCTX_SIZE;
    thread->saved_regs = thread->stack_top + reg_offset;

    memset(thread->saved_regs, 0, RESERVED_REGCTX_SIZE);

    thread->saved_regs->a0 = args;
    thread->saved_regs->a1 = (u32)thread->stack_top + arg_offset;

    return thread_init_and_start(thread, state);
}

int ExitThread()
{
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    thctx.current_thread->status = THS_DORMANT;
    list_insert(&thctx.dormant_queue, &thctx.current_thread->queue);
    thctx.run_next = NULL;
    thread_leave(0, 0, state, 1);

    Kprintf("panic ! Thread DORMANT !\n");
    asm __volatile__("break 1");

    return KE_OK;
}

int ExitDeleteThread()
{
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    thctx.current_thread->status = THS_DORMANT;
    list_insert(&thctx.delete_queue, &thctx.current_thread->queue);
    thctx.run_next = NULL;
    thread_leave(0, 0, state, 1);

    Kprintf("panic ! Thread ExitDeleted !\n");
    asm __volatile__("break 1");

    return KE_OK;
}

int TerminateThread(int thid)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_THID;
    }

    if (thread->status == THS_DORMANT) {
        CpuResumeIntr(state);
        return KE_DORMANT;
    }

    if (thread->status == THS_READY) {
        readyq_remove(thread, thread->priority);
    } else {
        list_remove(&thread->queue);
        if (thread->wait_type == TSW_DELAY) {
            CancelAlarm(thread_delay_cb, thread);
        } else if (thread->wait_type >= TSW_DELAY && thread->wait_type <= TSW_FPL) {
            thread->wait_event->waiter_count--;
        }
    }

    thread->status = THS_DORMANT;
    list_insert(&thctx.dormant_queue, &thread->queue);

    CpuResumeIntr(state);
    return KE_OK;
}

int iTerminateThread(int thid)
{
    struct thread *thread;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    if (thread->status == THS_DORMANT) {
        return KE_DORMANT;
    }

    if (thread == thctx.current_thread) {
        thctx.run_next = NULL;
    } else {
        if (thread->status == THS_READY) {
            readyq_remove(thread, thread->priority);
        } else {
            list_remove(&thread->queue);

            if (thread->status == THS_WAIT) {
                if (thread->wait_type == TSW_DELAY) {
                    iCancelAlarm(thread_delay_cb, thread);
                } else {
                    thread->wait_event->waiter_count--;
                }
            }
        }
    }

    thread->status = THS_DORMANT;
    list_insert(&thctx.dormant_queue, &thread->queue);

    return KE_OK;
}

int DisableDispatchThread(void)
{
    return KE_ERROR;
}

int EnableDispatchThread(void)
{
    return KE_ERROR;
}

int ChangeThreadPriority(int thid, int priority)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    if (thid == 0) {
        thread = thctx.current_thread;
    } else {
        thread = HANDLE_PTR(thid);
        if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
            CpuResumeIntr(state);
            return KE_UNKNOWN_THID;
        }

        if (thread->status == THS_DORMANT) {
            CpuResumeIntr(state);
            return KE_DORMANT;
        }
    }

    if (priority) {
        if (priority - 1 >= 126) {
            CpuResumeIntr(state);
            return KE_ILLEGAL_PRIORITY;
        }
    } else {
        priority = thctx.current_thread->priority;
    }

    if (thread == thctx.current_thread) {
        if (priority >= readyq_highest()) {
            thread->status   = THS_READY;
            thread->priority = priority;
            readyq_insert_back(thread);
            thctx.run_next = NULL;

            return thread_leave(KE_OK, 0, state, 0);
        }

        thread->priority = priority;
    } else {
        if (thread->status == THS_READY) {
            readyq_remove(thread, thread->priority);
            thread->priority = priority;

            return thread_start(thread, state);
        }

        // BUG: there was no check for TSW_DELAY here
        // in which case there would be a usec value in the wait_event union.
        //
        // Added check to prevent dereferencing garbage.

        if (thread->status == THS_WAIT && thread->wait_type != TSW_DELAY) {
            if (thread->wait_event->attr & SA_THPRI) {
                waitlist_insert(thread, thread->wait_event, priority);
            }
        }
    }

    CpuResumeIntr(state);
    return KE_OK;
}

int iChangeThreadPriority(int thid, int priority)
{
    struct thread *thread;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    if (thread->status == THS_DORMANT) {
        return KE_DORMANT;
    }

    if (priority == 0) {
        priority = thctx.current_thread->priority;
    }

    if (priority - 1 >= 126) {
        return KE_ILLEGAL_PRIORITY;
    }

    if (thread == thctx.current_thread) {
        thread->status = THS_READY;
    } else {
        if (thread->status != THS_READY) {
            thread->priority = priority;
            if (thread->status != THS_WAIT || thread->wait_type == TSW_DELAY) {
                return KE_OK;
            }

            if ((thread->wait_event->attr & 1) == 0) {
                return 0;
            }

            waitlist_insert(thread, thread->wait_event, priority);
        }
    }

    thread->priority = priority;
    readyq_insert_back(thread);
    thctx.run_next = NULL;

    return KE_OK;
}

int RotateThreadReadyQueue(int priority)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (priority >= 127) {
        return KE_ILLEGAL_PRIORITY;
    }

    CpuSuspendIntr(&state);

    thread = thctx.current_thread;

    if (priority == 0) {
        priority = thread->priority;
    }

    if (list_empty(&thctx.ready_queue[priority])) {
        CpuResumeIntr(state);
        return KE_OK;
    }

    if (priority != thread->priority) {
        thread = list_first_entry(&thctx.ready_queue[priority], struct thread, queue);
        list_remove(&thread->queue);
        list_insert(&thctx.ready_queue[priority], &thread->queue);

        CpuResumeIntr(state);
        return KE_OK;
    }

    thread->status = THS_READY;
    readyq_insert_back(thread);
    thctx.run_next = 0;
    return thread_leave(KE_OK, 0, state, 0);
}

int iRotateThreadReadyQueue(int priority)
{
    struct thread *thread;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    thread = thctx.current_thread;

    if (priority) {
        if (priority >= 126) {
            return KE_ILLEGAL_PRIORITY;
        }
    } else {
        priority = readyq_highest();
        if (thread->priority < priority) {
            priority = thread->priority;
        }
    }

    if (list_empty(&thctx.ready_queue[priority])) {
        return KE_OK;
    }

    if (priority == thread->priority) {
        thread->status = THS_READY;
        readyq_insert_back(thread);
        thctx.run_next = NULL;
    } else {
        thread = list_first_entry(&thctx.ready_queue[priority], struct thread, queue);
        list_remove(&thread->queue);
        list_insert(&thctx.ready_queue[priority], &thread->queue);
    }

    return KE_OK;
}

int ReleaseWaitThread(int thid)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    CpuSuspendIntr(&state);

    thread = HANDLE_PTR(thid);
    if (thread == thctx.current_thread) {
        CpuResumeIntr(state);
        return KE_ILLEGAL_THID;
    }

    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_THID;
    }

    if (thread->status != THS_WAIT) {
        CpuResumeIntr(state);
        return KE_NOT_WAIT;
    }

    thread->saved_regs->v0 = KE_RELEASE_WAIT;
    list_remove(&thread->queue);
    thread->status = THS_READY;

    if (thread->wait_type == TSW_DELAY) {
        CancelAlarm(thread_delay_cb, thread);
    } else {
        thread->wait_event->waiter_count--;
    }

    return thread_start(thread, state);
}

int iReleaseWaitThread(int thid)
{
    struct thread *thread;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    thread = HANDLE_PTR(thid);

    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    if (thread->status != THS_WAIT) {
        return KE_NOT_WAIT;
    }

    thread->saved_regs->v0 = KE_RELEASE_WAIT;
    list_remove(&thread->queue);
    thread->status = THS_READY;

    if (thread->wait_type == TSW_DELAY) {
        iCancelAlarm(thread_delay_cb, thread);
    } else {
        thread->wait_event->waiter_count--;
    }

    readyq_insert_back(thread);
    thctx.run_next = NULL;

    return KE_OK;
}

int GetThreadId(void)
{
    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    return MAKE_HANDLE(thctx.current_thread);
}

int CheckThreadStack(void)
{
    if (QueryIntrContext()) {
        return KE_OK;
    }

    return check_thread_stack();
}

int ReferThreadStatus(int thid, iop_thread_info_t *info)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    thread = refer_thread(thid, 1);
    if (thread < 0) {
        CpuResumeIntr(state);
        return (int)thread;
    }

    thread_get_status(thread, info);

    CpuResumeIntr(state);

    return KE_OK;
}

int iReferThreadStatus(int thid, iop_thread_info_t *info)
{
    struct thread *thread;

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    thread = HANDLE_PTR(thid);

    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    thread_get_status(thread, info);

    return KE_OK;
}

int SleepThread(void)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: SleepThread KE_CAN_NOT_WAIT\n");
    }
    check_thread_stack();

    thread = thctx.current_thread;

    if (thread->wakeup_count != 0) {
        thread->wakeup_count--;
        CpuResumeIntr(state);
        return KE_OK;
    }

    thread->status     = THS_WAIT;
    thread->wait_type  = TSW_SLEEP;
    thread->wait_event = NULL;
    // thread->wait_return = 0;
    thctx.run_next = NULL;

    list_insert(&thctx.sleep_queue, &thread->queue);

    return thread_leave(KE_OK, 0, state, 1);
}

int WakeupThread(int thid)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    CpuSuspendIntr(&state);

    thread = HANDLE_PTR(thid);
    if (thread == thctx.current_thread) {
        return KE_ILLEGAL_THID;
    }

    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    if (thread->status == THS_DORMANT) {
        CpuResumeIntr(state);
        return KE_DORMANT;
    }

    if (thread->status == THS_WAIT && thread->wait_type == TSW_SLEEP) {
        list_remove(&thread->queue);
        thread->status = THS_READY;

        return thread_start(thread, state);
    }

    thread->wakeup_count--;

    CpuResumeIntr(state);

    return KE_OK;
}

int iWakeupThread(int thid)
{
    struct thread *thread;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    if (thread->status == THS_DORMANT) {
        return KE_DORMANT;
    }

    if (thread->status == THS_WAIT && thread->wait_type == TSW_SLEEP) {
        list_remove(&thread->queue);
        thread->status = THS_READY;
        readyq_insert_back(thread);
        thctx.run_next = NULL;
    } else {
        thread->wakeup_count++;
    }

    return KE_OK;
}

int CancelWakeupThread(int thid)
{
    struct thread *thread;
    int state, wakeup_count;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    if (thid) {
        thread = HANDLE_PTR(thid);
        if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
            CpuResumeIntr(state);
            return KE_UNKNOWN_THID;
        }
    } else {
        thread = thctx.current_thread;
    }

    wakeup_count         = thread->wakeup_count;
    thread->wakeup_count = 0;

    CpuResumeIntr(state);
    return wakeup_count;
}

int iCancelWakeupThread(int thid)
{
    struct thread *thread;
    int wakeup_count;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (thid == 0) {
        return KE_ILLEGAL_THID;
    }

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return KE_UNKNOWN_THID;
    }

    wakeup_count         = thread->wakeup_count;
    thread->wakeup_count = 0;

    return wakeup_count;
}

int SuspendThread(int thid)
{
    return KE_ERROR;
}

int iSuspendThread(int thid)
{
    return KE_ERROR;
}

int ResumeThread(int thid)
{
    return KE_ERROR;
}

int iResumeThread(int thid)
{
    return KE_ERROR;
}

int DelayThread(int usec)
{
    iop_sys_clock_t clock;
    struct thread *thread;
    int ret, state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    USec2SysClock(usec, &clock);
    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: DelayThread KE_CAN_NOT_WAIT\n");
    }
    check_thread_stack();

    thread = thctx.current_thread;

    ret = SetAlarm(&clock, thread_delay_cb, thread);
    if (ret != KE_OK) {
        CpuResumeIntr(state);
        return ret;
    }

    thread->status     = THS_WAIT;
    thread->wait_type  = TSW_DELAY;
    thread->wait_usecs = usec;
    // thread->wait_return = 0;

    thctx.run_next = NULL;
    list_insert(&thctx.delay_queue, &thread->queue);

    return thread_leave(KE_OK, 0, state, 1);
}


int GetSystemTime(iop_sys_clock_t *sys_clock)
{
    return CpuInvokeInKmode(read_sys_time, sys_clock);
}

int SetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *), void *arg)
{
    iop_sys_clock_t systime;
    struct alarm *alarm;
    int state;
    u64 time;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    if (!list_empty(&thctx.alarm)) {
        list_for_each (alarm, &thctx.alarm, alarm_list) {
            if (alarm->cb == alarm_cb && alarm->userptr == arg) {
                CpuResumeIntr(state);
                return KE_FOUND_HANDLER;
            }
        }
    }

    alarm = alarm_alloc();
    if (!alarm) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    if (sys_clock->hi == 0 && sys_clock->lo < thctx.min_wait) {
        sys_clock->lo = thctx.min_wait;
    }

    GetSystemTime(&systime);
    // as_u64(systime.hi, systime.lo); // ???

    alarm->target  = add64(sys_clock->hi, sys_clock->lo, systime.hi, systime.lo);
    alarm->cb      = alarm_cb;
    alarm->userptr = arg;
    thctx.alarm_count++;
    alarm_insert(&thctx.alarm, alarm);

    GetSystemTime(&systime);
    time = as_u64(systime.hi, systime.lo);
    update_timer_compare(thctx.timer_id, time, &thctx.alarm);

    CpuResumeIntr(state);

    return KE_OK;
}

int iSetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *), void *arg)
{
    struct alarm *alarm;
    iop_sys_clock_t systime;
    u64 time;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    list_for_each (alarm, &thctx.alarm, alarm_list) {
        if (alarm->cb == alarm_cb && alarm->userptr == arg) {
            return KE_FOUND_HANDLER;
        }
    }

    alarm = alarm_alloc();
    if (!alarm) {
        return KE_NO_MEMORY;
    }

    if (sys_clock->hi == 0 && sys_clock->lo < thctx.min_wait) {
        sys_clock->lo = thctx.min_wait;
    }

    read_sys_time(&systime);
    // as_u64(systime.hi, systime.lo);
    alarm->target  = add64(sys_clock->hi, sys_clock->lo, systime.hi, systime.lo);
    alarm->cb      = alarm_cb;
    alarm->userptr = arg;
    thctx.alarm_count++;
    alarm_insert(&thctx.alarm, alarm);
    read_sys_time(&systime);
    time = as_u64(systime.hi, systime.lo);
    update_timer_compare(thctx.timer_id, time, &thctx.alarm);

    return KE_OK;
}

int CancelAlarm(unsigned int (*alarm_cb)(void *), void *arg)
{
    struct alarm *alarm;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    if (list_empty(&thctx.alarm)) {
        CpuResumeIntr(state);
        return KE_NOTFOUND_HANDLER;
    }

    list_for_each (alarm, &thctx.alarm, alarm_list) {
        if (alarm->cb == alarm_cb && alarm->userptr == arg) {
            list_remove(&alarm->alarm_list);
            alarm_free(alarm);
            thctx.alarm_count--;

            CpuResumeIntr(state);
            return KE_OK;
        }
    }

    CpuResumeIntr(state);

    return KE_NOTFOUND_HANDLER;
}

int iCancelAlarm(unsigned int (*alarm_cb)(void *), void *arg)
{
    struct alarm *alarm;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (list_empty(&thctx.alarm)) {
        return KE_NOTFOUND_HANDLER;
    }

    list_for_each (alarm, &thctx.alarm, alarm_list) {
        if (alarm->cb == alarm_cb && alarm->userptr == arg) {
            list_remove(&alarm->alarm_list);
            alarm_free(alarm);
            thctx.alarm_count--;

            return KE_OK;
        }
    }

    return KE_NOTFOUND_HANDLER;
}

void USec2SysClock(u32 usec, iop_sys_clock_t *sys_clock)
{
    sys_clock->hi = 0;
    sys_clock->lo = usec;
    clock_mul(sys_clock, sys_clock, thctx.unk_clock_mult);
    clock_div(sys_clock, sys_clock, thctx.unk_clock_div, NULL);
}

void SysClock2USec(iop_sys_clock_t *sys_clock, u32 *sec, u32 *usec)
{
    iop_sys_clock_t clock;
    clock_mul(&clock, sys_clock, thctx.unk_clock_div);
    clock_div(&clock, &clock, thctx.unk_clock_mult, NULL);
    clock_div(&clock, &clock, 1000000, usec);
    *sec = clock.lo;
}

int GetSystemStatusFlag()
{
    return thctx.sytem_status_flag;
}

int GetThreadCurrentPriority(void)
{
    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    return thctx.current_thread->priority;
}

unsigned int GetSystemTimeLow(void)
{
    return GetTimerCounter(thctx.timer_id);
}

int ReferSystemStatus(iop_sys_status_t *info, size_t size)
{
    int state, ret;
    if (size < sizeof(*info)) {
        return KE_ERROR;
    }

    memset(info, 0, size);

    ret = CpuSuspendIntr(&state);

    if (QueryIntrContext()) {
        info->status = TSS_NOTHREAD;
    } else if (ret == KE_CPUDI) {
        info->status = TSS_DISABLEINTR;
    } else {
        info->status = TSS_THREAD;
    }

    info->systemLowTimerWidth = 32;
    info->idleClocks.hi       = thctx.idle_thread->run_clocks_hi;
    info->idleClocks.lo       = thctx.idle_thread->run_clocks_lo;
    info->threadSwitchCount   = thctx.thread_switch_count;
    info->comesOutOfIdleCount = thctx.idle_thread->irq_preemption_count;

    CpuResumeIntr(state);

    return KE_OK;
}

int ReferThreadRunStatus(int thid, iop_thread_run_status_t *stat, size_t size)
{
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    thread = refer_thread(thid, 1);
    if ((int)thread <= 0) {
        CpuResumeIntr(state);
        return (int)thread;
    }

    if (size < sizeof(*stat)) {
        CpuResumeIntr(state);
        return KE_ILLEGAL_SIZE;
    }

    memset(stat, 0, size);
    thread_get_run_stats(thread, stat);

    CpuResumeIntr(state);
    return KE_OK;
}

/*
 * Gets the minimum stack size left so far
 * only works if stack filling was not disabled
 */
int GetThreadStackFreeSize(int thid)
{
    struct thread *thread;
    u32 stack_size, i;
    u32 *stack;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    thread = refer_thread(thid, 1);
    if ((int)thread < 0) {
        CpuResumeIntr(state);
        return (int)thread;
    }

    stack      = thread->stack_top;
    stack_size = thread->stack_size / 4;
    CpuResumeIntr(state);

    for (i = 0; i < stack_size; i++) {
        if (stack[i] != -1) {
            return i * 4;
        }
    }

    return i * 4;
}


int GetThreadmanIdList(int type, int *readbuf, int readbufsize, int *objectcount)
{
    int state, write_count, obj_count;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    write_count = 0;
    obj_count   = 0;

    switch (type) {
        case TMID_Thread: {
            struct thread *thread;
            list_for_each (thread, &thctx.thread_list, thread_list) {
                if (thread != thctx.idle_thread) {
                    if (write_count < readbufsize) {
                        *readbuf++ = MAKE_HANDLE(thread);
                        write_count++;
                    }
                    obj_count++;
                }
            }
        } break;
        case TMID_Semaphore: {
            struct semaphore *sema;
            list_for_each (sema, &thctx.semaphore, sema_list) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(sema);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_EventFlag: {
            struct event_flag *evf;
            list_for_each (evf, &thctx.event_flag, evf_list) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(evf);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_Mbox: {
            struct mbox *mbx;
            list_for_each (mbx, &thctx.mbox, mbox_list) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(mbx);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_Vpl: {
            struct vpool *vpl;
            list_for_each (vpl, &thctx.vpool, vpl_list) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(vpl);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_Fpl: {
            struct fpool *fpl;
            list_for_each (fpl, &thctx.fpool, fpl_list) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(fpl);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_SleepThread: {
            struct thread *thread;
            list_for_each (thread, &thctx.sleep_queue, queue) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(thread);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_DelayThread: {
            struct thread *thread;
            list_for_each (thread, &thctx.delay_queue, queue) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(thread);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        case TMID_DormantThread: {
            struct thread *thread;
            list_for_each (thread, &thctx.dormant_queue, queue) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(thread);
                    write_count++;
                }
                obj_count++;
            }
        } break;
        default: {
            struct heaptag *tag = HANDLE_PTR(type);
            struct thread *thread;
            struct event *event;

            if (type < 0 || tag->tag < TAG_SEMA || tag->tag > TAG_FPL || tag->id != HANDLE_ID(type)) {
                CpuResumeIntr(state);
                return KE_ILLEGAL_TYPE;
            }

            switch (tag->tag) {
                case TAG_SEMA:
                    event = &((struct semaphore *)tag)->event;
                    break;
                case TAG_EVF:
                    event = &((struct event_flag *)tag)->event;
                    break;
                case TAG_MBX:
                    event = &((struct mbox *)tag)->event;
                    break;
                case TAG_VPL:
                    event = &((struct vpool *)tag)->event;
                    break;
                case TAG_FPL:
                    event = &((struct fpool *)tag)->event;
                    break;
            }

            list_for_each (thread, &event->waiters, queue) {
                if (write_count < readbufsize) {
                    *readbuf++ = MAKE_HANDLE(thread);
                    write_count++;
                }
                obj_count++;
            }
        }
    }

    CpuResumeIntr(state);

    if (objectcount) {
        *objectcount = obj_count;
    }

    return write_count;
}

static void clock_mul(iop_sys_clock_t *dst, iop_sys_clock_t *src, u32 mul)
{
    u64 res;
    res = (u64)src->hi << 32 | src->lo;
    res *= mul;
    if (dst) {
        dst->hi = res >> 32;
        dst->lo = res;
    }
}

// TODO clean up
static void clock_div(iop_sys_clock_t *dst, iop_sys_clock_t *src, u32 d, u32 *r)
{
    int v4;
    u32 hi;
    u32 lo;
    u32 v7;
    u32 v8;
    u32 v9;
    int i;
    unsigned int v11;
    u32 v12;
    u32 v13;

    v4 = 0;
    hi = src->hi;
    lo = src->lo;
    v7 = hi / d;
    v8 = hi % d;
    v9 = v7;
    for (i = 0; i < 4; ++i) {
        v11 = (v8 << 8) | (lo >> 24);
        lo <<= 8;
        v12 = v11 / d;
        v4  = (v4 << 8) | (v9 >> 24);
        v13 = v11 % d;
        v8  = v11 % d;
        v9  = (v9 << 8) + v12;
    }
    if (dst) {
        dst->hi = v4;
        dst->lo = v9;
    }
    if (r) {
        *r = v13;
    }
}


static struct thread *refer_thread(int thid, int current)
{
    struct thread *thread;

    if (!thid && current) {
        return thctx.current_thread;
    }

    thread = HANDLE_PTR(thid);
    if (!HANDLE_VERIFY(thid, TAG_THREAD)) {
        return (struct thread *)KE_UNKNOWN_THID;
    }

    return thread;
}

static void thread_get_status(struct thread *thread, iop_thread_info_t *info)
{
    memset(info, 0, sizeof(*info));
    info->status          = thread->status;
    info->currentPriority = thread->priority;
    info->initPriority    = thread->init_priority;
    info->entry           = thread->entry;
    info->stack           = thread->stack_top;
    info->stackSize       = thread->stack_size;
    info->gpReg           = (void *)thread->gp;
    info->attr            = thread->attr;
    info->option          = thread->option;

    if (thread->status == THS_WAIT) {
        info->waitType = thread->wait_type;
        if (thread->wait_type == TSW_DELAY) {
            info->waitId = thread->wait_usecs;
        } else {
            switch (thread->wait_type) {
                case TSW_SEMA:
                    info->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct semaphore, event));
                    break;
                case TSW_EVENTFLAG:
                    info->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct event_flag, event));
                    break;
                case TSW_MBX:
                    info->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct mbox, event));
                    break;
                case TSW_FPL:
                    info->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct fpool, event));
                    break;
                case TSW_VPL:
                    info->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct vpool, event));
                    break;
                default:
                    // shouldn't happen
                    break;
            }
        }
    }

    info->wakeupCount = thread->wakeup_count;
    if (thread->status == THS_DORMANT || thread->status == THS_RUN) {
        info->regContext = 0;
    } else {
        info->regContext = (long *)thread->saved_regs;
    }
}

static void thread_get_run_stats(struct thread *thread, iop_thread_run_status_t *stat)
{
    stat->status          = thread->status;
    stat->currentPriority = thread->priority;

    if (thread->status == THS_WAIT) {
        stat->waitType = thread->wait_type;
        if (thread->wait_type == TSW_DELAY) {
            stat->waitId = thread->wait_usecs;
        } else {
            switch (thread->wait_type) {
                case TSW_SEMA:
                    stat->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct semaphore, event));
                    break;
                case TSW_EVENTFLAG:
                    stat->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct event_flag, event));
                    break;
                case TSW_MBX:
                    stat->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct mbox, event));
                    break;
                case TSW_FPL:
                    stat->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct fpool, event));
                    break;
                case TSW_VPL:
                    stat->waitId = MAKE_HANDLE(container_of(thread->wait_event, struct vpool, event));
                    break;
                default:
                    // shouldn't happen
                    break;
            }
        }
    }

    stat->wakeupCount = thread->wakeup_count;
    if (thread->status != THS_DORMANT && thread->status != THS_RUN) {
        stat->regContext = (long *)thread->saved_regs;
    }

    stat->runClocks.hi       = thread->run_clocks_hi;
    stat->runClocks.lo       = thread->run_clocks_lo;
    stat->intrPreemptCount   = thread->irq_preemption_count;
    stat->threadPreemptCount = thread->thread_preemption_count;
    stat->releaseCount       = thread->release_count;
}
