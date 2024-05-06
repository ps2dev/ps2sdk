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

typedef struct sif_rpc_data_
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
	SifRpcDataQueue_t *active_queue;
	int sif_rpc_sema_ef;
	int used_sema_bitfield;
} sif_rpc_data_t;

static sif_rpc_data_t sif_rpc_data;
static u32 init = 0;
static u8 pkt_table[0x800];
static u8 rdata_table[0x800];
static u8 client_data[0x800];

static void sif_cmd_handler_end(SifRpcRendPkt_t *data, sif_rpc_data_t *harg);
static void sif_cmd_handler_bind(SifRpcBindPkt_t *data, sif_rpc_data_t *harg);
static void sif_cmd_handler_call(SifRpcCallPkt_t *data, sif_rpc_data_t *harg);
static void sif_cmd_handler_rdata(SifRpcOtherDataPkt_t *data, sif_rpc_data_t *harg);

void sceSifInitRpc(int mode)
{
	int SystemStatusFlag;
	iop_event_t efp;
	int state;

	(void)mode;

	sceSifInitCmd();
	CpuSuspendIntr(&state);
	if ( init )
	{
		CpuResumeIntr(state);
	}
	else
	{
		sif_rpc_data.rdata_table = (u8 *)rdata_table;
		init = 1;
		sif_rpc_data.pkt_table = pkt_table;
		sif_rpc_data.pkt_table_len = 32;
		sif_rpc_data.unused1 = 0;
		sif_rpc_data.unused2 = 0;
		sif_rpc_data.rdata_table_len = 32;
		sif_rpc_data.client_table = (u8 *)client_data;
		sif_rpc_data.client_table_len = 32;
		sif_rpc_data.rdata_table_idx = 0;
		sif_rpc_data.pid = 1;
		sceSifAddCmdHandler(SIF_CMD_RPC_END, (SifCmdHandler_t)sif_cmd_handler_end, &sif_rpc_data);
		sceSifAddCmdHandler(SIF_CMD_RPC_BIND, (SifCmdHandler_t)sif_cmd_handler_bind, &sif_rpc_data);
		sceSifAddCmdHandler(SIF_CMD_RPC_CALL, (SifCmdHandler_t)sif_cmd_handler_call, &sif_rpc_data);
		sceSifAddCmdHandler(SIF_CMD_RPC_RDATA, (SifCmdHandler_t)sif_cmd_handler_rdata, &sif_rpc_data);
		efp.attr = EA_MULTI;
		efp.bits = 0;
		sif_rpc_data.sif_rpc_sema_ef = CreateEventFlag(&efp);
		sif_rpc_data.used_sema_bitfield = 0;
		CpuResumeIntr(state);
		pkt_table[4] = 0;
		pkt_table[5] = 1;
		sceSifSendCmd(SIF_CMD_SET_SREG, pkt_table, 24, 0, 0, 0);
	}
	SystemStatusFlag = GetSystemStatusFlag();
	WaitEventFlag(SystemStatusFlag, 0x800u, 0, 0);
}

static int sif_rpc_get_sema(sif_rpc_data_t *rpc_data)
{
	int i;
	int state;

	CpuSuspendIntr(&state);
	for ( i = 0; i < 32; ++i )
	{
		int used_sema_bitfield;

		used_sema_bitfield = rpc_data->used_sema_bitfield;
		if ( (used_sema_bitfield & ((u32)1 << i)) == 0 )
		{
			rpc_data->used_sema_bitfield = used_sema_bitfield | (1 << i);
			CpuResumeIntr(state);
			return i;
		}
	}
	CpuResumeIntr(state);
	return -1;
}

static int sif_rpc_free_sema(sif_rpc_data_t *rpc_data, char sema_id)
{
	int state;

	CpuSuspendIntr(&state);
	rpc_data->used_sema_bitfield &= ~(1 << sema_id);
	return CpuResumeIntr(state);
}

