/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE SIF RPC commands
 * MRB: This file now contains the SIF routines included
 * with libpsware.  Bug reports welcome.
 */

#include <tamtypes.h>
#include <ps2lib_err.h>
#include <kernel.h>
#include <sifcmd.h>
#include <sifrpc.h>

#define RPC_PACKET_SIZE 64

/** Set if the packet has been allocated */
#define PACKET_F_ALLOC 0x01

static inline void rpc_packet_free(void *packet)
{
    SifRpcRendPkt_t *rendpkt = (SifRpcRendPkt_t *)packet;

    rendpkt->rpc_id = 0;
    rendpkt->rec_id &= (~PACKET_F_ALLOC);
}

struct rpc_data
{
    int pid;
    void *pkt_table;
    int pkt_table_len;
    int unused1;
    int unused2;
    u8 *rdata_table;
    int rdata_table_len;
    u8 *client_table;
    int client_table_len;
    int rdata_table_idx;
    void *active_queue;
} __attribute__((aligned(64)));

extern int _iop_reboot_count;
extern struct rpc_data _sif_rpc_data;

void *_rpc_get_packet(struct rpc_data *rpc_data);
void *_rpc_get_fpacket(struct rpc_data *rpc_data);

#ifdef F__rpc_get_packet
void *_rpc_get_packet(struct rpc_data *rpc_data)
{
    SifRpcPktHeader_t *packet;
    int len;

    DI();

    len = rpc_data->pkt_table_len;
    if (len > 0) {
        int pid, rid;
        packet = (SifRpcPktHeader_t *)rpc_data->pkt_table;

        for (rid = 0; rid < len; rid++, packet = (SifRpcPktHeader_t *)(((unsigned char *)packet) + RPC_PACKET_SIZE)) {
            if (!(packet->rec_id & PACKET_F_ALLOC))
                break;
        }
        if (rid == len) {
            EI();
            return NULL;
        }

        pid = rpc_data->pid;
        if (pid) {
            rpc_data->pid = ++pid;
        } else {
            rpc_data->pid = pid + 2;
            pid           = 1;
        }

        packet->rec_id   = (rid << 16) | 0x04 | PACKET_F_ALLOC;
        packet->rpc_id   = pid;
        packet->pkt_addr = packet;
        EI();
        return packet;
    }
    EI();
    return NULL;
}
#endif

#ifdef F__rpc_get_fpacket
void *_rpc_get_fpacket(struct rpc_data *rpc_data)
{
    int index;

    index                     = rpc_data->rdata_table_idx % rpc_data->rdata_table_len;
    rpc_data->rdata_table_idx = index + 1;

    return (void *)(rpc_data->rdata_table + (index * RPC_PACKET_SIZE));
}
#endif

#ifdef F_sceSifBindRpc
int sceSifBindRpc(SifRpcClientData_t *cd, int sid, int mode)
{
    ee_sema_t sema;
    SifRpcBindPkt_t *bind;

    bind = (SifRpcBindPkt_t *)_rpc_get_packet(&_sif_rpc_data);
    if (!bind)
        return -E_SIF_PKT_ALLOC;

    cd->command      = 0;
    cd->server       = NULL;
    cd->hdr.pkt_addr = bind;
    cd->hdr.rpc_id   = bind->rpc_id;
    cd->hdr.sema_id  = -1;

    bind->sid      = sid;
    bind->pkt_addr = bind;
    bind->cd       = cd;

    if (mode & SIF_RPC_M_NOWAIT) {
        if (!sceSifSendCmd(SIF_CMD_RPC_BIND, bind, RPC_PACKET_SIZE, NULL, NULL, 0)) {
            rpc_packet_free(bind);
            return -E_SIF_PKT_SEND;
        }

        return 0;
    }

    sema.max_count  = 1;
    sema.init_count = 0;
    cd->hdr.sema_id = CreateSema(&sema);
    if (cd->hdr.sema_id < 0) {
        rpc_packet_free(bind);
        return -E_LIB_SEMA_CREATE;
    }

    if (!sceSifSendCmd(SIF_CMD_RPC_BIND, bind, RPC_PACKET_SIZE, NULL, NULL, 0)) {
        rpc_packet_free(bind);
        DeleteSema(cd->hdr.sema_id);
        return -E_SIF_PKT_SEND;
    }

    WaitSema(cd->hdr.sema_id);
    DeleteSema(cd->hdr.sema_id);

    return 0;
}
#endif

