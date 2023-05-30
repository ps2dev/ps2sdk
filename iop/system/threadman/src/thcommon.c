#include "thcommon.h"
#include "kerr.h"
#include "thevent.h"
#include "timrman.h"
#include "sysmem.h"
#include "heaplib.h"
#include "intrman.h"
#include "loadcore.h"
#include "sysclib.h"
#include "stdio.h"

#include <limits.h>

IRX_ID("Multi_Thread_Manager", 2, 3);
extern struct irx_export_table _exp_thrdman;
extern struct irx_export_table _exp_thbase;
extern struct irx_export_table _exp_thevent;
extern struct irx_export_table _exp_thsemap;
extern struct irx_export_table _exp_thmsgbx;
extern struct irx_export_table _exp_thfpool;
extern struct irx_export_table _exp_thvpool;

struct thread_context thctx;

struct alarm *alarm_alloc()
{
    struct alarm *alarm;
    if (list_empty(&thctx.alarm_pool)) {
        alarm = heap_alloc(0, sizeof(*alarm));
        thctx.alarm_id++;
        alarm->tag.id = thctx.alarm_id;
    } else {
        alarm = list_first_entry(&thctx.alarm_pool, struct alarm, alarm_list);
        list_remove(&alarm->alarm_list);
    }

    return alarm;
}

void alarm_free(struct alarm *alarm)
{
    if (alarm->tag.id >= 33) {
        heap_free(&alarm->tag);
    } else {
        list_insert(&thctx.alarm_pool, &alarm->alarm_list);
    }
}

void alarm_insert(struct list_head *list, struct alarm *alarm)
{
    struct alarm *i;

    list_for_each (i, list, alarm_list) {
        if (alarm->target < i->target) {
            break;
        }
    }

    list_insert(&i->alarm_list, &alarm->alarm_list);
}

void waitlist_insert(struct thread *thread, struct event *event, s32 priority)
{
    struct thread *weaker;

    list_remove(&thread->queue);

    weaker = list_first_entry(&event->waiters, struct thread, queue);
    list_for_each (weaker, &event->waiters, queue) {
        if (priority < weaker->priority) {
            break;
        }
    }

    list_insert(&weaker->queue, &thread->queue);
}

void update_timer_compare(int timid, u64 time, struct list_head *alarm_list)
{
    struct alarm *prev, *i;
    u32 counter, new_compare = 0;

    // what if list is empty? (luckily its not but....)
    prev = list_first_entry(alarm_list, struct alarm, alarm_list);

    if (!list_empty(alarm_list)) {
        list_for_each (i, alarm_list, alarm_list) {
            if (i->target >= prev->target + thctx.unk4c8) {
                break;
            }

            prev = i;
        }
    }

    if (prev->target - time >= thctx.unk4c8) {
        new_compare = prev->target;
    } else {
        counter     = GetTimerCounter(timid);
        new_compare = counter + thctx.unk4c8;
    }

    SetTimerCompare(timid, new_compare);
}

unsigned int thread_delay_cb(void *user)
{
    struct thread *thread = user;

    list_remove(&thread->queue);
    thread->status = THS_READY;
    readyq_insert_back(thread);
    thctx.run_next = NULL;

    return 0;
}

int check_thread_stack()
{
    int stack_remaining;
    stack_remaining = (u32)&stack_remaining - (u32)thctx.current_thread->stack_top;

    if (stack_remaining < 0xa8) {
        CpuDisableIntr();
        Kprintf("CheckThreadStack()\n");
        thread_leave(0, 0, 0, 0);
    }

    return stack_remaining;
}

void *heap_alloc(u16 tag, u32 bytes)
{
    struct heaptag *ptr = AllocHeapMemory(thctx.heap, bytes);
    if (ptr) {
        memset(ptr, 0, bytes);
        ptr->tag = tag;
    }

    return ptr;
}

int heap_free(struct heaptag *tag)
{
    tag->tag = 0;
    return FreeHeapMemory(thctx.heap, tag);
}

/**
 * Exit the current thread, intrman will call us back to get a new context to switch to.
 * The first two arguments will get saved into v0 and v1 in the stored context.
 *
 * The current CpuDisableIntr state is passed so that it can be restored when resuming
 * the next user thread.
 *
 * Release indicates if it was a voluntary wait or not.
 */
