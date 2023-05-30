#include "thevent.h"
#include "intrman.h"
#include "kerr.h"
#include "thcommon.h"
#include "list.h"

static void event_get_status(struct event_flag *event, iop_event_info_t *info);

int CreateEventFlag(iop_event_t *event_params)
{
    struct event_flag *event;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if ((event_params->attr & ~EA_MULTI) != 0) {
        return KE_ILLEGAL_ATTR;
    }

    CpuSuspendIntr(&state);

    event = heap_alloc(TAG_EVF, sizeof(*event));
    if (!event) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    event->tag.id       = ++thctx.evflag_id;
    event->event.attr   = event_params->attr;
    event->event.option = event_params->option;
    event->init_bits    = event_params->bits;
    event->bits         = event_params->bits;

    list_init(&event->event.waiters);

    list_insert(&thctx.event_flag, &event->evf_list);

    CpuResumeIntr(state);

    return MAKE_HANDLE(event);
}

int DeleteEventFlag(int ef)
{
    struct event_flag *event;
    struct thread *waiter;
    u32 waiter_count;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    event = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_EVFID;
    }

    list_for_each_safe (waiter, &event->event.waiters, queue) {
        waiter->saved_regs->v0 = KE_WAIT_DELETE;
        list_remove(&waiter->queue);
        waiter->status = THS_READY;
        readyq_insert_back(waiter);
    }

    waiter_count = event->event.waiter_count;
    list_remove(&event->evf_list);
    heap_free(&event->tag);

    if (waiter_count) {
        thctx.run_next = NULL;
        return thread_leave(KE_OK, 0, state, 0);
    }

    CpuResumeIntr(state);

    return KE_OK;
}

int SetEventFlag(int ef, u32 bits)
{
    struct event_flag *evt;
    struct thread *thread = NULL;
    int state, num_wakeups;
    u32 shared_bits, *resbits;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    evt = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_EVFID;
    }

    if (bits == 0) {
        CpuResumeIntr(state);
        return KE_OK;
    }

    evt->bits |= bits;
    num_wakeups = 0;

    list_for_each_safe (thread, &evt->event.waiters, queue) {
        if (!evt->bits) {
            break;
        }

        shared_bits = thread->event_bits & evt->bits;
        if ((thread->event_mode & WEF_OR) == 0) {
            shared_bits = shared_bits == thread->event_bits;
        }

        if (shared_bits) {
            // We stored the WaitEventFlag resbits arg
            // here when waiting.
            resbits = (u32 *)thread->saved_regs->v0;

            if (resbits) {
                *resbits = evt->bits;
            }

            if (thread->event_mode & WEF_CLEAR) {
                evt->bits = 0;
            }

            num_wakeups++;

            evt->event.waiter_count--;
            list_remove(&thread->queue);
            thread->status = THS_READY;
            readyq_insert_back(thread);
        }
    }

    if (num_wakeups == 1) {
        readyq_remove(thread, thread->priority);
        return thread_start(thread, state);
    }

    if (num_wakeups > 0) {
        thctx.run_next = 0;
        return thread_leave(KE_OK, 0, state, 0);
    }

    CpuResumeIntr(state);

    return KE_OK;
}

int iSetEventFlag(int ef, u32 bits)
{
    struct event_flag *evt;
    struct thread *thread;
    u32 shared_bits, *resbits;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    evt = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        return KE_UNKNOWN_EVFID;
    }

    if (bits == 0) {
        return KE_OK;
    }

    evt->bits |= bits;

    list_for_each_safe (thread, &evt->event.waiters, queue) {
        if (!evt->bits) {
            break;
        }

        shared_bits = thread->event_bits & evt->bits;
        if ((thread->event_mode & WEF_OR) == 0) {
            shared_bits = shared_bits == thread->event_bits;
        }

        if (shared_bits) {
            // We stored the WaitEventFlag resbits arg
            // here when waiting.
            resbits = (u32 *)thread->saved_regs->v0;

            if (resbits) {
                *resbits = evt->bits;
            }

            if (thread->event_mode & WEF_CLEAR) {
                evt->bits = 0;
            }

            evt->event.waiter_count--;
            list_remove(&thread->queue);
            thread->status = THS_READY;
            readyq_insert_back(thread);
            thctx.run_next = NULL;
        }
    }


    return KE_OK;
}