#ifdef F_sceSifCallRpc
int sceSifCallRpc(SifRpcClientData_t *cd, int rpc_number, int mode, void *sendbuf,
    int ssize, void *recvbuf, int rsize, SifRpcEndFunc_t end_function, void *end_param)
{
    ee_sema_t sema;
    SifRpcCallPkt_t *call;

    call = (SifRpcCallPkt_t *)_rpc_get_packet(&_sif_rpc_data);
    if (!call)
        return -E_SIF_PKT_ALLOC;

    cd->hdr.pkt_addr = call;
    cd->hdr.rpc_id   = call->rpc_id;
    cd->hdr.sema_id  = -1;
    cd->end_function = end_function;
    cd->end_param    = end_param;

    call->rpc_number = rpc_number;
    call->send_size  = ssize;
    call->recvbuf    = recvbuf;
    call->recv_size  = rsize;
    call->rmode      = 1;
    call->pkt_addr   = call;
    call->cd         = cd;
    call->sd         = cd->server;

    if (!(mode & SIF_RPC_M_NOWBDC)) {
        if (ssize > 0)
            sceSifWriteBackDCache(sendbuf, ssize);
        if (rsize > 0)
            sceSifWriteBackDCache(recvbuf, rsize);
    }

    if (mode & SIF_RPC_M_NOWAIT) {
        if (!end_function)
            call->rmode = 0;

        if (!sceSifSendCmd(SIF_CMD_RPC_CALL, call, RPC_PACKET_SIZE, sendbuf, cd->buf, ssize)) {
            rpc_packet_free(call);
            return -E_SIF_PKT_SEND;
        }

        return 0;
    }

    sema.max_count  = 1;
    sema.init_count = 0;
    cd->hdr.sema_id = CreateSema(&sema);
    if (cd->hdr.sema_id < 0) {
        rpc_packet_free(call);
        return -E_LIB_SEMA_CREATE;
    }

    if (!sceSifSendCmd(SIF_CMD_RPC_CALL, call, RPC_PACKET_SIZE, sendbuf, cd->buf, ssize)) {
        rpc_packet_free(call);
        DeleteSema(cd->hdr.sema_id);
        return -E_SIF_PKT_SEND;
    }

    WaitSema(cd->hdr.sema_id);
    DeleteSema(cd->hdr.sema_id);

    return 0;
}
#endif

#ifdef F_sceSifGetOtherData
int sceSifGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest, int size, int mode)
{
    ee_sema_t sema;
    SifRpcOtherDataPkt_t *other;

    other = (SifRpcOtherDataPkt_t *)_rpc_get_packet(&_sif_rpc_data);
    if (!other)
        return -E_SIF_PKT_ALLOC;

    rd->hdr.pkt_addr = other;
    rd->hdr.rpc_id   = other->rpc_id;
    rd->hdr.sema_id  = -1;

    other->src     = src;
    other->dest    = dest;
    other->size    = size;
    other->recvbuf = rd;

    if (mode & SIF_RPC_M_NOWAIT) {
        if (!sceSifSendCmd(SIF_CMD_RPC_RDATA, other, RPC_PACKET_SIZE, NULL, NULL, 0)) {
            rpc_packet_free(other);
            return -E_SIF_PKT_SEND;
        }

        return 0;
    }

    sema.max_count  = 1;
    sema.init_count = 0;
    rd->hdr.sema_id = CreateSema(&sema);
    if (rd->hdr.sema_id < 0) {
        rpc_packet_free(other);
        return -E_LIB_SEMA_CREATE;
    }

    if (!sceSifSendCmd(SIF_CMD_RPC_RDATA, other, RPC_PACKET_SIZE, NULL, NULL, 0)) {
        rpc_packet_free(other);
        DeleteSema(rd->hdr.sema_id);
        return -E_SIF_PKT_SEND;
    }

    WaitSema(rd->hdr.sema_id);
    DeleteSema(rd->hdr.sema_id);

    return 0;
}
#endif

