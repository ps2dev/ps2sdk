#include "intrman.h"
#include "kerr.h"
#include "thcommon.h"
#include "thpool.h"

static void fpl_block_free(struct fpool *fpl, struct fpl_block *block);
static struct fpl_block *fpl_block_alloc(struct fpool *fpl);
static void fpl_get_info(struct fpool *fpl, iop_fpl_info_t *info);

int CreateFpl(iop_fpl_param *param)
{
    struct fpool *fpl;
    u32 block_size, mem_size;
    int state;
    void *mem;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (param->attr & ~(FA_THFIFO | FA_THPRI | FA_MEMBTM)) {
        return KE_ILLEGAL_ATTR;
    }

    if (param->block_size == 0 || param->blocks == 0) {
        return KE_ILLEGAL_MEMSIZE;
    }

    CpuSuspendIntr(&state);

    block_size = ALIGN(param->block_size);
    mem_size   = block_size * param->blocks;

    mem = AllocSysMemory((param->attr & FA_MEMBTM) >> 9, mem_size, NULL);
    if (!mem) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    fpl = heap_alloc(TAG_FPL, sizeof(*fpl));
    if (!fpl) {
        FreeSysMemory(mem);
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    fpl->memory       = mem;
    fpl->block_size   = param->block_size;
    fpl->blocks       = param->blocks;
    fpl->mem_size     = mem_size;
    fpl->event.attr   = param->attr;
    fpl->event.option = param->option;

    list_init(&fpl->event.waiters);
    list_insert(&thctx.fpool, &fpl->fpl_list);

    fpl->free = NULL;
    for (int i = 0; i < param->blocks; i++, mem += block_size) {
        fpl_block_free(fpl, mem);
    }

    CpuResumeIntr(state);

    return MAKE_HANDLE(fpl);
}

int DeleteFpl(int fplId)
{
    struct thread *waiter;
    struct fpool *fpl;
    u32 waiter_count;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_FPLID;
    }

    list_for_each_safe (waiter, &fpl->event.waiters, queue) {
        waiter->saved_regs->v0 = KE_WAIT_DELETE;
        waiter->status         = THS_READY;
        list_remove(&waiter->queue);
        readyq_insert_back(waiter);
    }

    waiter_count = fpl->event.waiter_count;
    FreeSysMemory(fpl->memory);
    list_remove(&fpl->fpl_list);
    heap_free(&fpl->tag);

    if (waiter_count) {
        thctx.run_next = NULL;
        return thread_leave(KE_OK, 0, state, 0);
    }

    CpuResumeIntr(state);

    return KE_OK;
}

void *AllocateFpl(int fplId)
{
    struct thread *thread;
    struct fpool *fpl;
    void *block;
    int state;

    if (QueryIntrContext()) {
        return (void *)KE_ILLEGAL_CONTEXT;
    }

    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: AllocateFpl KE_CAN_NOT_WAIT\n");
    }

    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        CpuResumeIntr(state);
        return (void *)KE_UNKNOWN_FPLID;
    }

    if (fpl->free) {
        block = fpl_block_alloc(fpl);
        CpuResumeIntr(state);
        return block;
    }

    thread = thctx.current_thread;

    thread->status     = THS_WAIT;
    thread->wait_type  = TSW_FPL;
    thread->wait_event = &fpl->event;
    thctx.run_next     = NULL;

    fpl->event.waiter_count++;

    if (fpl->event.attr & FA_THPRI) {
        waitlist_insert(thread, &fpl->event, thread->priority);
    } else {
        list_insert(&fpl->event.waiters, &thread->queue);
    }

    return (void *)thread_leave(KE_OK, 0, state, 1);
}

void *pAllocateFpl(int fplId)
{
    struct fpool *fpl;
    void *block;
    int state;

    if (QueryIntrContext()) {
        return (void *)KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        CpuResumeIntr(state);
        return (void *)KE_UNKNOWN_FPLID;
    }

    if (!fpl->free) {
        CpuResumeIntr(state);
        return (void *)KE_NO_MEMORY;
    }

    block = fpl_block_alloc(fpl);
    CpuResumeIntr(state);

    return block;
}

void *ipAllocateFpl(int fplId)
{
    struct fpool *fpl;

    if (!QueryIntrContext()) {
        return (void *)KE_ILLEGAL_CONTEXT;
    }

    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        return (void *)KE_UNKNOWN_FPLID;
    }

    if (!fpl->free) {
        return (void *)KE_NO_MEMORY;
    }

    return fpl_block_alloc(fpl);
}

int FreeFpl(int fplId, void *memory)
{
    struct thread *waiter;
    struct fpool *fpl;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_FPLID;
    }

    if (memory < fpl->memory || memory >= (fpl->memory + fpl->mem_size)) {
        return KE_ILLEGAL_MEMBLOCK;
    }

    if (fpl->event.waiter_count) {
        waiter = list_first_entry(&fpl->event.waiters, struct thread, queue);
        fpl->event.waiter_count--;
        list_remove(&waiter->queue);
        waiter->status         = THS_READY;
        waiter->saved_regs->v0 = (u32)memory;

        return thread_start(waiter, state);
    }

    fpl_block_free(fpl, memory);

    return KE_OK;
}

int ReferFplStatus(int fplId, iop_fpl_info_t *info)
{
    struct fpool *fpl;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);
    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_FPLID;
    }

    fpl_get_info(fpl, info);

    CpuResumeIntr(state);

    return KE_OK;
}

int iReferFplStatus(int fplId, iop_fpl_info_t *info)
{
    struct fpool *fpl;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    fpl = HANDLE_PTR(fplId);
    if (!HANDLE_VERIFY(fplId, TAG_FPL)) {
        return KE_UNKNOWN_FPLID;
    }

    fpl_get_info(fpl, info);

    return KE_OK;
}

static struct fpl_block *fpl_block_alloc(struct fpool *fpl)
{
    struct fpl_block *tail, *head;

    if (!fpl->free) {
        return NULL;
    }

    fpl->free_blocks--;

    tail = fpl->free;
    head = tail->next;

    if (tail == head) {
        fpl->free = NULL;
    } else {
        tail->next = head->next;
    }

    return head;
}

static void fpl_block_free(struct fpool *fpl, struct fpl_block *block)
{
    struct fpl_block *tail;
    tail = fpl->free;

    fpl->free_blocks++;
    if (tail) {
        block->next = tail->next;
        tail->next  = block;
        fpl->free   = block;
    } else {
        fpl->free   = block;
        block->next = block;
    }
}

static void fpl_get_info(struct fpool *fpl, iop_fpl_info_t *info)
{
    info->attr           = fpl->event.attr;
    info->option         = fpl->event.option;
    info->blockSize      = fpl->block_size;
    info->numBlocks      = fpl->blocks;
    info->freeBlocks     = fpl->free_blocks;
    info->numWaitThreads = fpl->event.waiter_count;
}