static SifRpcPktHeader_t *sif_rpc_packet_get(sif_rpc_data_t *rpc_data)
{
	SifRpcPktHeader_t *pkt_table_0;
	int i;
	int state;

	CpuSuspendIntr(&state);
	pkt_table_0 = (SifRpcPktHeader_t *)rpc_data->pkt_table;
	i = 0;
	if ( rpc_data->pkt_table_len <= 0 )
	{
		CpuResumeIntr(state);
		return 0;
	}
	else
	{
		void **p_pkt_addr;
		int pid;
		int pid_1_tmp;

		p_pkt_addr = &pkt_table_0->pkt_addr;
		while ( ((unsigned int)*(p_pkt_addr - 1) & 2) != 0 )
		{
			++i;
			p_pkt_addr += 16;
			pkt_table_0 = (SifRpcPktHeader_t *)((char *)pkt_table_0 + 64);
			if ( i >= rpc_data->pkt_table_len )
			{
				CpuResumeIntr(state);
				return 0;
			}
		}
		*(p_pkt_addr - 1) = (void *)((i << 16) | 6);
		pid = rpc_data->pid;
		pid_1_tmp = rpc_data->pid + 1;
		rpc_data->pid = pid_1_tmp;
		if ( pid_1_tmp == 1 )
		{
			pid_1_tmp = 1;
			rpc_data->pid = pid + 2;
		}
		p_pkt_addr[1] = (void *)pid_1_tmp;
		*p_pkt_addr = pkt_table_0;
		CpuResumeIntr(state);
		return pkt_table_0;
	}
}

static void sif_rpc_packet_free(SifRpcRendPkt_t *pkt)
{
	pkt->rpc_id = 0;
	pkt->rec_id &= 0xFFFFFFFD;
}

static void *sif_rpc_get_fpacket(sif_rpc_data_t *rpc_data)
{
	int rdata_table_idx;
	int rdata_table_len;
	int index_calc;

	rdata_table_idx = rpc_data->rdata_table_idx;
	rdata_table_len = rpc_data->rdata_table_len;
	index_calc = rdata_table_idx % rdata_table_len;
	if ( rdata_table_len == -1 && (u32)rdata_table_idx == 0x80000000 )
		__builtin_trap();
	rpc_data->rdata_table_idx = index_calc + 1;
	return &rpc_data->rdata_table[64 * index_calc];
}

static void *sif_rpc_get_fpacket2(sif_rpc_data_t *rpc_data, int rid)
{
	if ( rid >= 0 && rid < rpc_data->client_table_len )
		return &rpc_data->client_table[64 * rid];
	else
		return sif_rpc_get_fpacket(rpc_data);
}

static void sif_cmd_handler_end(SifRpcRendPkt_t *data, sif_rpc_data_t *harg)
{
	u32 cid;
	SifRpcClientData_t *client1;
	SifRpcEndFunc_t end_function;
	SifRpcClientData_t *client2;
	SifRpcClientData_t *client3;
	int sema_id;

	cid = data->cid;
	if ( cid == SIF_CMD_RPC_CALL )
	{
		client1 = data->client;
		end_function = client1->end_function;
		if ( end_function )
			end_function(client1->end_param);
	}
	else if ( cid == SIF_CMD_RPC_BIND )
	{
		client2 = data->client;
		client2->server = data->server;
		client2->buff = data->buff;
	}
	client3 = data->client;
	sema_id = client3->hdr.sema_id;
	if ( sema_id >= 0 )
		iSetEventFlag(harg->sif_rpc_sema_ef, 1 << sema_id);
	sif_rpc_packet_free((SifRpcRendPkt_t *)client3->hdr.pkt_addr);
	client3->hdr.pkt_addr = 0;
}

static unsigned int sif_cmd_handler_rdata_alarm_retry(SifRpcRendPkt_t *a1)
{
	if ( isceSifSendCmd(SIF_CMD_RPC_END, a1, 64, a1->server, a1->buff, (int)a1->cbuff) != 0 )
		return 0;
	return 0xF000;
}