#ifdef F_SifRpcMain

/* The packets sent on EE RPC requests are allocated from this table.  */
static u8 pkt_table[2048] __attribute__((aligned(64)));
/* A ring buffer used to allocate packets sent on IOP requests.  */
static u8 rdata_table[2048] __attribute__((aligned(64)));
static u8 client_table[2048] __attribute__((aligned(64)));

struct rpc_data _sif_rpc_data = {
    pid : 1,
    pkt_table : pkt_table,
    pkt_table_len : sizeof(pkt_table) / RPC_PACKET_SIZE,
    rdata_table : rdata_table,
    rdata_table_len : sizeof(rdata_table) / RPC_PACKET_SIZE,
    client_table : client_table,
    client_table_len : sizeof(client_table) / RPC_PACKET_SIZE,
    rdata_table_idx : 0
};

static int init = 0;


/* Command 0x80000008 */
static void _request_end(SifRpcRendPkt_t *request, void *data)
{
    SifRpcClientData_t *cd = request->cd;

    (void)data;

    if (request->cid == SIF_CMD_RPC_CALL) {
        if (cd->end_function)
            cd->end_function(cd->end_param);
    } else if (request->cid == SIF_CMD_RPC_BIND) {
        cd->server  = request->sd;
        cd->buf    = request->buf;
        cd->cbuf   = request->cbuf;
    }

    if (cd->hdr.sema_id >= 0)
        iSignalSema(cd->hdr.sema_id);

    rpc_packet_free(cd->hdr.pkt_addr);
    cd->hdr.pkt_addr = NULL;
}

static void *search_svdata(u32 sid, struct rpc_data *rpc_data)
{
    SifRpcServerData_t *sd;
    SifRpcDataQueue_t *queue = rpc_data->active_queue;

    if (!queue)
        return NULL;

    while (queue) {
        sd = queue->link;
        while (sd) {
            if ((u32)(sd->sid) == sid)
                return sd;

            sd = sd->link;
        }

        queue = queue->next;
    }

    return NULL;
}

/* Command 0x80000009 */
static void _request_bind(SifRpcBindPkt_t *bind, void *data)
{
    SifRpcRendPkt_t *rend;
    SifRpcServerData_t *sd;

    rend           = _rpc_get_fpacket(data);
    rend->pkt_addr = bind->pkt_addr;
    rend->cd       = bind->cd;
    rend->cid      = SIF_CMD_RPC_BIND;

    sd = search_svdata(bind->sid, data);
    if (!sd) {
        rend->sd = NULL;
        rend->buf    = NULL;
        rend->cbuf   = NULL;
    } else {
        rend->sd     = sd;
        rend->buf    = sd->buf;
        rend->cbuf   = sd->cbuf;
    }

    isceSifSendCmd(SIF_CMD_RPC_END, rend, RPC_PACKET_SIZE, NULL, NULL, 0);
}

/* Command 0x8000000a */
static void _request_call(SifRpcCallPkt_t *request, void *data)
{
    SifRpcServerData_t *sd     = request->sd;
    SifRpcDataQueue_t *base    = sd->base;

    (void)data;

    if (base->start)
        base->end->next = sd;
    else
        base->start = sd;

    base->end      = sd;
    sd->pkt_addr   = request->pkt_addr;
    sd->client     = request->cd;
    sd->rpc_number = request->rpc_number;
    sd->size       = request->send_size;
    sd->recvbuf    = request->recvbuf;
    sd->rsize      = request->recv_size;
    sd->rmode      = request->rmode;
    sd->rid        = request->rec_id;

    if (base->thread_id < 0 || base->active != 0)
        return;

    iWakeupThread(base->thread_id);
}

