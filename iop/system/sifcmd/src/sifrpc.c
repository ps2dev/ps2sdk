/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include "sifcmd.h"

typedef union sif_rpc_tbl_data_
{
	u8 m_data[64];
} __attribute__((aligned(16))) sif_rpc_tbl_data_t;

typedef struct sif_rpc_data_
{
	int m_pid;
	sif_rpc_tbl_data_t *m_pkt_table;
	int m_pkt_table_len;
	int m_unused1;
	int m_unused2;
	sif_rpc_tbl_data_t *m_rdata_table;
	int m_rdata_table_len;
	sif_rpc_tbl_data_t *m_client_table;
	int m_client_table_len;
	int m_rdata_table_idx;
	SifRpcDataQueue_t *m_active_queue;
	int m_sif_rpc_sema_ef;
	u32 m_used_sema_bitfield;
} __attribute__((aligned(16))) sif_rpc_data_t;

static sif_rpc_data_t g_sif_rpc_data_table;
static u32 g_init = 0;
static sif_rpc_tbl_data_t g_packet_buffer[32];
static sif_rpc_tbl_data_t g_free_buffer[32];
static sif_rpc_tbl_data_t g_free_buffer2[32];

static void _request_end(void *data, void *harg);
static void _request_bind(void *data, void *harg);
static void _request_call(void *data, void *harg);
static void _request_rdata(void *data, void *harg);

void sceSifInitRpc(int mode)
{
	int state;

	(void)mode;

	sceSifInitCmd();
	CpuSuspendIntr(&state);
	if ( g_init )
	{
		CpuResumeIntr(state);
	}
	else
	{
		SifCmdSRegData_t *spkt;
		iop_event_t efp;

		g_init = 1;
		g_sif_rpc_data_table.m_pkt_table = g_packet_buffer;
		g_sif_rpc_data_table.m_pkt_table_len = sizeof(g_packet_buffer) / sizeof(g_packet_buffer[0]);
		g_sif_rpc_data_table.m_unused1 = 0;
		g_sif_rpc_data_table.m_unused2 = 0;
		g_sif_rpc_data_table.m_rdata_table = g_free_buffer;
		g_sif_rpc_data_table.m_rdata_table_len = sizeof(g_free_buffer) / sizeof(g_free_buffer[0]);
		g_sif_rpc_data_table.m_client_table = g_free_buffer2;
		g_sif_rpc_data_table.m_client_table_len = sizeof(g_free_buffer2) / sizeof(g_free_buffer2[0]);
		g_sif_rpc_data_table.m_rdata_table_idx = 0;
		g_sif_rpc_data_table.m_pid = 1;
		sceSifAddCmdHandler(SIF_CMD_RPC_END, _request_end, (void *)&g_sif_rpc_data_table);
		sceSifAddCmdHandler(SIF_CMD_RPC_BIND, _request_bind, (void *)&g_sif_rpc_data_table);
		sceSifAddCmdHandler(SIF_CMD_RPC_CALL, _request_call, (void *)&g_sif_rpc_data_table);
		sceSifAddCmdHandler(SIF_CMD_RPC_RDATA, _request_rdata, (void *)&g_sif_rpc_data_table);
		efp.attr = EA_MULTI;
		efp.bits = 0;
		g_sif_rpc_data_table.m_sif_rpc_sema_ef = CreateEventFlag(&efp);
		g_sif_rpc_data_table.m_used_sema_bitfield = 0;
		CpuResumeIntr(state);
		spkt = (SifCmdSRegData_t *)&g_sif_rpc_data_table.m_pkt_table[0].m_data;
		spkt->index = SIF_SREG_RPCINIT;
		spkt->value = 1;
		sceSifSendCmd(SIF_CMD_SET_SREG, spkt, sizeof(*spkt), NULL, NULL, 0);
	}
	WaitEventFlag(GetSystemStatusFlag(), 0x800, WEF_AND, NULL);
}

