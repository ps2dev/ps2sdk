/*
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2001, Gustavo Scotti (gustavo@scotti.com)
                           (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
  ------------------------------------------------------------------------
  sifrpc.c
			EE SIF RPC commands
                        These are my findings based on debug-info elf files.

			MRB: This file now contains the SIF routines included
			with libpsware.  Bug reports welcome.
*/

#include <tamtypes.h>
#include <ps2lib_err.h>
#include <kernel.h>
#include <sifcmd.h>
#include <sifrpc.h>

#define RPC_PACKET_SIZE	64

/* Set if the packet has been allocated */
#define PACKET_F_ALLOC	0x01

struct rpc_data {
	int	pid;
	void	*pkt_table;
	int	pkt_table_len;
	int	unused1;
	int	unused2;
	u8	*rdata_table;
	int	rdata_table_len;
	u8	*client_table;
	int	client_table_len;
	int	rdata_table_idx;
	void	*active_queue;
};

extern struct rpc_data _sif_rpc_data;

void *_rpc_get_packet(struct rpc_data *rpc_data);
void *_rpc_get_fpacket(struct rpc_data *rpc_data);

#ifdef F__rpc_get_packet
void *_rpc_get_packet(struct rpc_data *rpc_data)
{
	SifRpcPktHeader_t *packet;
	int len, pid, rid = 0;

	DI();

	len = rpc_data->pkt_table_len;
	if (len > 0) {
		packet = (SifRpcPktHeader_t *)rpc_data->pkt_table;

		for (rid = 0; rid < len; rid++, (u8 *)packet += 64) {
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
			pid = 1;
		}
		
		packet->rec_id = (rid << 16) | 0x04 | PACKET_F_ALLOC;
		packet->rpc_id = pid;
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

	index = rpc_data->rdata_table_idx % rpc_data->rdata_table_len;
	rpc_data->rdata_table_idx = index + 1;

	return (void *)(rpc_data->rdata_table + (index * RPC_PACKET_SIZE));
}
#endif

#ifdef F_SifBindRpc
int SifBindRpc(SifRpcClientData_t *cd, int sid, int mode)
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

	bind->sid        = sid;
	bind->pkt_addr   = bind;
	bind->client     = cd;

	if (mode & SIF_RPC_M_NOWAIT) {
		if (!SifSendCmd(0x80000009, bind, RPC_PACKET_SIZE, NULL, NULL, 0))
			return -E_SIF_PKT_SEND;

		return 0;
	}

	sema.max_count  = 1;
	sema.init_count = 0;
	cd->hdr.sema_id = CreateSema(&sema);
	if (cd->hdr.sema_id < 0)
		return -E_LIB_SEMA_CREATE;

	if (!SifSendCmd(0x80000009, bind, RPC_PACKET_SIZE, NULL, NULL, 0))
		return -E_SIF_PKT_SEND;

	WaitSema(cd->hdr.sema_id);
	DeleteSema(cd->hdr.sema_id);

	return 0;
}
#endif