int thread_leave(int ret1, int ret2, int intr_state, int release)
{
    register u32 a0 __asm__("a0") = ret1;
    register u32 a1 __asm__("a1") = ret2;
    register u32 a2 __asm__("a2") = intr_state;
    register s32 result __asm__("v0");

    if (!release) {
        thctx.current_thread->reason_counter = &thctx.current_thread->thread_preemption_count;
    } else {
        thctx.current_thread->reason_counter = &thctx.current_thread->release_count;
    }

    asm __volatile__("li $v0, 0x20\n"
                     "syscall\n"
                     : "=r"(result)
                     : "r"(a0), "r"(a1), "r"(a2)
                     : "memory");

    return result;
}

int thread_start(struct thread *thread, int intr_state)
{
    if (thread->priority < thctx.current_thread->priority) {
        thctx.current_thread->status = THS_READY;
        readyq_insert_front(thctx.current_thread);
        thread->status = THS_RUN;
        thctx.run_next = thread;

        return thread_leave(KE_OK, 0, intr_state, 0);
    } else {
        thread->status = THS_READY;
        readyq_insert_back(thread);

        CpuResumeIntr(intr_state);
        return KE_OK;
    }
}

int thread_init_and_start(struct thread *thread, int intr_state)
{
    thread->wait_type       = 0;
    thread->wait_usecs      = 0;
    thread->wakeup_count    = 0;
    thread->priority        = thread->init_priority;
    thread->saved_regs->unk = -2;
    thread->saved_regs->sp  = (u32)&thread->saved_regs[1];
    thread->saved_regs->fp  = thread->saved_regs->sp;
    thread->saved_regs->ra  = (u32)ExitThread;
    thread->saved_regs->gp  = thread->gp;
    thread->saved_regs->sr  = 0x404;
    thread->saved_regs->sr |= thread->attr & 8;
    thread->saved_regs->pc     = (u32)thread->entry;
    thread->saved_regs->I_CTRL = 1;

    list_remove(&thread->queue);

    return thread_start(thread, intr_state);
}

int post_boot_callback_1(iop_init_entry_t *next, int delayed)
{
    CpuEnableIntr();
    printf("\r\nIOP Realtime Kernel Ver. 2.2\r\n    Copyright 1999-2002 (C) Sony Computer Entertainment Inc. \r\n");
    return 0;
}

int post_boot_callback_2(iop_init_entry_t *next, int delayed)
{
    CpuEnableIntr();
    ChangeThreadPriority(0, 126);
    if (!next->callback) {
        while (1) {
            DelayThread(1000000);
        }
    }

    return 0;
}

int read_sys_time(iop_sys_clock_t *clock)
{
    u32 hi      = thctx.time_hi;
    u32 counter = GetTimerCounter(thctx.timer_id);

    if (counter >= thctx.time_lo) {
        thctx.time_lo = counter;
    } else {
        hi++;
    }

    if (clock) {
        clock->hi = hi;
        clock->lo = counter;
    }

    return 0;
}

// didn't try to figure out the original algo
// just grabbed this one
static u32 ntz(u32 x)
{
    u32 n;

    if (x == 0)
        return (32);
    n = 1;
    if ((x & 0x0000FFFF) == 0) {
        n = n + 16;
        x = x >> 16;
    }
    if ((x & 0x000000FF) == 0) {
        n = n + 8;
        x = x >> 8;
    }
    if ((x & 0x0000000F) == 0) {
        n = n + 4;
        x = x >> 4;
    }
    if ((x & 0x00000003) == 0) {
        n = n + 2;
        x = x >> 2;
    }
    return n - (x & 1);
}

u32 readyq_highest()
{
    for (int i = 0; i < 4; i++) {
        if (thctx.queue_map[i]) {
            return ntz(thctx.queue_map[i]) + 32 * i;
        }
    }

    return 128;
}

void report_stack_overflow(struct thread *thread)
{
    ModuleInfo_t *img_info;
    char *name;

    Kprintf("\nThread (thid=%x, #%d) stack overflow\n Stack = %x, Stack size = %x, SP=%x\n",
            MAKE_HANDLE(thread),
            thread->tag.id,
            thread->stack_top,
            thread->stack_size,
            thread->saved_regs);

    img_info = FindImageInfo(thread->entry);
    if (img_info) {
        name = img_info->name;
        if (name) {
            Kprintf(" Module Name = %s\n", name);
        }
    }

    asm __volatile__("break 1");
}