static int sif_rpc_get_sema(sif_rpc_data_t *sci)
{
	unsigned int i;
	int state;

	CpuSuspendIntr(&state);
	for ( i = 0; i < (sizeof(sci->m_used_sema_bitfield) * 8); i += 1 )
	{
		if ( !(sci->m_used_sema_bitfield & ((u32)1 << i)) )
		{
			sci->m_used_sema_bitfield |= 1 << i;
			CpuResumeIntr(state);
			return i;
		}
	}
	CpuResumeIntr(state);
	return -1;
}

static void sif_rpc_free_sema(sif_rpc_data_t *sci, char sema_id)
{
	int state;

	CpuSuspendIntr(&state);
	sci->m_used_sema_bitfield &= ~(1 << sema_id);
	CpuResumeIntr(state);
}

static SifRpcPktHeader_t *_sceRpcGetPacket(sif_rpc_data_t *sci)
{
	int i;
	int state;

	CpuSuspendIntr(&state);
	for ( i = 0; i < sci->m_pkt_table_len; i += 1 )
	{
		SifRpcPktHeader_t *pkt;

		pkt = (SifRpcPktHeader_t *)&sci->m_pkt_table[i].m_data;
		if ( !((unsigned int)pkt->rec_id & 2) )
		{
			pkt->rec_id = (i << 16) | 6;
			sci->m_pid += 1;
			pkt->rpc_id = sci->m_pid;
			sci->m_pid += !!(sci->m_pid == 1);
			pkt->pkt_addr = pkt;
			CpuResumeIntr(state);
			return pkt;
		}
	}
	CpuResumeIntr(state);
	return NULL;
}

static void _sceRpcFreePacket(SifRpcPktHeader_t *pkt)
{
	pkt->rpc_id = 0;
	pkt->rec_id &= ~2;
}

static void *_sceRpcGetFPacket(sif_rpc_data_t *sci)
{
	if ( sci->m_rdata_table_len == -1 && (u32)sci->m_rdata_table_idx == 0x80000000 )
		__builtin_trap();
	sci->m_rdata_table_idx %= sci->m_rdata_table_len;
	sci->m_rdata_table_idx += 1;
	return &sci->m_rdata_table[sci->m_rdata_table_idx - 1].m_data;
}

static void *_sceRpcGetFPacket2(sif_rpc_data_t *sci, int rid)
{
	return (rid >= 0 && rid < sci->m_client_table_len) ? &sci->m_client_table[rid].m_data : _sceRpcGetFPacket(sci);
}

static void _request_end(void *data, void *harg)
{
	SifRpcRendPkt_t *rpkt;
	const sif_rpc_data_t *sci;

	rpkt = (SifRpcRendPkt_t *)data;
	sci = (const sif_rpc_data_t *)harg;
	switch ( rpkt->cid )
	{
		case SIF_CMD_RPC_CALL:
			// Unofficial: unconditionally call callback
			rpkt->cd->end_function(rpkt->cd->end_param);
			break;
		case SIF_CMD_RPC_BIND:
			rpkt->cd->server = rpkt->sd;
			rpkt->cd->buf = rpkt->buf;
			break;
		default:
			break;
	}
	if ( rpkt->cd->hdr.sema_id >= 0 )
		iSetEventFlag(sci->m_sif_rpc_sema_ef, 1 << rpkt->cd->hdr.sema_id);
	_sceRpcFreePacket((SifRpcPktHeader_t *)rpkt->cd->hdr.pkt_addr);
	rpkt->cd->hdr.pkt_addr = NULL;
}

static unsigned int _alarm_rdata(void *userdata)
{
	SifRpcRendPkt_t *spkt;

	spkt = (SifRpcRendPkt_t *)userdata;
	return isceSifSendCmd(SIF_CMD_RPC_END, spkt, sizeof(sif_rpc_tbl_data_t), spkt->sd, spkt->buf, (uiptr)spkt->cbuf) ?
					 0 :
					 0xF000;
}