#ifdef F_SifCallRpc
int SifCallRpc(SifRpcClientData_t *cd, int rpc_number, int mode,
		void *sendbuf, int ssize, void *recvbuf, int rsize,
		SifRpcEndFunc_t endfunc, void *efarg)
{
	ee_sema_t sema;
	SifRpcCallPkt_t *call;

	call = (SifRpcCallPkt_t *)_rpc_get_packet(&_sif_rpc_data);
	if (!call)
		return -E_SIF_PKT_ALLOC;

	cd->hdr.pkt_addr  = call;
	cd->hdr.rpc_id    = call->rpc_id;
	cd->hdr.sema_id   = -1;
	cd->end_function  = endfunc;
	cd->end_param     = efarg;

	call->rpc_number  = rpc_number;
	call->send_size   = ssize;
	call->receive     = recvbuf;
	call->recv_size   = rsize;
	call->rmode       = 1;
	call->pkt_addr    = call;
	call->client      = cd;
	call->server      = cd->server;

	if (!(mode & SIF_RPC_M_NOWBDC)) {
		if (ssize > 0)
			SifWriteBackDCache(sendbuf, ssize);
		if (rsize > 0)
			SifWriteBackDCache(recvbuf, rsize);
	}

	if (mode & SIF_RPC_M_NOWAIT) {
		if (!endfunc)
			call->rmode = 0;

		if (!SifSendCmd(0x8000000a, call, RPC_PACKET_SIZE, sendbuf,
					cd->buff, ssize))
			return -E_SIF_PKT_SEND;

		return 0;
	}

	sema.max_count  = 1;
	sema.init_count = 0;
	cd->hdr.sema_id = CreateSema(&sema);
	if (cd->hdr.sema_id < 0)
		return -E_LIB_SEMA_CREATE;

	if (!SifSendCmd(0x8000000a, call, RPC_PACKET_SIZE, sendbuf,
				cd->buff, ssize))
		return -E_SIF_PKT_SEND;

	WaitSema(cd->hdr.sema_id);
	DeleteSema(cd->hdr.sema_id);

	return 0;
}
#endif

#ifdef F_SifRpcGetOtherData
int SifRpcGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest,
		int size, int mode)
{
	ee_sema_t sema;
	SifRpcOtherDataPkt_t *other;

	other = (SifRpcOtherDataPkt_t *)_rpc_get_packet(&_sif_rpc_data);
	if (!other)
		return -E_SIF_PKT_ALLOC;

	rd->hdr.pkt_addr = other;
	rd->hdr.rpc_id   = other->rpc_id;
	rd->hdr.sema_id  = -1;

	other->src        = src;
	other->dest       = dest;
	other->size       = size;
	other->receive    = rd;

	if (mode & SIF_RPC_M_NOWAIT) {
		if (!SifSendCmd(0x8000000c, other, 64, NULL, NULL, 0))
			return -E_SIF_PKT_SEND;

		return 0;
	}

	sema.max_count  = 1;
	sema.init_count = 1;
	rd->hdr.sema_id = CreateSema(&sema);
	if (rd->hdr.sema_id < 0)
		return -E_LIB_SEMA_CREATE;

	if (!SifSendCmd(0x8000000c, other, 64, NULL, NULL, 0))
		return -E_SIF_PKT_SEND;

	WaitSema(rd->hdr.sema_id);
	DeleteSema(rd->hdr.sema_id);

	return 0;
}
#endif

#ifdef F_SifRpcMain

/* The packets sent on EE RPC requests are allocated from this table.  */
static u8 pkt_table[2048] __attribute__((aligned(64)));
/* A ring buffer used to allocate packets sent on IOP requests.  */
static u8 rdata_table[2048];
static u8 client_table[2048];

struct rpc_data _sif_rpc_data = {
	pid:			1,
	pkt_table:		pkt_table,
	pkt_table_len:		sizeof(pkt_table)/RPC_PACKET_SIZE,
	rdata_table:		rdata_table,
	rdata_table_len:	sizeof(rdata_table)/RPC_PACKET_SIZE,
	client_table:		client_table,
	client_table_len:	sizeof(client_table)/RPC_PACKET_SIZE,
	rdata_table_idx:	0
};

static int init = 0;

static void rpc_packet_free(void *packet)
{
	SifRpcRendPkt_t *rendpkt = (SifRpcRendPkt_t *)packet;

	rendpkt->rpc_id = 0;
	rendpkt->rec_id &= (~PACKET_F_ALLOC);
}

/* Command 0x80000008 */
static void _request_end(SifRpcRendPkt_t *request, void *data)
{
	SifRpcClientData_t *client = request->client;

	if (request->cid == 0x8000000a) {
		if (client->end_function)
			client->end_function(client->end_param);
	} else if (request->cid == 0x80000009) {
		client->server = request->server;
		client->buff   = request->buff;
		client->cbuff  = request->cbuff;
	}

	if (client->hdr.sema_id >= 0)
		iSignalSema(client->hdr.sema_id);

	rpc_packet_free(client->hdr.pkt_addr);
	client->hdr.pkt_addr = NULL;
}