static void sif_cmd_handler_rdata(SifRpcOtherDataPkt_t *data, sif_rpc_data_t *harg)
{
	unsigned int rec_id;
	SifRpcRendPkt_t *fpacket2;
	SifRpcClientData_t *receive;
	iop_sys_clock_t clk;

	rec_id = data->rec_id;
	if ( (rec_id & 4) != 0 )
		fpacket2 = (SifRpcRendPkt_t *)sif_rpc_get_fpacket2(harg, (rec_id >> 16) & 0xFFFF);
	else
		fpacket2 = (SifRpcRendPkt_t *)sif_rpc_get_fpacket(harg);
	fpacket2->pkt_addr = data->pkt_addr;
	receive = (SifRpcClientData_t *)data->receive;
	fpacket2->cid = SIF_CMD_RPC_RDATA;
	fpacket2->client = receive;
	fpacket2->server = (SifRpcServerData_t *)data->src;
	fpacket2->buff = data->dest;
	fpacket2->cbuff = (void *)data->size;
	if ( !isceSifSendCmd(SIF_CMD_RPC_END, fpacket2, 64, data->src, data->dest, data->size) )
	{
		clk.hi = 0;
		clk.lo = 0xF000;
		iSetAlarm(&clk, (unsigned int (*)(void *))sif_cmd_handler_rdata_alarm_retry, fpacket2);
	}
}

int sceSifGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest, int size, int mode)
{
	SifRpcOtherDataPkt_t *other;

	other = (SifRpcOtherDataPkt_t *)sif_rpc_packet_get(&sif_rpc_data);
	if ( other )
	{
		rd->hdr.pkt_addr = other;
		rd->hdr.rpc_id = other->rpc_id;
		other->pkt_addr = other;
		other->receive = rd;
		other->src = src;
		other->dest = dest;
		other->size = size;
		if ( (mode & SIF_RPC_M_NOWAIT) != 0 )
		{
			rd->hdr.sema_id = -1;
			if ( sceSifSendCmd(SIF_CMD_RPC_RDATA, other, 64, 0, 0, 0) == 0 )
			{
				sif_rpc_packet_free((SifRpcRendPkt_t *)other);
				return -2;
			}
			return 0;
		}
		else
		{
			int sema;

			sema = sif_rpc_get_sema(&sif_rpc_data);
			rd->hdr.sema_id = sema;
			if ( sema >= 0 )
			{
				if ( sceSifSendCmd(SIF_CMD_RPC_RDATA, other, 64, 0, 0, 0) )
				{
					WaitEventFlag(sif_rpc_data.sif_rpc_sema_ef, 1 << rd->hdr.sema_id, 0, 0);
					ClearEventFlag(sif_rpc_data.sif_rpc_sema_ef, ~(1 << rd->hdr.sema_id));
					sif_rpc_free_sema(&sif_rpc_data, rd->hdr.sema_id);
					return 0;
				}
				else
				{
					sif_rpc_packet_free((SifRpcRendPkt_t *)other);
					sif_rpc_free_sema(&sif_rpc_data, rd->hdr.sema_id);
					return -2;
				}
			}
			else
			{
				sif_rpc_packet_free((SifRpcRendPkt_t *)other);
				return -4;
			}
		}
	}
	return -1;
}

static SifRpcServerData_t *sif_search_svdata(int sid, const sif_rpc_data_t *rpc_data)
{
	const SifRpcDataQueue_t *queue;
	SifRpcServerData_t *server;

	queue = rpc_data->active_queue;
	while ( queue )
	{
		server = queue->link;
		while ( server )
		{
			if ( server->sid == sid )
				return server;
			server = server->link;
		}
		queue = queue->next;
	}

	return NULL;
}

static unsigned int sif_cmd_handler_bind_alarm_retry(void *a1)
{
	if ( isceSifSendCmd(SIF_CMD_RPC_END, a1, 64, 0, 0, 0) != 0 )
		return 0;
	return 0xF000;
}