static void _request_rdata(void *data, void *harg)
{
	SifRpcOtherDataPkt_t *rpkt;
	sif_rpc_data_t *sci;
	SifRpcRendPkt_t *spkt;
	iop_sys_clock_t clk;

	rpkt = (SifRpcOtherDataPkt_t *)data;
	sci = (sif_rpc_data_t *)harg;
	spkt = (SifRpcRendPkt_t *)((rpkt->rec_id & 4) ? _sceRpcGetFPacket2(sci, (rpkt->rec_id >> 16) & 0xFFFF) :
																									_sceRpcGetFPacket(sci));
	spkt->pkt_addr = rpkt->pkt_addr;
	spkt->cid = SIF_CMD_RPC_RDATA;
	spkt->cd = (SifRpcClientData_t *)rpkt->recvbuf;
	spkt->sd = (SifRpcServerData_t *)rpkt->src;
	spkt->buf = rpkt->dest;
	spkt->cbuf = (void *)rpkt->size;
	if ( isceSifSendCmd(SIF_CMD_RPC_END, spkt, sizeof(sif_rpc_tbl_data_t), rpkt->src, rpkt->dest, rpkt->size) )
		return;
	clk.hi = 0;
	clk.lo = 0xF000;
	iSetAlarm(&clk, _alarm_rdata, spkt);
}

int sceSifGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest, int size, int mode)
{
	SifRpcOtherDataPkt_t *spkt;

	spkt = (SifRpcOtherDataPkt_t *)_sceRpcGetPacket(&g_sif_rpc_data_table);
	if ( !spkt )
		return -1;
	rd->hdr.pkt_addr = spkt;
	rd->hdr.rpc_id = spkt->rpc_id;
	spkt->pkt_addr = spkt;
	spkt->recvbuf = rd;
	spkt->src = src;
	spkt->dest = dest;
	spkt->size = size;
	if ( (mode & SIF_RPC_M_NOWAIT) )
	{
		rd->hdr.sema_id = -1;
		if ( !sceSifSendCmd(SIF_CMD_RPC_RDATA, spkt, sizeof(sif_rpc_tbl_data_t), NULL, NULL, 0) )
		{
			_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
			return -2;
		}
		return 0;
	}
	rd->hdr.sema_id = sif_rpc_get_sema(&g_sif_rpc_data_table);
	if ( rd->hdr.sema_id < 0 )
	{
		_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
		return -4;
	}
	if ( !sceSifSendCmd(SIF_CMD_RPC_RDATA, spkt, sizeof(sif_rpc_tbl_data_t), NULL, NULL, 0) )
	{
		_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
		sif_rpc_free_sema(&g_sif_rpc_data_table, rd->hdr.sema_id);
		return -2;
	}
	WaitEventFlag(g_sif_rpc_data_table.m_sif_rpc_sema_ef, 1 << rd->hdr.sema_id, WEF_AND, NULL);
	ClearEventFlag(g_sif_rpc_data_table.m_sif_rpc_sema_ef, ~(1 << rd->hdr.sema_id));
	sif_rpc_free_sema(&g_sif_rpc_data_table, rd->hdr.sema_id);
	return 0;
}

static SifRpcServerData_t *_search_svdata(int sid, const sif_rpc_data_t *rpc_data)
{
	const SifRpcDataQueue_t *qd;
	SifRpcServerData_t *sd;

	for ( qd = rpc_data->m_active_queue; qd; qd = qd->next )
		for ( sd = qd->link; sd; sd = sd->link )
			if ( sd->sid == sid )
				return sd;
	return NULL;
}

static unsigned int _alarm_bind(void *userdata)
{
	SifRpcRendPkt_t *spkt;

	spkt = (SifRpcRendPkt_t *)userdata;
	return isceSifSendCmd(SIF_CMD_RPC_END, spkt, sizeof(sif_rpc_tbl_data_t), NULL, NULL, 0) ? 0 : 0xF000;
}