static void *search_svdata(u32 sid, struct rpc_data *rpc_data)
{
	SifRpcServerData_t *server;
	SifRpcDataQueue_t *queue = rpc_data->active_queue;

	if (!queue)
		return NULL;

	while (queue) {
		server = queue->link;
		while (server) {
			if (server->sid == sid)
				return server;

			server = server->link;
		}

		queue = queue->next;
	}

	return NULL;
}

/* Command 0x80000009 */
static void _request_bind(SifRpcBindPkt_t *bind, void *data)
{
	SifRpcRendPkt_t *rend;
	SifRpcServerData_t *server;

	rend = _rpc_get_fpacket(data);
	rend->pkt_addr = bind->pkt_addr;
	rend->client = bind->client;
	rend->cid = 0x80000009;

	server = search_svdata(bind->sid, data);
	if (!server) {
		rend->server = NULL;
		rend->buff   = NULL;
		rend->cbuff  = NULL;
	} else {
		rend->server = server;
		rend->buff   = server->buff;
		rend->cbuff  = server->cbuff;
	}

	iSifSendCmd(0x80000008, rend, 64, NULL, NULL, 0);
}

/* Command 0x8000000a */
static void _request_call(SifRpcCallPkt_t *request, void *data)
{
	SifRpcServerData_t *server = request->server;
	SifRpcDataQueue_t *base = server->base;

	if (base->start)
		base->end->link = server;
	else
		base->start = server;

	base->end          = server;
	server->pkt_addr   = request->pkt_addr;
	server->client     = request->client;
	server->rpc_number = request->rpc_number;
	server->size       = request->send_size;
	server->receive    = request->receive;
	server->rsize      = request->recv_size;
	server->rmode      = request->rmode;
	server->rid        = request->rec_id;

	if (base->thread_id < 0 || base->active == 0)
		return;

	iWakeupThread(base->thread_id);
}

/* Command 0x8000000c */
static void _request_rdata(SifRpcOtherDataPkt_t *rdata, void *data)
{
	SifRpcRendPkt_t *rend;

	rend = (SifRpcRendPkt_t *)_rpc_get_fpacket(data);
	rend->pkt_addr = rdata->pkt_addr;
	rend->client = (SifRpcClientData_t *)rdata->receive;
	rend->cid = 0x8000000c;

	iSifSendCmd(0x80000008, rend, 64, rdata->src, rdata->dest, rdata->size);
}

void SifInitRpc(int mode)
{
	u32 *cmdp;

	if (init)
		return;
	init = 1;

	SifInitCmd();

	DI();
	_sif_rpc_data.pkt_table    = UNCACHED_SEG(_sif_rpc_data.pkt_table);
	_sif_rpc_data.rdata_table  = UNCACHED_SEG(_sif_rpc_data.rdata_table);
	_sif_rpc_data.client_table = UNCACHED_SEG(_sif_rpc_data.client_table);
	
	SifAddCmdHandler(0x80000008, (void *)_request_end, &_sif_rpc_data);
	SifAddCmdHandler(0x80000009, (void *)_request_bind, &_sif_rpc_data);
	SifAddCmdHandler(0x8000000a, (void *)_request_call, &_sif_rpc_data);
	SifAddCmdHandler(0x8000000c, (void *)_request_rdata, &_sif_rpc_data);
	EI();

	if (SifGetReg(0x80000002))
		return;

	cmdp = (u32 *)&pkt_table[64];
	cmdp[3] = 1;
	SifSendCmd(0x80000002, cmdp, 16, NULL, NULL, 0);

	while (!SifGetSreg(0))
		;
	SifSetReg(0x80000002, 1);
}

void SifExitRpc(void)
{
    SifExitCmd();
    init = 0;
}
#endif