void do_delete_thread()
{
    struct thread *thread;

    while (!list_empty(&thctx.delete_queue)) {
        thread = list_first_entry(&thctx.delete_queue, struct thread, queue);
        if (thread->attr & TH_CLEAR_STACK) {
            memset(thread->stack_top, 0, thread->stack_size);
        }

        FreeSysMemory(thread->stack_top);
        list_remove(&thread->queue);
        list_remove(&thread->thread_list);
        heap_free(&thread->tag);
    }
}

void schedule_next()
{
    struct thread *cur, *new;
    u32 prio;

    cur            = thctx.current_thread;
    thctx.run_next = thctx.current_thread;

    prio = readyq_highest();

    // originally would fall down and hit the bottom kprintf
    // but i don't want the nesting
    if (prio >= 128) {
        Kprintf("Panic: not found ready Thread\n");
        return;
    }

    new = list_first_entry(&thctx.ready_queue[prio], struct thread, queue);

    if (thctx.current_thread->status == THS_RUN) {
        if (thctx.debug_flags & 4) {
            Kprintf("    THS_RUN cp=%d : hp=%d ", cur->priority, prio);
        }

        if (prio < cur->priority) {
            if (thctx.debug_flags & 4) {
                Kprintf("  readyq = %x, newrun = %x:%d, prio = %d",
                        &thctx.ready_queue[prio],
                        new,
                        new->tag.id,
                        prio);
            }

            readyq_remove(new, prio);
            new->status    = THS_RUN;
            thctx.run_next = new;
            cur->status    = THS_READY;
            readyq_insert_front(cur);
        }
    } else {
        if (thctx.debug_flags & 4) {
            Kprintf("    not THS_RUN ");

            Kprintf(" readyq = %x, newrun = %x:%d, prio = %d",
                    &thctx.ready_queue[prio],
                    new,
                    new->tag.id,
                    prio);
        }

        readyq_remove(new, prio);
        new->status    = THS_RUN;
        thctx.run_next = new;
    }

    if ((thctx.debug_flags & 4) != 0)
        Kprintf("\n");
}

struct regctx *new_context_cb(struct regctx *ctx)
{
    u64 new_time;
    u32 timer;

    if ((thctx.debug_flags & 3) != 0) {
        if ((thctx.debug_flags & 3) == 1)
            Kprintf("[%3d->", thctx.current_thread->tag.id);
        if ((thctx.debug_flags & 3) == 2)
            Kprintf("switch_context(%x:%x,pc=%x,ei=%x =>%x:%d)\n",
                    ctx,
                    ctx->unk,
                    ctx->pc,
                    ctx->I_CTRL,
                    thctx.current_thread,
                    thctx.current_thread->tag.id);
    }

    thctx.current_thread->saved_regs = ctx;
    if ((u32)ctx < (u32)thctx.current_thread->stack_top) {
        report_stack_overflow(thctx.current_thread);
    }

    if (!thctx.run_next) {
        if (!list_empty(&thctx.delete_queue)) {
            do_delete_thread();
        }

        schedule_next();
    }

    if (thctx.current_thread == thctx.run_next) {
        thctx.thread_resume_count++;
    } else {
        timer    = thctx.timer_func();
        new_time = add64(0, timer - thctx.last_timer, thctx.current_thread->run_clocks_hi, thctx.current_thread->run_clocks_lo);

        thctx.current_thread->run_clocks_lo = (u32)new_time;
        thctx.current_thread->run_clocks_hi = (u32)(new_time >> 32);
        thctx.thread_switch_count++;
        (*thctx.current_thread->reason_counter)++;
    }

    thctx.current_thread = thctx.run_next;

    if ((thctx.debug_flags & 3) != 0) {
        if ((thctx.debug_flags & 3) == 1)
            Kprintf("%3d]", thctx.run_next->tag.id);
        if ((thctx.debug_flags & 3) == 2)
            Kprintf("   switch_context --> %x:%x,pc=%x,ei=%x =>%x:%d\n",
                    thctx.run_next->saved_regs,
                    thctx.run_next->saved_regs->unk,
                    thctx.run_next->saved_regs->pc,
                    thctx.run_next->saved_regs->I_CTRL,
                    thctx.run_next,
                    thctx.run_next->tag.id);
    }