static void _request_bind(void *data, void *harg)
{
	SifRpcBindPkt_t *rpkt;
	sif_rpc_data_t *sci;
	SifRpcRendPkt_t *spkt;
	iop_sys_clock_t clk;

	rpkt = (SifRpcBindPkt_t *)data;
	sci = (sif_rpc_data_t *)harg;
	spkt = (SifRpcRendPkt_t *)_sceRpcGetFPacket(sci);
	spkt->pkt_addr = rpkt->pkt_addr;
	spkt->cid = SIF_CMD_RPC_BIND;
	spkt->cd = rpkt->cd;
	spkt->sd = _search_svdata(rpkt->sid, sci);
	spkt->buf = spkt->sd ? spkt->sd->buf : NULL;
	if ( isceSifSendCmd(SIF_CMD_RPC_END, spkt, sizeof(sif_rpc_tbl_data_t), NULL, NULL, 0) )
		return;
	clk.hi = 0;
	clk.lo = 0xF000;
	iSetAlarm(&clk, _alarm_bind, spkt);
}

int sceSifBindRpc(SifRpcClientData_t *cd, int sid, int mode)
{
	SifRpcBindPkt_t *spkt;

	cd->command = 0;
	cd->server = NULL;
	spkt = (SifRpcBindPkt_t *)_sceRpcGetPacket(&g_sif_rpc_data_table);
	if ( !spkt )
		return -1;
	cd->hdr.pkt_addr = spkt;
	cd->hdr.rpc_id = spkt->rpc_id;
	spkt->pkt_addr = spkt;
	spkt->cd = cd;
	spkt->sid = sid;
	if ( (mode & SIF_RPC_M_NOWAIT) )
	{
		cd->hdr.sema_id = -1;
		if ( !sceSifSendCmd(SIF_CMD_RPC_BIND, spkt, sizeof(sif_rpc_tbl_data_t), NULL, NULL, 0) )
		{
			_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
			return -2;
		}
		return 0;
	}
	cd->hdr.sema_id = sif_rpc_get_sema(&g_sif_rpc_data_table);
	if ( cd->hdr.sema_id < 0 )
	{
		_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
		return -4;
	}
	if ( !sceSifSendCmd(SIF_CMD_RPC_BIND, spkt, sizeof(sif_rpc_tbl_data_t), NULL, NULL, 0) )
	{
		_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
		sif_rpc_free_sema(&g_sif_rpc_data_table, cd->hdr.sema_id);
		return -2;
	}
	WaitEventFlag(g_sif_rpc_data_table.m_sif_rpc_sema_ef, 1 << cd->hdr.sema_id, WEF_AND, NULL);
	ClearEventFlag(g_sif_rpc_data_table.m_sif_rpc_sema_ef, ~(1 << cd->hdr.sema_id));
	sif_rpc_free_sema(&g_sif_rpc_data_table, cd->hdr.sema_id);
	return 0;
}

static void _request_call(void *data, void *harg)
{
	SifRpcCallPkt_t *rpkt;
	SifRpcServerData_t *sd;

	(void)harg;

	rpkt = (SifRpcCallPkt_t *)data;
	sd = rpkt->sd;
	*(sd->base->start ? &(sd->base->end->next) : &(sd->base->start)) = sd;
	sd->base->end = sd;
	sd->pkt_addr = rpkt->pkt_addr;
	sd->client = rpkt->cd;
	sd->rpc_number = rpkt->rpc_number;
	sd->size = rpkt->send_size;
	sd->recvbuf = rpkt->recvbuf;
	sd->rsize = rpkt->recv_size;
	sd->rmode = rpkt->rmode;
	sd->rid = rpkt->rec_id;
	if ( sd->base->thread_id >= 0 && !sd->base->active )
		iWakeupThread(sd->base->thread_id);
}

static void sif_end_function_noop(void *userdata)
{
	(void)userdata;
}