#ifdef F_SifRegisterRpc
SifRpcServerData_t *
SifRegisterRpc(SifRpcServerData_t *sd, 
		int sid, SifRpcFunc_t func, void *buff, SifRpcFunc_t cfunc,
		void *cbuff, SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *server;

	DI();

	sd->link  = NULL;
	sd->next  = NULL;
	sd->sid   = sid;
	sd->func  = func;
	sd->buff  = buff;
	sd->cfunc = cfunc;
	sd->cbuff = cbuff;
	sd->base  = qd;

	if (!(server = qd->link)) {
		qd->link = sd;
	} else {
		while ((server = server->next))
			;

		server->next = sd;
	}

	EI();

	return server;
}
#endif

#ifdef F_SifSetRpcQueue
SifRpcDataQueue_t *
SifSetRpcQueue(SifRpcDataQueue_t *qd, int thread_id)
{
	SifRpcDataQueue_t *queue = NULL;

	DI();

	qd->thread_id = thread_id;
	qd->active = 0;
	qd->link   = NULL;
	qd->start  = NULL;
	qd->end    = NULL;
	qd->next   = NULL;

	if (!_sif_rpc_data.active_queue) {
		_sif_rpc_data.active_queue = qd;
	} else {
		queue = _sif_rpc_data.active_queue;

		while ((queue = queue->next))
			;

		queue->next = qd;
	}

	EI();

	return queue;
}
#endif

#ifdef F_SifGetNextRequest
SifRpcServerData_t *
SifGetNextRequest(SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *server;

	DI();

	server = qd->start;
	qd->active = 1;

	if (server) {
		qd->active = 0;
		qd->start  = server->next;
	}

	EI();

	return server;
}
#endif

#ifdef F_SifExecRequest
static void *_rpc_get_fpacket2(struct rpc_data *rpc_data, int rid)
{
	if (rid < 0 || rid < rpc_data->client_table_len)
		return _rpc_get_fpacket(rpc_data);
	else
		return rpc_data->client_table + (rid * RPC_PACKET_SIZE);
}

void SifExecRequest(SifRpcServerData_t *sd)
{
	SifDmaTransfer_t dmat;
	SifRpcRendPkt_t *rend;
	void *rec = NULL;

	rec = sd->func(sd->rpc_number, sd->buff, sd->size);

	if (sd->size)
		SifWriteBackDCache(sd->buff, sd->size);

	if (sd->rsize)
		SifWriteBackDCache(rec, sd->rsize);

	DI();

	if (sd->rid & 4)
		rend = (SifRpcRendPkt_t *)
			_rpc_get_fpacket2(&_sif_rpc_data, (sd->rid >> 16) & 0xffff);
	else
		rend = (SifRpcRendPkt_t *)
			_rpc_get_fpacket(&_sif_rpc_data);

	EI();

	rend->client = sd->client;
	rend->cid    = 0x8000000a;
	rend->rpc_id = 0;  /* XXX: is this correct? */

	if (sd->rmode) {
		if (!SifSendCmd(0x80000008, rend, RPC_PACKET_SIZE, rec, sd->receive,
					sd->rsize))
			return;
	}

	rend->rec_id = 0;

	if (sd->rsize) {
		dmat.src = rec;
		dmat.dest = sd->receive;
		dmat.size = sd->rsize;
		dmat.attr = 0;
	} else {
		dmat.src = rend;
		dmat.dest = sd->pkt_addr;
		dmat.size = 64;
		dmat.attr = 0;
	}

	while (!SifSetDma(&dmat, 1))
		nopdelay();
}
#endif

#ifdef F_SifRpcLoop
void SifRpcLoop(SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *server;

	while (1) {
		while ((server = SifGetNextRequest(qd)))
			SifExecRequest(server);

		SleepThread();
	}
}
#endif

#ifdef F_SifCheckStatRpc
int SifCheckStatRpc(SifRpcClientData_t *cd)
{
	SifRpcPktHeader_t *packet = (SifRpcPktHeader_t *)cd->hdr.pkt_addr;

	if (!packet)
		return 0;

	if (cd->hdr.rpc_id != packet->rpc_id)
		return 0;

	if (!(packet->rec_id & PACKET_F_ALLOC))
		return 0;

	return 1;
}
#endif
