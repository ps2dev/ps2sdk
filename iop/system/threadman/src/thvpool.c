#include "intrman.h"
#include "kerr.h"
#include "thcommon.h"
#include "thpool.h"
#include "heaplib.h"

static void vpl_get_info(struct vpool *vpl, iop_vpl_info_t *info);

int CreateVpl(iop_vpl_param *param)
{
    struct vpool *vpl;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (param->attr & ~(VA_THFIFO | VA_THPRI | VA_MEMBTM)) {
        return KE_ILLEGAL_ATTR;
    }

    if (param->size == 0) {
        return KE_ILLEGAL_MEMSIZE;
    }

    CpuSuspendIntr(&state);

    vpl = heap_alloc(TAG_VPL, sizeof(*vpl));
    if (!vpl) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    vpl->heap         = CreateHeap(param->size, (param->attr & VA_MEMBTM) << 1);
    vpl->free_size    = HeapTotalFreeSize(vpl->heap);
    vpl->event.attr   = param->attr;
    vpl->event.option = param->option;

    list_init(&vpl->event.waiters);
    list_insert(&thctx.vpool, &vpl->vpl_list);

    CpuResumeIntr(state);

    return MAKE_HANDLE(vpl);
}

int DeleteVpl(int vplId)
{
    struct thread *waiter;
    struct vpool *vpl;
    u32 waiter_count;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_VPLID;
    }

    list_for_each_safe (waiter, &vpl->event.waiters, queue) {
        waiter->saved_regs->v0 = KE_WAIT_DELETE;
        waiter->status         = THS_READY;
        list_remove(&waiter->queue);
        readyq_insert_back(waiter);
    }

    waiter_count = vpl->event.waiter_count;
    DeleteHeap(vpl->heap);
    list_remove(&vpl->vpl_list);
    heap_free(&vpl->tag);

    if (waiter_count) {
        thctx.run_next = NULL;
        return thread_leave(KE_OK, 0, state, 0);
    }

    CpuResumeIntr(state);

    return KE_OK;
}

void *AllocateVpl(int vplId, int size)
{
    struct thread *thread;
    struct vpool *vpl;
    void *mem;
    int state;

    if (QueryIntrContext()) {
        return (void *)KE_ILLEGAL_CONTEXT;
    }

    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: AllocateVpl KE_CAN_NOT_WAIT\n");
    }

    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        CpuResumeIntr(state);
        return (void *)KE_UNKNOWN_VPLID;
    }

    if (!size || vpl->free_size < size) {
        CpuResumeIntr(state);
        return (void *)KE_ILLEGAL_MEMSIZE;
    }

    mem = AllocHeapMemory(vpl->heap, size);
    if (mem) {
        CpuResumeIntr(state);
        return mem;
    }

    thread             = thctx.current_thread;
    thread->status     = THS_WAIT;
    thread->wait_type  = TSW_FPL;
    thread->wait_event = &vpl->event;

    thctx.run_next = NULL;
    vpl->event.waiter_count++;

    if (vpl->event.attr & VA_THPRI) {
        waitlist_insert(thread, &vpl->event, thread->priority);
    } else {
        list_insert(&vpl->event.waiters, &thread->queue);
    }

    return (void *)thread_leave(size, 0, state, 1);
}

void *pAllocateVpl(int vplId, int size)
{
    struct vpool *vpl;
    void *mem;
    int state;

    if (QueryIntrContext()) {
        return (void *)KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        CpuResumeIntr(state);
        return (void *)KE_UNKNOWN_VPLID;
    }

    if (!size || vpl->free_size < size) {
        CpuResumeIntr(state);
        return (void *)KE_ILLEGAL_MEMSIZE;
    }

    mem = AllocHeapMemory(vpl->heap, size);
    CpuResumeIntr(state);

    if (!mem) {
        return (void *)KE_NO_MEMORY;
    }

    return mem;
}

void *ipAllocateVpl(int vplId, int size)
{
    struct vpool *vpl;
    void *mem;

    if (!QueryIntrContext()) {
        return (void *)KE_ILLEGAL_CONTEXT;
    }

    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        return (void *)KE_UNKNOWN_VPLID;
    }

    mem = AllocHeapMemory(vpl->heap, size);
    if (!mem) {
        return (void *)KE_NO_MEMORY;
    }

    return mem;
}

int FreeVpl(int vplId, void *memory)
{
    struct thread *thread;
    struct vpool *vpl;
    void *new_mem;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        CpuResumeIntr(state);
        // BUG: originally wrong value
        // return KE_UNKNOWN_FPLID;
        return KE_UNKNOWN_VPLID;
    }

    if (FreeHeapMemory(vpl->heap, memory) < 0) {
        CpuResumeIntr(state);
        return KE_ERROR;
    }

    if (vpl->event.waiter_count) {
        thread  = list_first_entry(&vpl->event.waiters, struct thread, queue);
        new_mem = AllocHeapMemory(vpl->heap, thread->saved_regs->v0);
        if (new_mem) {
            vpl->event.waiter_count--;
            list_remove(&thread->queue);
            thread->status         = THS_READY;
            thread->saved_regs->v0 = (u32)new_mem;

            return thread_start(thread, state);
        }
    }

    CpuResumeIntr(state);

    return KE_OK;
}

int ReferVplStatus(int vplId, iop_vpl_info_t *info)
{
    struct vpool *vpl;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_VPLID;
    }

    vpl_get_info(vpl, info);

    CpuResumeIntr(state);

    return KE_OK;
}

int iReferVplStatus(int vplId, iop_vpl_info_t *info)
{
    struct vpool *vpl;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    vpl = HANDLE_PTR(vplId);
    if (!HANDLE_VERIFY(vplId, TAG_VPL)) {
        return KE_UNKNOWN_VPLID;
    }

    vpl_get_info(vpl, info);

    return KE_OK;
}

static void vpl_get_info(struct vpool *vpl, iop_vpl_info_t *info)
{
    info->attr           = vpl->event.attr;
    info->option         = vpl->event.option;
    info->size           = vpl->free_size;
    info->freeSize       = HeapTotalFreeSize(vpl->heap);
    info->numWaitThreads = vpl->event.waiter_count;
}