int sceSifCallRpc(
	SifRpcClientData_t *cd,
	int rpc_number,
	int mode,
	void *sendbuf,
	int ssize,
	void *recvbuf,
	int rsize,
	SifRpcEndFunc_t end_function,
	void *end_param)
{
	SifRpcCallPkt_t *spkt;

	spkt = (SifRpcCallPkt_t *)_sceRpcGetPacket(&g_sif_rpc_data_table);
	if ( !spkt )
		return -1;
	cd->hdr.pkt_addr = spkt;
	// Unofficial: use no-op function if function is NULL
	cd->end_function = end_function ? end_function : &sif_end_function_noop;
	cd->end_param = end_param;
	cd->hdr.rpc_id = spkt->rpc_id;
	spkt->pkt_addr = spkt;
	spkt->cd = cd;
	spkt->rpc_number = rpc_number;
	spkt->send_size = ssize;
	spkt->recvbuf = recvbuf;
	spkt->recv_size = rsize;
	spkt->sd = cd->server;
	if ( (mode & SIF_RPC_M_NOWAIT) )
	{
		spkt->rmode = !!end_function;
		cd->hdr.sema_id = -1;
		if ( !sceSifSendCmd(SIF_CMD_RPC_CALL, spkt, sizeof(sif_rpc_tbl_data_t), sendbuf, cd->buf, ssize) )
		{
			_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
			return -2;
		}
		return 0;
	}
	spkt->rmode = 1;
	cd->hdr.sema_id = sif_rpc_get_sema(&g_sif_rpc_data_table);
	if ( cd->hdr.sema_id < 0 )
	{
		_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
		return -4;
	}
	if ( !sceSifSendCmd(SIF_CMD_RPC_CALL, spkt, sizeof(sif_rpc_tbl_data_t), sendbuf, cd->buf, ssize) )
	{
		_sceRpcFreePacket((SifRpcPktHeader_t *)spkt);
		sif_rpc_free_sema(&g_sif_rpc_data_table, cd->hdr.sema_id);
		return -2;
	}
	WaitEventFlag(g_sif_rpc_data_table.m_sif_rpc_sema_ef, 1 << cd->hdr.sema_id, WEF_AND, NULL);
	ClearEventFlag(g_sif_rpc_data_table.m_sif_rpc_sema_ef, ~(1 << cd->hdr.sema_id));
	sif_rpc_free_sema(&g_sif_rpc_data_table, cd->hdr.sema_id);
	return 0;
}

int sceSifCheckStatRpc(SifRpcClientData_t *cd)
{
	const SifRpcPktHeader_t *pkt_addr;

	pkt_addr = (SifRpcPktHeader_t *)cd->hdr.pkt_addr;
	return cd->hdr.pkt_addr && (int)(cd->hdr.rpc_id) == pkt_addr->rpc_id && (pkt_addr->rec_id & 2);
}

void sceSifSetRpcQueue(SifRpcDataQueue_t *qd, int thread_id)
{
	SifRpcDataQueue_t *cur_qd;
	int state;

	CpuSuspendIntr(&state);
	qd->thread_id = thread_id;
	qd->active = 0;
	qd->link = NULL;
	qd->start = NULL;
	qd->end = NULL;
	qd->next = NULL;
	for ( cur_qd = g_sif_rpc_data_table.m_active_queue; cur_qd && cur_qd->next; cur_qd = cur_qd->next )
		;
	*(cur_qd ? &(cur_qd->next) : &g_sif_rpc_data_table.m_active_queue) = qd;
	CpuResumeIntr(state);
}

void sceSifRegisterRpc(
	SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf, SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *cur_sd;
	int state;

	CpuSuspendIntr(&state);
	sd->sid = sid;
	sd->func = func;
	sd->buf = buf;
	sd->next = NULL;
	sd->link = NULL;
	sd->cfunc = cfunc;
	sd->cbuf = cbuf;
	sd->base = qd;
	cur_sd = qd->link;
	for ( ; cur_sd && cur_sd->link; cur_sd = cur_sd->link )
		;
	*(cur_sd ? &(cur_sd->link) : &(qd->link)) = sd;
	CpuResumeIntr(state);
}