int ClearEventFlag(int ef, u32 bits)
{
    struct event_flag *evt;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    evt = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_EVFID;
    }

    evt->bits &= bits;

    CpuResumeIntr(state);

    return KE_OK;
}

int iClearEventFlag(int ef, u32 bits)
{
    struct event_flag *evt;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    evt = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        return KE_UNKNOWN_EVFID;
    }

    evt->bits &= bits;

    return KE_OK;
}

int WaitEventFlag(int ef, u32 bits, int mode, u32 *resbits)
{
    struct event_flag *evt;
    struct thread *thread;
    u32 shared_bits;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (mode & ~(WEF_AND | WEF_OR | WEF_CLEAR)) {
        return KE_ILLEGAL_MODE;
    }

    if (bits == 0) {
        return KE_EVF_ILPAT;
    }

    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: DelayThread KE_CAN_NOT_WAIT\n");
    }

    check_thread_stack();

    evt = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_EVFID;
    }

    if ((evt->event.attr & EA_MULTI) == 0 && evt->event.waiter_count >= 0) {
        CpuResumeIntr(state);
        return KE_EVF_MULTI;
    }

    if (mode & WEF_OR) {
        shared_bits = evt->bits & bits;
    } else {
        shared_bits = (evt->bits & bits) == bits;
    }

    if (shared_bits) {
        if (resbits) {
            *resbits = evt->bits;
        }

        if (mode & WEF_CLEAR) {
            evt->bits = 0;
        }

        CpuResumeIntr(state);
        return KE_OK;
    }

    thread             = thctx.current_thread;
    thread->status     = THS_WAIT;
    thread->wait_type  = TSW_EVENTFLAG;
    thread->wait_event = &evt->event;
    thread->event_bits = bits;
    thread->event_mode = bits;
    thctx.run_next     = NULL;
    evt->event.waiter_count++;
    list_insert(&evt->event.waiters, &thread->queue);

    // put resbits in the return value
    // so we can grab it in SetEventFlag
    return thread_leave((int)resbits, 0, state, 1);
}

int PollEventFlag(int ef, u32 bits, int mode, u32 *resbits)
{
    struct event_flag *evt;
    u32 shared_bits;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (mode & ~(WEF_AND | WEF_OR | WEF_CLEAR)) {
        return KE_ILLEGAL_MODE;
    }

    if (bits == 0) {
        return KE_EVF_ILPAT;
    }

    CpuSuspendIntr(&state);

    evt = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_EVFID;
    }

    if ((evt->event.attr & EA_MULTI) == 0 && evt->event.waiter_count >= 0) {
        CpuResumeIntr(state);
        return KE_EVF_MULTI;
    }

    if (mode & WEF_OR) {
        shared_bits = evt->bits & bits;
    } else {
        shared_bits = (evt->bits & bits) == bits;
    }

    if (shared_bits == 0) {
        CpuResumeIntr(state);
        return KE_EVF_COND;
    }

    if (resbits) {
        *resbits = evt->bits;
    }

    if (mode & WEF_CLEAR) {
        evt->bits = 0;
    }

    CpuResumeIntr(state);

    return KE_OK;
}

int ReferEventFlagStatus(int ef, iop_event_info_t *info)
{
    struct event_flag *event;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    event = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_EVFID;
    }

    event_get_status(event, info);

    CpuResumeIntr(state);

    return KE_OK;
}

int iReferEventFlagStatus(int ef, iop_event_info_t *info)
{
    struct event_flag *event;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    event = HANDLE_PTR(ef);
    if (!HANDLE_VERIFY(ef, TAG_EVF)) {
        return KE_UNKNOWN_EVFID;
    }

    event_get_status(event, info);

    return KE_OK;
}

static void event_get_status(struct event_flag *event, iop_event_info_t *info)
{
    info->attr       = event->event.attr;
    info->currBits   = event->bits;
    info->initBits   = event->init_bits;
    info->numThreads = event->event.waiter_count;
    info->option     = event->event.option;
}