static void sif_cmd_handler_bind(SifRpcBindPkt_t *data, sif_rpc_data_t *harg)
{
	SifRpcRendPkt_t *fpacket;
	SifRpcClientData_t *client;
	SifRpcServerData_t *server;
	iop_sys_clock_t clk;

	fpacket = (SifRpcRendPkt_t *)sif_rpc_get_fpacket(harg);
	fpacket->pkt_addr = data->pkt_addr;
	client = data->client;
	fpacket->cid = SIF_CMD_RPC_BIND;
	fpacket->client = client;
	server = sif_search_svdata(data->sid, harg);
	if ( server )
	{
		fpacket->server = server;
		fpacket->buff = server->buff;
	}
	else
	{
		fpacket->server = 0;
		fpacket->buff = 0;
	}
	if ( !isceSifSendCmd(SIF_CMD_RPC_END, fpacket, 64, 0, 0, 0) )
	{
		clk.hi = 0;
		clk.lo = 0xF000;
		iSetAlarm(&clk, (unsigned int (*)(void *))sif_cmd_handler_bind_alarm_retry, fpacket);
	}
}

int sceSifBindRpc(SifRpcClientData_t *client, int rpc_number, int mode)
{
	SifRpcBindPkt_t *bind;

	client->command = 0;
	client->server = 0;
	bind = (SifRpcBindPkt_t *)sif_rpc_packet_get(&sif_rpc_data);
	if ( bind )
	{
		client->hdr.pkt_addr = bind;
		client->hdr.rpc_id = bind->rpc_id;
		bind->pkt_addr = bind;
		bind->client = client;
		bind->sid = rpc_number;
		if ( (mode & SIF_RPC_M_NOWAIT) != 0 )
		{
			client->hdr.sema_id = -1;
			if ( sceSifSendCmd(SIF_CMD_RPC_BIND, bind, 64, 0, 0, 0) == 0 )
			{
				sif_rpc_packet_free((SifRpcRendPkt_t *)bind);
				return -2;
			}
			return 0;
		}
		else
		{
			int sema;

			sema = sif_rpc_get_sema(&sif_rpc_data);
			client->hdr.sema_id = sema;
			if ( sema >= 0 )
			{
				if ( sceSifSendCmd(SIF_CMD_RPC_BIND, bind, 64, 0, 0, 0) )
				{
					WaitEventFlag(sif_rpc_data.sif_rpc_sema_ef, 1 << client->hdr.sema_id, 0, 0);
					ClearEventFlag(sif_rpc_data.sif_rpc_sema_ef, ~(1 << client->hdr.sema_id));
					sif_rpc_free_sema(&sif_rpc_data, client->hdr.sema_id);
					return 0;
				}
				else
				{
					sif_rpc_packet_free((SifRpcRendPkt_t *)bind);
					sif_rpc_free_sema(&sif_rpc_data, client->hdr.sema_id);
					return -2;
				}
			}
			else
			{
				sif_rpc_packet_free((SifRpcRendPkt_t *)bind);
				return -4;
			}
		}
	}
	return -1;
}

static void sif_cmd_handler_call(SifRpcCallPkt_t *data, sif_rpc_data_t *harg)
{
	SifRpcServerData_t *server;
	SifRpcDataQueue_t *base;

	(void)harg;

	server = data->server;
	base = server->base;
	if ( base->start )
		base->end->next = server;
	else
		base->start = server;
	base->end = server;
	server->pkt_addr = data->pkt_addr;
	server->client = data->client;
	server->rpc_number = data->rpc_number;
	server->size = data->send_size;
	server->receive = data->receive;
	server->rsize = data->recv_size;
	server->rmode = data->rmode;
	server->rid = data->rec_id;
	if ( base->thread_id >= 0 && !base->active )
		iWakeupThread(base->thread_id);
}