    // some sort of debug display?
    if (thctx.debug_flags & 0x20) {
        _sw(~(1 << ((thctx.run_next->tag.id - 1) & 7)), 0xbf802070);
    }

    return thctx.run_next->saved_regs;
}

int preempt_cb(int unk)
{
    if (thctx.run_next != thctx.current_thread) {
        thctx.current_thread->reason_counter = &thctx.current_thread->irq_preemption_count;
        return 1;
    }

    return 0;
}

void idle_thread()
{
    while (1)
        ;
}

int timer_handler(void *user)
{
    struct thread_context *thctx = user;
    struct alarm *alarm;
    u32 status, counter, ret;
    u64 time = 0;

    status  = GetTimerStatus(thctx->timer_id);
    counter = GetTimerCounter(thctx->timer_id);

    // overflow
    if (status & 0x1000) {
        thctx->time_hi++;
        thctx->time_lo = counter;
    }

    // compare
    if (status & 0x800) {
        list_for_each_safe (alarm, &thctx->alarm, alarm_list) {
            counter = GetTimerCounter(thctx->timer_id);
            status  = GetTimerStatus(thctx->timer_id);
            if (counter < thctx->time_lo && (status & 0x1000)) {
                thctx->time_hi++;
                thctx->time_lo = counter;
            }

            time = as_u64(thctx->time_hi, counter);
            if (time < alarm->target) {
                break;
            }

            // alarm has fired, remove, update, and reschedule
            list_remove(&alarm->alarm_list);

            if (alarm->tag.id == 1) {
                alarm->target += 0x100000000;
            } else {
                ret = alarm->cb(alarm->userptr);
                if (!ret) {
                    alarm_free(alarm);
                    thctx->alarm_count--;
                    continue;
                }

                if (ret < thctx->min_wait) {
                    ret = thctx->min_wait;
                }

                alarm->target += ret;
            }

            alarm_insert(&thctx->alarm, alarm);
        }

        update_timer_compare(thctx->timer_id, time, &thctx->alarm);
    }


    return 1;
}


void init_timer()
{
    iop_sys_clock_t compare;
    s32 timer_id, timer_irq;
    struct alarm *alarm;
    int *bootmode;
    int state;

    thctx.unk_clock_mult = 0x1200;
    thctx.unk_clock_div  = 125;

    bootmode = QueryBootMode(7);
    if (bootmode && *bootmode == 200) {
        thctx.unk_clock_mult = 25;
        thctx.unk_clock_div  = 1;
    }

    USec2SysClock(100, &compare);

    thctx.min_wait = compare.lo;
    thctx.unk4c8   = 2 * compare.lo;

    timer_id         = AllocHardTimer(1, 32, 1);
    thctx.timer_id   = timer_id;
    thctx.timer_func = GetTimerReadFunc(timer_id);
    timer_irq        = GetHardTimerIntrCode(timer_id);
    RegisterIntrHandler(timer_irq, 1, timer_handler, &thctx);

    list_init(&thctx.alarm);
    list_init(&thctx.alarm_pool);
    thctx.alarm_id = 0;
    CpuSuspendIntr(&state);
    alarm = alarm_alloc();
    list_insert(&thctx.alarm, &alarm->alarm_list);
    USec2SysClock(2000, &compare);
    // hmm
    alarm->target = 0x100000000LL - compare.lo;

    thctx.alarm_count = 1;

    for (int i = 0; i < 32; i++) {
        alarm = heap_alloc(0, sizeof(*alarm));
        thctx.alarm_id++;
        alarm->tag.id = thctx.alarm_id;
        alarm_free(alarm);
    }

    SetTimerMode(timer_id, 0);
    SetTimerCompare(timer_id, compare.lo);
    SetTimerCounter(timer_id, 0);
    SetTimerMode(timer_id, 0x70);
    EnableIntr(GetHardTimerIntrCode(timer_id));
    CpuResumeIntr(state);
}


