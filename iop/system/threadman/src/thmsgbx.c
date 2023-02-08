#include "intrman.h"
#include "kerr.h"
#include "thcommon.h"
#include "thmsgbx.h"

static void mbx_send(struct mbox *mbx, iop_message_t *msg);
static void mbx_get_status(struct mbox *mbx, iop_mbx_status_t *info);

int CreateMbx(iop_mbx_t *mbx_param)
{
    struct mbox *mbx;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (mbx_param->attr & ~(MBA_MSFIFO | MBA_THPRI | MBA_MSFIFO | MBA_MSPRI)) {
        return KE_ILLEGAL_ATTR;
    }

    CpuSuspendIntr(&state);

    mbx = heap_alloc(TAG_MBX, sizeof(*mbx));
    if (!mbx) {
        CpuResumeIntr(state);
        return KE_NO_MEMORY;
    }

    mbx->base.tag.id = ++thctx.mbox_id;
    mbx->base.attr   = mbx_param->attr;
    mbx->base.option = mbx_param->option;
    list_init(&mbx->base.waiters);
    list_insert(&thctx.mbox, &mbx->base.tag.list);

    CpuResumeIntr(state);

    return MAKE_HANDLE(mbx);
}

int DeleteMbx(int mbxid)
{
    struct thread *waiter;
    struct mbox *mbx;
    u32 waiter_count;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_MBXID;
    }

    list_for_each_safe (waiter, &mbx->base.waiters, tag.list) {
        waiter->saved_regs->v0 = KE_WAIT_DELETE;
        list_remove(&waiter->tag.list);
        waiter->status = THS_READY;
        readyq_insert_back(waiter);
    }

    waiter_count = mbx->base.waiter_count;
    list_remove(&mbx->base.tag.list);
    heap_free(&mbx->base.tag);

    if (waiter_count) {
        thctx.run_next = NULL;
        return thread_leave(KE_OK, 0, state, 0);
    }

    CpuResumeIntr(state);

    return KE_OK;
}

int SendMbx(int mbxid, void *msg)
{
    struct thread *thread;
    struct mbox *mbx;
    void **msg_dest;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_MBXID;
    }

    if (mbx->base.waiter_count == 0) {
        mbx_send(mbx, msg);
        CpuResumeIntr(state);
        return KE_OK;
    }

    thread = (struct thread *)mbx->base.waiters.next;
    mbx->base.waiter_count--;
    list_remove(&thread->tag.list);

    msg_dest = (void **)thread->saved_regs->v0;

    thread->saved_regs->v0 = KE_OK;

    if (msg_dest) {
        *msg_dest = msg;
    }

    return thread_start(thread, state);
}

int iSendMbx(int mbxid, void *msg)
{
    struct thread *thread;
    struct mbox *mbx;
    void **msg_dest;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        return KE_UNKNOWN_MBXID;
    }

    if (mbx->base.waiter_count == 0) {
        mbx_send(mbx, msg);
        return KE_OK;
    }

    thread = list_first_entry(&mbx->base.waiters, struct thread, tag.list);
    mbx->base.waiter_count--;
    list_remove(&thread->tag.list);
    msg_dest               = (void **)thread->saved_regs->v0;
    thread->saved_regs->v0 = KE_OK;

    if (msg_dest) {
        *msg_dest = msg;
    }

    readyq_insert_back(thread);
    thctx.run_next = NULL;

    return KE_OK;
}

int ReceiveMbx(void **msgvar, int mbxid)
{
    struct thread *thread;
    iop_message_t *msg;
    struct mbox *mbx;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    if (CpuSuspendIntr(&state) == KE_CPUDI && (thctx.debug_flags & 8)) {
        Kprintf("WARNING: ReceiveMbx:KE_CAN_NOT_WAIT\n");
    }

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_MBXID;
    }

    if (mbx->msg_count == 0) {
        thread             = thctx.current_thread;
        thread->status     = THS_WAIT;
        thread->wait_type  = TSW_MBX;
        thread->wait_event = &mbx->base;
        thctx.run_next     = NULL;

        if (mbx->base.attr & MBA_THPRI) {
            waitlist_insert(thread, &mbx->base, thread->priority);
        } else {
            list_insert(&mbx->base.waiters, &thread->tag.list);
        }

        return thread_leave((int)msgvar, 0, state, 1);
    }

    mbx->msg_count--;
    msg = mbx->newest_msg->next;
    if (msg == msg->next) {
        mbx->newest_msg = NULL;
    } else {
        mbx->newest_msg->next = msg->next;
    }

    *msgvar = msg;

    CpuResumeIntr(state);

    return KE_OK;
}

int PollMbx(void **msgvar, int mbxid)
{
    iop_message_t *msg;
    struct mbox *mbx;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_MBXID;
    }

    if (mbx->msg_count == 0) {
        CpuResumeIntr(state);
        return KE_MBOX_NOMSG;
    }

    mbx->msg_count--;
    msg = mbx->newest_msg->next;
    if (msg == msg->next) {
        mbx->newest_msg = NULL;
    } else {
        mbx->newest_msg->next = msg->next;
    }

    *msgvar = msg;

    CpuResumeIntr(state);

    return KE_OK;
}

int ReferMbxStatus(int mbxid, iop_mbx_status_t *info)
{
    struct mbox *mbx;
    int state;

    if (QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    CpuSuspendIntr(&state);

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        CpuResumeIntr(state);
        return KE_UNKNOWN_MBXID;
    }

    mbx_get_status(mbx, info);

    CpuResumeIntr(state);

    return KE_OK;
}

int iReferMbxStatus(int mbxid, iop_mbx_status_t *info)
{
    struct mbox *mbx;

    if (!QueryIntrContext()) {
        return KE_ILLEGAL_CONTEXT;
    }

    mbx = HANDLE_PTR(mbxid);
    if (!HANDLE_VERIFY(mbxid, TAG_MBX)) {
        return KE_UNKNOWN_MBXID;
    }

    mbx_get_status(mbx, info);

    return KE_OK;
}

static void mbx_send(struct mbox *mbx, iop_message_t *new_msg)
{
    iop_message_t *latest, *oldest;

    latest = mbx->newest_msg;
    oldest = latest->next;

    mbx->msg_count++;

    if (!mbx->newest_msg) {
        mbx->newest_msg = new_msg;
        new_msg->next   = new_msg;
        return;
    }

    if ((mbx->base.attr & MBA_MSPRI) == 0) {
        new_msg->next   = oldest;
        latest->next    = new_msg;
        mbx->newest_msg = new_msg;
    } else {
        // FIXME this is mostly copied out of ghidra because its awful
        iop_message_t *piVar1;
        iop_message_t *piVar2;
        u32 prio;

        prio   = latest->next->priority;
        piVar2 = latest;
        piVar1 = latest->next;

        while (1) {
            if (new_msg->priority < prio) {
                new_msg->next = piVar2->next;
                piVar2->next  = new_msg;
                return;
            }

            piVar2 = piVar1;

            if (piVar1 == latest) {
                mbx->newest_msg = new_msg;
                new_msg->next   = piVar2->next;
                piVar2->next    = new_msg;
                return;
            }

            prio   = piVar1->next->priority;
            piVar1 = piVar1->next;
        }
    }
}

static void mbx_get_status(struct mbox *mbx, iop_mbx_status_t *info)
{
    info->attr           = mbx->base.attr;
    info->option         = mbx->base.option;
    info->numWaitThreads = mbx->base.waiter_count;
    info->numMessage     = mbx->msg_count;
    info->topPacket      = mbx->newest_msg->next;
}