int sceSifCallRpc(
	SifRpcClientData_t *client,
	int rpc_number,
	int mode,
	void *send,
	int ssize,
	void *receive,
	int rsize,
	SifRpcEndFunc_t end_function,
	void *end_param)
{
	SifRpcCallPkt_t *call;

	call = (SifRpcCallPkt_t *)sif_rpc_packet_get(&sif_rpc_data);
	if ( call )
	{
		int rpc_id;

		client->hdr.pkt_addr = call;
		rpc_id = call->rpc_id;
		client->end_function = end_function;
		client->end_param = end_param;
		client->hdr.rpc_id = rpc_id;
		call->pkt_addr = call;
		call->client = client;
		call->rpc_number = rpc_number;
		call->send_size = ssize;
		call->receive = receive;
		call->recv_size = rsize;
		call->server = client->server;
		if ( (mode & SIF_RPC_M_NOWAIT) != 0 )
		{
			void *dest_extra;

			if ( end_function )
				call->rmode = 1;
			else
				call->rmode = 0;
			dest_extra = client->buff;
			client->hdr.sema_id = -1;
			if ( sceSifSendCmd(SIF_CMD_RPC_CALL, call, 64, send, dest_extra, ssize) == 0 )
			{
				sif_rpc_packet_free((SifRpcRendPkt_t *)call);
				return -2;
			}
			return 0;
		}
		else
		{
			int sema;

			call->rmode = 1;
			sema = sif_rpc_get_sema(&sif_rpc_data);
			client->hdr.sema_id = sema;
			if ( sema >= 0 )
			{
				if ( sceSifSendCmd(SIF_CMD_RPC_CALL, call, 64, send, client->buff, ssize) )
				{
					WaitEventFlag(sif_rpc_data.sif_rpc_sema_ef, 1 << client->hdr.sema_id, 0, 0);
					ClearEventFlag(sif_rpc_data.sif_rpc_sema_ef, ~(1 << client->hdr.sema_id));
					sif_rpc_free_sema(&sif_rpc_data, client->hdr.sema_id);
					return 0;
				}
				else
				{
					sif_rpc_packet_free((SifRpcRendPkt_t *)call);
					sif_rpc_free_sema(&sif_rpc_data, client->hdr.sema_id);
					return -2;
				}
			}
			else
			{
				sif_rpc_packet_free((SifRpcRendPkt_t *)call);
				return -4;
			}
		}
	}
	return -1;
}

int sceSifCheckStatRpc(SifRpcClientData_t *cd)
{
	const SifRpcPktHeader_t *pkt_addr;

	pkt_addr = (SifRpcPktHeader_t *)cd->hdr.pkt_addr;
	return cd->hdr.pkt_addr && (int)(cd->hdr.rpc_id) == pkt_addr->rpc_id && (pkt_addr->rec_id & 2) != 0;
}

SifRpcDataQueue_t *sceSifSetRpcQueue(SifRpcDataQueue_t *q, int thread_id)
{
	SifRpcDataQueue_t *queue;
	int state;

	CpuSuspendIntr(&state);
	q->thread_id = thread_id;
	q->active = 0;
	q->link = 0;
	q->start = 0;
	q->end = 0;
	q->next = 0;
	if ( sif_rpc_data.active_queue )
	{
		for ( queue = sif_rpc_data.active_queue; queue->next; queue = queue->next )
			;
		queue->next = q;
	}
	else
	{
		sif_rpc_data.active_queue = q;
	}
	return (SifRpcDataQueue_t *)CpuResumeIntr(state);
}

void sceSifRegisterRpc(
	SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf, SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *server;
	int state;

	CpuSuspendIntr(&state);
	sd->sid = sid;
	sd->func = func;
	sd->buff = buf;
	sd->next = 0;
	sd->link = 0;
	sd->cfunc = cfunc;
	sd->cbuff = cbuf;
	sd->base = qd;
	server = qd->link;
	if ( server )
	{
		for ( ; server->link; server = server->link )
			;
		server->link = sd;
	}
	else
	{
		qd->link = sd;
	}
	CpuResumeIntr(state);
}

SifRpcServerData_t *sceSifRemoveRpc(SifRpcServerData_t *sd, SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *server1;
	const SifRpcServerData_t *server2;
	int state;

	CpuSuspendIntr(&state);
	server1 = qd->link;
	if ( server1 == sd )
	{
		qd->link = server1->link;
	}
	else if ( server1 )
	{
		while ( 1 )
		{
			server2 = server1->link;
			if ( server2 == sd )
			{
				server1->link = server2->link;
				break;
			}
			server1 = server1->link;
			if ( !server2 )
			{
				break;
			}
		}
	}
	CpuResumeIntr(state);
	return server1;
}