int _start(int argc, char **argv)
{
    struct thread *idle, *current;
    iop_event_t flag;
    int *BootMode;
    int state;
    int i;

    if (RegisterNonAutoLinkEntries(&_exp_thrdman)) {
        return MODULE_NO_RESIDENT_END;
    }

    if (RegisterLibraryEntries(&_exp_thbase)) {
        return MODULE_NO_RESIDENT_END;
    }

    CpuSuspendIntr(&state);
    RegisterLibraryEntries(&_exp_thevent);
    RegisterLibraryEntries(&_exp_thsemap);
    RegisterLibraryEntries(&_exp_thmsgbx);
    RegisterLibraryEntries(&_exp_thfpool);
    RegisterLibraryEntries(&_exp_thvpool);

    memset(&thctx, 0, sizeof(thctx));
    thctx.debug_flags = DEBUG_FLAGS;

    list_init(&thctx.semaphore);
    list_init(&thctx.event_flag);
    list_init(&thctx.mbox);
    list_init(&thctx.vpool);
    list_init(&thctx.fpool);
    list_init(&thctx.sleep_queue);
    list_init(&thctx.delay_queue);
    // list_init(&thctx.unused_list1);
    // list_init(&thctx.unused_list2);
    list_init(&thctx.dormant_queue);
    list_init(&thctx.delete_queue);
    list_init(&thctx.thread_list);

    for (int i = 0; i < 128; i++) {
        list_init(&thctx.ready_queue[i]);
    }

    thctx.heap = CreateHeap(2048, 1);

    // Create the idle thread
    idle                = heap_alloc(TAG_THREAD, sizeof(*idle));
    idle->tag.id        = ++thctx.thread_id;
    idle->stack_size    = 512;
    idle->stack_top     = AllocSysMemory(1, 512, 0);
    idle->init_priority = 127;
    idle->priority      = 127;
    idle->attr          = TH_C;
    idle->status        = THS_READY;
    idle->entry         = idle_thread;
    idle->saved_regs    = idle->stack_top + (((idle->stack_size << 2) >> 2) - RESERVED_REGCTX_SIZE);
    memset(idle->saved_regs, 0, RESERVED_REGCTX_SIZE);

    asm __volatile__("sw $gp, 0(%0)\n"
                     : "=r"(idle->gp)::);

    idle->saved_regs->unk = -2;
    idle->saved_regs->sp  = (u32)&idle->saved_regs[1];
    idle->saved_regs->gp  = idle->gp;
    idle->saved_regs->fp  = idle->saved_regs->sp;
    idle->saved_regs->ra  = (u32)ExitThread;
    idle->saved_regs->sr  = (idle->attr & 0xF0000000) | 0x404;
    idle->saved_regs->sr |= idle->attr & 8;
    idle->saved_regs->pc     = (u32)idle->entry;
    idle->saved_regs->I_CTRL = 1;

    list_insert(&thctx.thread_list, &idle->thread_list);
    thctx.idle_thread = idle;
    readyq_insert_back(idle);

    // Create a thread entry for our current state
    current         = heap_alloc(TAG_THREAD, sizeof(*current));
    current->tag.id = ++thctx.thread_id;
    // Taking the address of a stack variable to get
    // the allocated stack and size. Cute.
    current->stack_size    = QueryBlockSize(&i);
    current->stack_top     = QueryBlockTopAddress(&i);
    current->init_priority = 8;
    current->priority      = 1;
    current->attr          = TH_C;
    current->status        = THS_RUN;

    asm __volatile__("sw $gp, 0(%0)\n"
                     : "=r"(current->gp)::);

    list_insert(&thctx.thread_list, &current->thread_list);
    thctx.current_thread = current;
    thctx.run_next       = current;
    current->queue.next  = NULL;
    current->queue.prev  = NULL;

    SetNewCtxCb(new_context_cb);
    SetShouldPreemptCb(preempt_cb);
    init_timer();

    flag.attr               = EA_MULTI;
    flag.bits               = 0;
    flag.option             = 0;
    thctx.sytem_status_flag = CreateEventFlag(&flag);

    BootMode = QueryBootMode(4);
    if (BootMode) {
        SetEventFlag(thctx.sytem_status_flag, 1 << (*BootMode & 3));
    }

    RegisterPostBootCallback(post_boot_callback_1, 2, 0);
    RegisterPostBootCallback(post_boot_callback_2, 3, 0);

    // mismatched with suspend?
    CpuEnableIntr();

    return MODULE_RESIDENT_END;
}
