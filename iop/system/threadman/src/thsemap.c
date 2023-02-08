#include "thcommon.h"
#include "thsemap.h"
#include "kerr.h"
#include "intrman.h"

static void sema_get_status(struct semaphore *sema, iop_sema_info_t *info);

int CreateSema(iop_sema_t *sema_params)
{
    struct semaphore *sema;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (sema_params->attr & ~(SA_THFIFO | SA_THPRI | SA_IHTHPRI)) {
        return KE_ILLEGAL_ATTR;
    }

    CpuSuspendIntr(&state);

    sema = heap_alloc(TAG_SEMA, sizeof(*sema));
    if (!sema) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    sema->base.tag.id   = ++thctx.sema_id;
    sema->base.attr     = sema_params->attr;
    sema->base.option   = sema_params->option;
    sema->count         = sema_params->initial;
    sema->initial_count = sema_params->initial;
    sema->max_count     = sema_params->max;

    list_init(&sema->base.waiters);

    list_insert(&thctx.semaphore, &sema->base.tag.list);

    CpuResumeIntr(state);

    return MAKE_HANDLE(sema);
}

int DeleteSema(int semid)
{
    struct semaphore *sema;
    struct thread *waiter;
    u32 waiter_count;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_SEMID;
    }

    list_for_each_safe (waiter, &sema->base.waiters, tag.list) {
        waiter->saved_regs->v0 = KE_WAIT_DELETE;
        waiter->status         = THS_READY;
        list_remove(&waiter->tag.list);
        readyq_insert_back(waiter);
    }

    waiter_count = sema->base.waiter_count;
    list_remove(&sema->base.tag.list);
    heap_free(&sema->base.tag);

    if (waiter_count) {
        thctx.run_next = NULL;
        return thread_leave(KE_OK, 0, state, 0);
    }

    CpuResumeIntr(state);

    return KE_OK;
}

int SignalSema(int semid)
{
    struct semaphore *sema;
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_SEMID;
    }

    if (sema->base.waiter_count > 0) {
        thread = list_first_entry(&sema->base.waiters, struct thread, tag.list);
        sema->base.waiter_count--;
        list_remove(&thread->tag.list);
        thread->status = THS_READY;

        return thread_start(thread, state);
    }

    if (sema->count >= sema->max_count) {
        CpuResumeIntr(state);
        return KE_SEMA_OVF;
    }

    sema->count++;

    CpuResumeIntr(state);

    return KE_OK;
}

int iSignalSema(int semid)
{
    struct semaphore *sema;
    struct thread *thread;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        return KE_UNKNOWN_SEMID;
    }

    if (sema->base.waiter_count > 0) {
        thread = list_first_entry(&sema->base.waiters, struct thread, tag.list);
        sema->base.waiter_count--;
        list_remove(&thread->tag.list);
        thread->saved_regs->v0 = KE_OK;
        thread->status         = THS_READY;
        readyq_insert_back(thread);
        thctx.run_next = 0;

        return KE_OK;
    }

    if (sema->count >= sema->max_count) {
        return KE_SEMA_OVF;
    }

    sema->count++;

    return KE_OK;
}

int WaitSema(int semid)
{
    struct semaphore *sema;
    struct thread *thread;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: DelayThread KE_CAN_NOT_WAIT\n");
    }

    check_thread_stack();

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_SEMID;
    }

    thread = thctx.current_thread;

    if (sema->count >= 1) {
        sema->count--;
        CpuResumeIntr(state);
        return KE_OK;
    }

    thread->status     = THS_WAIT;
    thread->wait_type  = TSW_SEMA;
    thread->wait_event = &sema->base;
    thctx.run_next     = NULL;
    sema->base.waiter_count++;

    if (sema->base.attr & SA_THPRI) {
        // originally just a loop (or inlined)
        // i don't see why not to use this function though
        waitlist_insert(thread, &sema->base, thread->priority);
    } else {
        list_insert(&sema->base.waiters, &thread->tag.list);
    }

    return thread_leave(KE_OK, 0, state, 1);
}

int PollSema(int semid)
{
    struct semaphore *sema;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_SEMID;
    }

    if (sema->count == 0) {
        CpuResumeIntr(state);
        return KE_SEMA_ZERO;
    }

    sema->count--;

    CpuResumeIntr(state);
    return KE_OK;
}

int ReferSemaStatus(int semid, iop_sema_info_t *info)
{
    struct semaphore *sema;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_SEMID;
    }

    sema_get_status(sema, info);

    CpuResumeIntr(state);

    return KE_OK;
}

int iReferSemaStatus(int semid, iop_sema_info_t *info)
{
    struct semaphore *sema;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    sema = HANDLE_PTR(semid);
    if (!HANDLE_VERIFY(semid, TAG_SEMA)) {
        return KE_UNKNOWN_SEMID;
    }

    sema_get_status(sema, info);

    return KE_OK;
}

static void sema_get_status(struct semaphore *sema, iop_sema_info_t *info)
{
    info->attr           = sema->base.attr;
    info->current        = sema->count;
    info->max            = sema->max_count;
    info->initial        = sema->initial_count;
    info->numWaitThreads = sema->base.waiter_count;
    info->option         = sema->base.option;
}