/* Command 0x8000000c */
static void _request_rdata(SifRpcOtherDataPkt_t *rdata, void *data)
{
    SifRpcRendPkt_t *rend;

    rend           = (SifRpcRendPkt_t *)_rpc_get_fpacket(data);
    rend->pkt_addr = rdata->pkt_addr;
    rend->cd       = (SifRpcClientData_t *)rdata->recvbuf;
    rend->cid      = SIF_CMD_RPC_RDATA;

    isceSifSendCmd(SIF_CMD_RPC_END, rend, RPC_PACKET_SIZE, rdata->src, rdata->dest, rdata->size);
}

void sceSifInitRpc(int mode)
{
    u32 *cmdp;

    (void)mode;

    static int _rb_count = 0;
    if (_rb_count != _iop_reboot_count) {
        _rb_count = _iop_reboot_count;
        sceSifExitCmd();
        init = 0;
    }

    if (init)
        return;
    init = 1;

    sceSifInitCmd();

    DI();
    _sif_rpc_data.pkt_table    = UNCACHED_SEG(_sif_rpc_data.pkt_table);
    _sif_rpc_data.rdata_table  = UNCACHED_SEG(_sif_rpc_data.rdata_table);
    _sif_rpc_data.client_table = UNCACHED_SEG(_sif_rpc_data.client_table);

    _sif_rpc_data.rdata_table_idx = 0;
    struct rpc_data *rpc_data     = (struct rpc_data *)(&_sif_rpc_data);

    int len = rpc_data->pkt_table_len;
    if (len > 0) {
        int rid;
        SifRpcPktHeader_t *packet = (SifRpcPktHeader_t *)rpc_data->pkt_table;

        for (rid = 0; rid < len; rid++, packet = (SifRpcPktHeader_t *)(((unsigned char *)packet) + RPC_PACKET_SIZE)) {
            rpc_packet_free(packet);
        }
    }

    sceSifAddCmdHandler(SIF_CMD_RPC_END, (void *)_request_end, &_sif_rpc_data);
    sceSifAddCmdHandler(SIF_CMD_RPC_BIND, (void *)_request_bind, &_sif_rpc_data);
    sceSifAddCmdHandler(SIF_CMD_RPC_CALL, (void *)_request_call, &_sif_rpc_data);
    sceSifAddCmdHandler(SIF_CMD_RPC_RDATA, (void *)_request_rdata, &_sif_rpc_data);
    EI();

    if (sceSifGetReg(SIF_SYSREG_RPCINIT))
        return;

    cmdp    = (u32 *)&pkt_table[64];
    cmdp[3] = 1;
    sceSifSendCmd(SIF_CMD_INIT_CMD, cmdp, 16, NULL, NULL, 0);

    while (!sceSifGetSreg(SIF_SREG_RPCINIT))
        ;
    sceSifSetReg(SIF_SYSREG_RPCINIT, 1);
}

void sceSifExitRpc(void)
{
    sceSifExitCmd();
    init = 0;
}
#endif

#ifdef F_sceSifRegisterRpc
void sceSifRegisterRpc(SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf,
    SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd)
{
    SifRpcServerData_t *server;

    DI();

    sd->link  = NULL;
    sd->next  = NULL;
    sd->sid   = sid;
    sd->func  = func;
    sd->buf   = buf;
    sd->cfunc = cfunc;
    sd->cbuf  = cbuf;
    sd->base  = qd;

    if (!(server = qd->link)) {
        qd->link = sd;
    } else {
        while (server->link != NULL) {
            server = server->link;
        }

        server->link = sd;
    }

    EI();
}
#endif

#ifdef F_sceSifRemoveRpc
SifRpcServerData_t *sceSifRemoveRpc(SifRpcServerData_t *sd, SifRpcDataQueue_t *qd)
{
    SifRpcServerData_t *server;

    DI();

    if ((server = qd->link) == sd) {
        qd->link = server->link;
    } else {
        while (server != NULL) {
            if (server->link == sd) {
                server->link = sd->link;
                break;
            }

            server = server->link;
        }
    }

    EI();

    return server;
}
#endif