SifRpcDataQueue_t *sceSifRemoveRpcQueue(SifRpcDataQueue_t *qd)
{
	SifRpcDataQueue_t *queue1;
	const SifRpcDataQueue_t *queue2;
	int state;

	CpuSuspendIntr(&state);
	queue1 = sif_rpc_data.active_queue;
	if ( sif_rpc_data.active_queue == qd )
	{
		sif_rpc_data.active_queue = sif_rpc_data.active_queue->next;
	}
	else if ( sif_rpc_data.active_queue )
	{
		while ( 1 )
		{
			queue2 = queue1->next;
			if ( queue2 == qd )
			{
				queue1->next = queue2->next;
				break;
			}
			queue1 = queue1->next;
			if ( !queue2 )
				break;
		}
	}
	CpuResumeIntr(state);
	return queue1;
}

SifRpcServerData_t *sceSifGetNextRequest(SifRpcDataQueue_t *qd)
{
	SifRpcServerData_t *server;
	int state;

	CpuSuspendIntr(&state);
	server = qd->start;
	if ( server )
	{
		qd->active = 1;
		qd->start = server->next;
	}
	else
	{
		qd->active = 0;
	}
	CpuResumeIntr(state);
	return server;
}

void sceSifExecRequest(SifRpcServerData_t *srv)
{
	int size_extra;
	void *rec;
	unsigned int rid;
	SifRpcRendPkt_t *fpacket2;
	SifRpcRendPkt_t *rend;
	SifRpcClientData_t *client;
	int exsz;
	void *receive;
	void *pkt_addr;
	SifDmaTransfer_t dmat2[2];
	int state[2];

	size_extra = 0;
	rec = (void *)srv->func(srv->rpc_number, srv->buff, srv->size);
	if ( rec )
		size_extra = srv->rsize;
	CpuSuspendIntr(state);
	rid = srv->rid;
	if ( (rid & 4) != 0 )
		fpacket2 = (SifRpcRendPkt_t *)sif_rpc_get_fpacket2(&sif_rpc_data, (rid >> 16) & 0xFFFF);
	else
		fpacket2 = (SifRpcRendPkt_t *)sif_rpc_get_fpacket(&sif_rpc_data);
	rend = fpacket2;
	CpuResumeIntr(state[0]);
	client = srv->client;
	rend->cid = SIF_CMD_RPC_CALL;
	rend->client = client;
	exsz = 0;
	if ( srv->rmode )
	{
		while ( !sceSifSendCmd(SIF_CMD_RPC_END, rend, 64, rec, srv->receive, size_extra) )
			;
	}
	else
	{
		int exsz1;
		int exsz2;
		SifDmaTransfer_t *dmat1;

		rend->rpc_id = 0;
		rend->rec_id = 0;
		if ( size_extra > 0 )
		{
			exsz = 1;
			dmat2[0].src = rec;
			receive = srv->receive;
			dmat2[0].size = size_extra;
			dmat2[0].attr = 0;
			dmat2[0].dest = receive;
		}
		exsz1 = exsz;
		exsz2 = exsz + 1;
		dmat1 = &dmat2[exsz1];
		dmat1->src = rend;
		pkt_addr = srv->pkt_addr;
		dmat1->size = 64;
		dmat1->attr = 0;
		dmat1->dest = pkt_addr;
		while ( 1 )
		{
			int dmar;
			int busywait;

			CpuSuspendIntr(state);
			dmar = sceSifSetDma(dmat2, exsz2);
			CpuResumeIntr(state[0]);
			if ( dmar )
				break;
			busywait = 0xFFFE;
			while ( busywait-- != -1 )
				;
		}
	}
}

void sceSifRpcLoop(SifRpcDataQueue_t *qd)
{
	while ( 1 )
	{
		SifRpcServerData_t *server;

		server = sceSifGetNextRequest(qd);
		if ( server )
			sceSifExecRequest(server);
		else
			SleepThread();
	}
}