SifRpcServerData_t *sceSifRemoveRpc(SifRpcServerData_t *sd, SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t **p_sd;
	int state;

	CpuSuspendIntr(&state);
	for ( p_sd = &(qd->link); *p_sd; p_sd = &((*p_sd)->link) )
	{
		if ( *p_sd == sd )
		{
			*p_sd = (*p_sd)->link;
			p_sd = NULL;
			break;
		}
	}
	CpuResumeIntr(state);
	return p_sd ? NULL : sd;
}

SifRpcDataQueue_t *sceSifRemoveRpcQueue(SifRpcDataQueue_t *qd)
{
	SifRpcDataQueue_t **p_qd;
	int state;

	CpuSuspendIntr(&state);
	for ( p_qd = &g_sif_rpc_data_table.m_active_queue; *p_qd; p_qd = &((*p_qd)->next) )
	{
		if ( *p_qd == qd )
		{
			*p_qd = (*p_qd)->next;
			p_qd = NULL;
			break;
		}
	}
	CpuResumeIntr(state);
	return p_qd ? NULL : qd;
}

SifRpcServerData_t *sceSifGetNextRequest(SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *sd;
	int state;

	CpuSuspendIntr(&state);
	sd = qd->start;
	qd->active = !!sd;
	qd->start = sd ? qd->start->next : NULL;
	CpuResumeIntr(state);
	return sd;
}

void sceSifExecRequest(SifRpcServerData_t *sd)
{
	void *rec;
	SifRpcRendPkt_t *spkt;
	int state;
	int dmatc;
	SifDmaTransfer_t dmat[2];

	rec = (void *)sd->func(sd->rpc_number, sd->buf, sd->size);
	CpuSuspendIntr(&state);
	spkt = (SifRpcRendPkt_t *)((sd->rid & 4) ? _sceRpcGetFPacket2(&g_sif_rpc_data_table, (sd->rid >> 16) & 0xFFFF) :
																						 _sceRpcGetFPacket(&g_sif_rpc_data_table));
	CpuResumeIntr(state);
	spkt->cid = SIF_CMD_RPC_CALL;
	spkt->cd = sd->client;
	if ( sd->rmode )
	{
		while ( !sceSifSendCmd(SIF_CMD_RPC_END, spkt, sizeof(sif_rpc_tbl_data_t), rec, sd->recvbuf, rec ? sd->rsize : 0) )
			;
		return;
	}
	dmatc = 0;
	spkt->rpc_id = 0;
	spkt->rec_id = 0;
	dmat[dmatc].src = rec;
	dmat[dmatc].size = rec ? sd->rsize : 0;
	dmat[dmatc].attr = 0;
	dmat[dmatc].dest = sd->recvbuf;
	dmatc += !!((int)(dmat[dmatc].size) > 0);
	dmat[dmatc].src = spkt;
	dmat[dmatc].size = sizeof(sif_rpc_tbl_data_t);
	dmat[dmatc].attr = 0;
	dmat[dmatc].dest = sd->pkt_addr;
	while ( 1 )
	{
		int trid;
		int busywait;

		CpuSuspendIntr(&state);
		trid = sceSifSetDma(dmat, dmatc + 1);
		CpuResumeIntr(state);
		if ( trid )
			break;
		for ( busywait = 0xFFFF; busywait; busywait -= 1 )
			;
	}
}

void sceSifRpcLoop(SifRpcDataQueue_t *qd)
{
	while ( 1 )
	{
		SifRpcServerData_t *sd;

		sd = sceSifGetNextRequest(qd);
		if ( sd )
			sceSifExecRequest(sd);
		else
			SleepThread();
	}
}