#ifdef F_sceSifSetRpcQueue
void sceSifSetRpcQueue(SifRpcDataQueue_t *qd, int thread_id)
{
    SifRpcDataQueue_t *queue = NULL;

    DI();

    qd->thread_id = thread_id;
    qd->active    = 0;
    qd->link      = NULL;
    qd->start     = NULL;
    qd->end       = NULL;
    qd->next      = NULL;

    if (_sif_rpc_data.active_queue == NULL) {
        _sif_rpc_data.active_queue = qd;
    } else {
        queue = _sif_rpc_data.active_queue;

        while (queue->next != NULL)
            queue = queue->next;

        queue->next = qd;
    }

    EI();
}
#endif

#ifdef F_sceSifRemoveRpcQueue
SifRpcDataQueue_t *sceSifRemoveRpcQueue(SifRpcDataQueue_t *qd)
{
    SifRpcDataQueue_t *queue;

    DI();

    if ((queue = _sif_rpc_data.active_queue) == qd) {
        _sif_rpc_data.active_queue = queue->next;
    } else {
        while (queue != NULL) {
            if (queue->next == qd) {
                queue->next = qd->next;
                break;
            }

            queue = queue->next;
        }
    }

    EI();

    return queue;
}
#endif

#ifdef F_sceSifGetNextRequest
SifRpcServerData_t *sceSifGetNextRequest(SifRpcDataQueue_t *qd)
{
    SifRpcServerData_t *sd;

    DI();

    sd = qd->start;
    if (sd != NULL) {
        qd->active = 1;
        qd->start  = sd->next;
    } else {
        qd->active = 0;
    }

    EI();

    return sd;
}
#endif

#ifdef F_sceSifExecRequest
static void *_rpc_get_fpacket2(struct rpc_data *rpc_data, int rid)
{
    if (rid < 0 || rid < rpc_data->client_table_len)
        return _rpc_get_fpacket(rpc_data);
    else
        return rpc_data->client_table + (rid * RPC_PACKET_SIZE);
}
void sceSifExecRequest(SifRpcServerData_t *sd)
{
    SifDmaTransfer_t dmat;
    SifRpcRendPkt_t *rend;
    void *rec = NULL;

    rec = sd->func(sd->rpc_number, sd->buf, sd->size);

    if (sd->size)
        sceSifWriteBackDCache(sd->buf, sd->size);

    if (sd->rsize)
        sceSifWriteBackDCache(rec, sd->rsize);

    DI();

    if (sd->rid & 4)
        rend = (SifRpcRendPkt_t *)
            _rpc_get_fpacket2(&_sif_rpc_data, (sd->rid >> 16) & 0xffff);
    else
        rend = (SifRpcRendPkt_t *)
            _rpc_get_fpacket(&_sif_rpc_data);

    EI();

    rend->cd     = sd->client;
    rend->cid    = SIF_CMD_RPC_CALL;

    if (sd->rmode) {
        if (!sceSifSendCmd(SIF_CMD_RPC_END, rend, RPC_PACKET_SIZE, rec, sd->recvbuf,
                        sd->rsize))
            return;
    }

    rend->rpc_id = 0;
    rend->rec_id = 0;

    if (sd->rsize) {
        dmat.src  = rec;
        dmat.dest = sd->recvbuf;
        dmat.size = sd->rsize;
        dmat.attr = 0;
    } else {
        dmat.src  = rend;
        dmat.dest = sd->pkt_addr;
        dmat.size = 64;
        dmat.attr = 0;
    }

    while (!sceSifSetDma(&dmat, 1))
        nopdelay();
}
#endif

#ifdef F_sceSifRpcLoop
void sceSifRpcLoop(SifRpcDataQueue_t *qd)
{
    while (1) {
        SifRpcServerData_t *sd;

        while ((sd = sceSifGetNextRequest(qd)))
            sceSifExecRequest(sd);

        SleepThread();
    }
}
#endif

#ifdef F_sceSifCheckStatRpc
int sceSifCheckStatRpc(SifRpcClientData_t *cd)
{
    SifRpcPktHeader_t *packet = (SifRpcPktHeader_t *)cd->hdr.pkt_addr;

    if (!packet)
        return 0;

    if (cd->hdr.rpc_id != (u32)(packet->rpc_id))
        return 0;

    if (!(packet->rec_id & PACKET_F_ALLOC))
        return 0;

    return 1;
}
#endif
