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
#include <kerr.h>
#include <msifrpc.h>

#ifdef _IOP
IRX_ID("IOP_MSIF_rpc_interface", 2, 7);
#endif
// Based on the module from SCE SDK 3.1.0.

typedef struct _sifm_receive_data
{
	sceSifMRpcData rpcd;
	void *src;
	void *dest;
	int size;
} sceSifMReceiveData;

struct _sifm_smsg_data
{
	void *cl_paddr;
	sceSifMRpcData *client;
	void *local;
	sceSifMServeEntry *sentry;
	unsigned int buffersize;
	unsigned int stacksize;
	int prio;
	sceSifMServeData *sd;
};

typedef struct _sifm_queue_data
{
	int key;
	int active;
	int sleep;
	struct _sifm_serve_data *link;
	struct _sifm_serve_data *start;
	struct _sifm_serve_data *end;
	struct _sifm_queue_data *next;
} sceSifMQueueData;

struct msif_data
{
	int field_0;
	int m_rdata_table;
	int m_rdata_table_len;
	int m_client_table;
	int m_client_table_len;
	int m_rdata_table_idx;
	sceSifMQueueData *m_active_queue;
	int field_1C[2];
	sceSifMServeEntry *g_mserv_entries_ll;
};

struct msif_msgbox_msg2
{
	void *m_pkt_addr;
	sceSifMClientData *m_eebuf_cd;
	struct msif_data *m_msif_data;
	sceSifMServeEntry *m_mserve_entry;
	int m_buffersize;
	int m_stacksize;
	int m_priority;
	sceSifMServeData *m_sd;
};

struct msif_msgbox_msg
{
	int m_x1;
	char m_probunused_unkx21;
	char m_x22;
	char m_x23;
	char m_x24;
	int m_in_cmd;
	struct msif_msgbox_msg2 m_msg2;
};

typedef struct t_SifMRpcRendPkt
{
	struct t_SifCmdHeader sifcmd;
	int rec_id;
	void *pkt_addr;
	int rpc_id;
	sceSifMClientData *cd;
	u32 cid;
	sceSifMServeData *sd;
	void *buf;
	void *cbuf;
} SifMRpcRendPkt_t;

typedef struct t_SifMRpcBindPkt
{
	struct t_SifCmdHeader sifcmd;
	int rec_id;
	void *pkt_addr;
	int rpc_id;
	sceSifMClientData *cd;
	int sid;
	int m_eebuf_or_threadstate;
	int m_eeserver;
	int m_toee_unkxb;
} SifMRpcBindPkt_t;

typedef struct t_SifMRpcCallPkt
{
	struct t_SifCmdHeader sifcmd;
	int rec_id;
	void *pkt_addr;
	int rpc_id;
	sceSifMClientData *cd;
	int rpc_number;
	sceSifMServeData *sd;
	void *recvbuf;
	int recv_size;
	int rmode;
} SifMRpcCallPkt_t;

struct msif_cmd_bindrpcparam_80000019
{
	int m_x01;
	int m_x02;
	int m_x03;
	int m_x04;
	int m_x05;
	void *m_pkt_addr;
	int m_x07;
	sceSifMClientData *m_cd;
	int m_fromee_cmd;
	int m_buffersize;
	int m_stacksize;
	int m_priority;
	int m_x13;
	int m_x14;
	int m_x15;
	int m_x16;
};

struct msif_cmd_unbindrpc_8000001D
{
	int m_x01;
	int m_x02;
	int m_x03;
	int m_x04;
	int m_x05;
	void *m_pkt_addr;
	int m_x07;
	sceSifMClientData *m_cd;
	int m_command;
	sceSifMServeData *m_sd;
	int m_x11;
	int m_x12;
	int m_x13;
	int m_x14;
	int m_x15;
	int m_x16;
};

struct msif_cmd_callrpc_8000001A
{
	int m_x01;
	int m_x02;
	int m_x03;
	int m_x04;
	int m_rid;
	void *m_paddr;
	int m_x07;
	sceSifMClientData *m_cd;
	int m_fno;
	int m_size;
	void *m_receive;
	int m_rsize;
	int m_rmode;
	sceSifMServeData *m_sd;
	int m_x15;
	int m_x16;
};

static void sif_cmdh_bindrpcparam_80000019(struct msif_cmd_bindrpcparam_80000019 *data, struct msif_data *harg);
static void sif_cmdh_unbindrpc_8000001D(struct msif_cmd_unbindrpc_8000001D *data, struct msif_data *harg);
static void sif_cmdh_callrpc_8000001A(struct msif_cmd_callrpc_8000001A *data, struct msif_data *harg);

extern struct irx_export_table _exp_msifrpc;
// Unofficial: move to bss
static int g_first_inited;
static int g_pkt_table[512];
static int g_client_table[512];
static struct msif_data g_msif_data;

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;
	printf("Multi-thread available sifrpc module...\n");
	return RegisterLibraryEntries(&_exp_msifrpc) ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END;
}

void sceSifMInitRpc(unsigned int mode)
{
	int state;

	(void)mode;
	CpuSuspendIntr(&state);
	if ( g_first_inited )
	{
		CpuResumeIntr(state);
		while ( !sceSifGetSreg(1) )
		{
			int delayth_1;

			delayth_1 = DelayThread(10000);
			if ( delayth_1 )
				printf("return value of DelayThread() is %d\n", delayth_1);
		}
	}
	else
	{
		g_first_inited = 1;
		g_msif_data.m_rdata_table = (int)g_pkt_table;
		g_msif_data.m_rdata_table_len = 32;
		g_msif_data.m_client_table = (int)g_client_table;
		g_msif_data.m_client_table_len = 32;
		g_msif_data.m_rdata_table_idx = 0;
		g_msif_data.field_0 = 1;
		g_msif_data.g_mserv_entries_ll = 0;
		sceSifAddCmdHandler(0x80000019, (SifCmdHandler_t)sif_cmdh_bindrpcparam_80000019, &g_msif_data);
		sceSifAddCmdHandler(0x8000001A, (SifCmdHandler_t)sif_cmdh_callrpc_8000001A, &g_msif_data);
		sceSifAddCmdHandler(0x8000001D, (SifCmdHandler_t)sif_cmdh_unbindrpc_8000001D, &g_msif_data);
		CpuResumeIntr(state);
		g_pkt_table[4] = 1;
		g_pkt_table[5] = 1;
		while ( !sceSifSendCmd(0x80000001, g_pkt_table, 24, 0, 0, 0) )
			DelayThread(0xF000);
		while ( !sceSifGetSreg(1) )
		{
			int delayth_2;

			delayth_2 = DelayThread(10000);
			if ( delayth_2 )
				printf("return value of DelayThread() is %d\n", delayth_2);
		}
	}
}

static int sif_mrpc_get_fpacket(struct msif_data *rpc_data)
{
	int m_rdata_table_idx;
	int m_rdata_table_len;
	int index_calc;

	m_rdata_table_idx = rpc_data->m_rdata_table_idx;
	m_rdata_table_len = rpc_data->m_rdata_table_len;
	index_calc = m_rdata_table_idx % m_rdata_table_len;
	if ( m_rdata_table_len == -1 && (unsigned int)m_rdata_table_idx == 0x80000000 )
		__builtin_trap();
	rpc_data->m_rdata_table_idx = index_calc + 1;
	return rpc_data->m_rdata_table + (index_calc << 6);
}

static int sif_mrpc_get_fpacket2(struct msif_data *rpc_data, int rid)
{
	return (rid >= 0 && rid < rpc_data->m_client_table_len) ? (rpc_data->m_client_table + (rid << 6)) :
																														sif_mrpc_get_fpacket(rpc_data);
}

static sceSifMServeEntry *do_get_mserve_entry(int cmd, struct msif_data *msd)
{
	sceSifMServeEntry *g_mserv_entries_ll;

	for ( g_mserv_entries_ll = msd->g_mserv_entries_ll; g_mserv_entries_ll && g_mserv_entries_ll->command != cmd;
				g_mserv_entries_ll = g_mserv_entries_ll->next )
		;
	return g_mserv_entries_ll;
}

static unsigned int alarm_cb_cmd_80000018_1(void *pkt)
{
	return isceSifSendCmd(0x80000018, pkt, 64, 0, 0, 0) ? 0 : 0xF000;
}

static void sif_cmdh_bindrpcparam_80000019(struct msif_cmd_bindrpcparam_80000019 *data, struct msif_data *harg)
{
	sceSifMServeEntry *mserve_entry;
	SifMRpcBindPkt_t *fpacket;
	iop_sys_clock_t sysclks;

	mserve_entry = do_get_mserve_entry(data->m_fromee_cmd, harg);
	if ( mserve_entry )
	{
		struct msif_msgbox_msg *msgdat;

		msgdat = (struct msif_msgbox_msg *)AllocSysMemory(0, sizeof(struct msif_msgbox_msg), 0);
		if ( !msgdat )
		{
			printf("AllocSysMemory() failed.\n");
			// Unofficial: early return
			return;
		}
		msgdat->m_probunused_unkx21 = 0;
		msgdat->m_in_cmd = 0x80000019;
		msgdat->m_msg2.m_pkt_addr = data->m_pkt_addr;
		msgdat->m_msg2.m_msif_data = harg;
		msgdat->m_msg2.m_mserve_entry = mserve_entry;
		msgdat->m_msg2.m_eebuf_cd = data->m_cd;
		msgdat->m_msg2.m_buffersize = data->m_buffersize;
		msgdat->m_msg2.m_stacksize = data->m_stacksize;
		msgdat->m_msg2.m_priority = data->m_priority;
		iSendMbx(mserve_entry->mbxid, msgdat);
		return;
	}
	fpacket = (SifMRpcBindPkt_t *)sif_mrpc_get_fpacket(harg);
	fpacket->pkt_addr = data->m_pkt_addr;
	fpacket->sid = 0x80000019;
	fpacket->m_eebuf_or_threadstate = 0;
	fpacket->m_eeserver = 0;
	fpacket->m_toee_unkxb = 0;
	fpacket->cd = data->m_cd;
	if ( !isceSifSendCmd(0x80000018, fpacket, 64, 0, 0, 0) )
	{
		sysclks.hi = 0;
		sysclks.lo = 0xF000;
		iSetAlarm(&sysclks, alarm_cb_cmd_80000018_1, fpacket);
	}
}

static unsigned int alarm_cb_cmd_80000018_2(void *pkt)
{
	return isceSifSendCmd(0x80000018, pkt, 64, 0, 0, 0) ? 0 : 0xF000;
}

static void sif_cmdh_unbindrpc_8000001D(struct msif_cmd_unbindrpc_8000001D *data, struct msif_data *harg)
{
	sceSifMServeEntry *mserve_entry;
	int threadstate_tmp;
	SifMRpcBindPkt_t *fpacket;
	iop_sys_clock_t alarmdat;

	mserve_entry = do_get_mserve_entry(data->m_command, harg);
	if ( !mserve_entry || (data->m_sd->sentry != mserve_entry) )
	{
		threadstate_tmp = 3;
	}
	else if ( !data->m_sd->base->sleep )
	{
		threadstate_tmp = 4;
	}
	else
	{
		struct msif_msgbox_msg *msgboxdat;

		msgboxdat = (struct msif_msgbox_msg *)AllocSysMemory(0, sizeof(struct msif_msgbox_msg), 0);
		if ( !msgboxdat )
		{
			printf("AllocSysMemory() failed.\n");
			// Unofficial: early return
			return;
		}
		msgboxdat->m_probunused_unkx21 = 0;
		msgboxdat->m_in_cmd = 0x8000001D;
		msgboxdat->m_msg2.m_pkt_addr = data->m_pkt_addr;
		msgboxdat->m_msg2.m_msif_data = harg;
		msgboxdat->m_msg2.m_mserve_entry = mserve_entry;
		msgboxdat->m_msg2.m_sd = data->m_sd;
		msgboxdat->m_msg2.m_eebuf_cd = data->m_cd;
		iSendMbx(mserve_entry->mbxid, msgboxdat);
		return;
	}
	fpacket = (SifMRpcBindPkt_t *)sif_mrpc_get_fpacket(harg);
	fpacket->pkt_addr = data->m_pkt_addr;
	fpacket->sid = 0x8000001D;
	fpacket->m_eebuf_or_threadstate = threadstate_tmp;
	fpacket->m_eeserver = 0;
	fpacket->m_toee_unkxb = 0;
	fpacket->cd = data->m_cd;
	if ( !isceSifSendCmd(0x80000018, fpacket, 64, 0, 0, 0) )
	{
		alarmdat.hi = 0;
		alarmdat.lo = 0xF000;
		iSetAlarm(&alarmdat, alarm_cb_cmd_80000018_2, fpacket);
	}
}

static void sif_cmdh_callrpc_8000001A(struct msif_cmd_callrpc_8000001A *data, struct msif_data *harg)
{
	(void)harg;
	if ( data->m_sd->base->start )
		data->m_sd->base->end->next = data->m_sd;
	else
		data->m_sd->base->start = data->m_sd;
	data->m_sd->base->end = data->m_sd;
	data->m_sd->paddr = data->m_paddr;
	data->m_sd->client = data->m_cd;
	data->m_sd->fno = data->m_fno;
	data->m_sd->size = data->m_size;
	data->m_sd->receive = data->m_receive;
	data->m_sd->rsize = data->m_rsize;
	data->m_sd->rmode = data->m_rmode;
	data->m_sd->rid = data->m_rid;
	if ( data->m_sd->base->key >= 0 && !data->m_sd->base->active )
		iWakeupThread(data->m_sd->base->key);
}

static void do_set_rpc_queue(sceSifMQueueData *qd, int key)
{
	sceSifMQueueData *i;
	int state;

	CpuSuspendIntr(&state);
	qd->key = key;
	qd->active = 0;
	qd->link = 0;
	qd->start = 0;
	qd->end = 0;
	qd->next = 0;
	if ( g_msif_data.m_active_queue )
	{
		for ( i = g_msif_data.m_active_queue; i->next; i = i->next )
			;
		i->next = qd;
	}
	else
	{
		g_msif_data.m_active_queue = qd;
	}
	CpuResumeIntr(state);
}

// Removed unused func

static void do_msif_remove_rpc(sceSifMServeData *sd)
{
	sceSifMServeData *server1;
	int state;

	CpuSuspendIntr(&state);
	server1 = sd->sentry->serve_list;
	if ( server1 == sd )
	{
		sd->sentry->serve_list = server1->next;
	}
	else
	{
		const sceSifMServeData *server2;

		server2 = server1;
		while ( server2 )
		{
			server2 = server1->next;
			if ( server2 == sd )
			{
				server1->next = sd->next;
				break;
			}
			server1 = server1->next;
		}
	}
	CpuResumeIntr(state);
}

static void do_sif_remove_rpc_queue(const sceSifMQueueData *qd)
{
	sceSifMQueueData *queue1;
	sceSifMQueueData *queue2;
	int state;

	CpuSuspendIntr(&state);
	queue1 = g_msif_data.m_active_queue;
	if ( queue1 == qd )
	{
		g_msif_data.m_active_queue = queue1->next;
	}
	else
	{
		queue2 = queue1;
		while ( queue2 )
		{
			queue2 = queue1->next;
			if ( queue2 == qd )
			{
				queue1->next = queue2->next;
				break;
			}
			queue1 = queue1->next;
		}
	}
	CpuResumeIntr(state);
}

static struct _sifm_serve_data *do_msif_get_next_request(sceSifMQueueData *qd)
{
	sceSifMServeData *start;
	int state;

	CpuSuspendIntr(&state);
	start = qd->start;
	qd->active = start ? 1 : 0;
	if ( start )
	{
		qd->start = start->next;
	}
	CpuResumeIntr(state);
	return start;
}

static void do_msif_exec_request(sceSifMServeData *sd)
{
	int size_extra;
	void *sentry_ret;
	SifMRpcRendPkt_t *fpacket2;
	int adddmat;
	int state;

	size_extra = 0;
	sentry_ret = sd->sentry->func(sd->fno, sd->func_buff, sd->size);
	if ( sentry_ret )
		size_extra = sd->rsize;
	CpuSuspendIntr(&state);
	fpacket2 = (sd->rid & 4) ? (SifMRpcRendPkt_t *)sif_mrpc_get_fpacket2(&g_msif_data, (sd->rid >> 16) & 0xFFFF) :
														 (SifMRpcRendPkt_t *)sif_mrpc_get_fpacket(&g_msif_data);
	CpuResumeIntr(state);
	fpacket2->cid = 0x8000001A;
	fpacket2->cd = sd->client;
	adddmat = 0;
	if ( sd->rmode )
	{
		while ( !sceSifSendCmd(0x80000018, fpacket2, 64, sentry_ret, sd->receive, size_extra) )
			;
	}
	else
	{
		SifDmaTransfer_t dmat[2];

		fpacket2->rpc_id = 0;
		fpacket2->rec_id = 0;
		if ( size_extra > 0 )
		{
			adddmat = 1;
			dmat[0].src = sentry_ret;
			dmat[0].size = size_extra;
			dmat[0].attr = 0;
			dmat[0].dest = sd->receive;
		}
		dmat[adddmat].src = fpacket2;
		dmat[adddmat].size = 64;
		dmat[adddmat].attr = 0;
		dmat[adddmat].dest = sd->paddr;
		while ( 1 )
		{
			int dmaid;
			int i;

			CpuSuspendIntr(&state);
			dmaid = sceSifSetDma(dmat, adddmat + 1);
			CpuResumeIntr(state);
			if ( dmaid )
				break;
			for ( i = 0xFFFF; i != 0; i -= 1 )
				;
		}
	}
}

static void do_msif_rpc_loop(sceSifMQueueData *qd)
{
	while ( 1 )
	{
		sceSifMServeData *next_request;

		next_request = do_msif_get_next_request(qd);
		if ( next_request )
		{
			do_msif_exec_request(next_request);
		}
		else
		{
			qd->sleep = 1;
			SleepThread();
			qd->sleep = 0;
		}
	}
}

static void thread_proc_80000019(void *userdata)
{
	sceSifMServeData *sd;
	sceSifMQueueData *qd;
	void *funcbuf;
	SifMRpcCallPkt_t *fpacket;
	int state;
	struct msif_msgbox_msg *msgboxdat;

	msgboxdat = (struct msif_msgbox_msg *)userdata;
	CpuSuspendIntr(&state);
	sd = (sceSifMServeData *)AllocSysMemory(0, sizeof(sceSifMServeData), 0);
	if ( !sd )
		printf("AllocSysMemory() failed.\n");
	qd = (sceSifMQueueData *)AllocSysMemory(0, sizeof(sceSifMQueueData), 0);
	if ( !qd )
		printf("AllocSysMemory() failed.\n");
	funcbuf = AllocSysMemory(0, msgboxdat->m_msg2.m_buffersize, 0);
	if ( !funcbuf )
		printf("AllocSysMemory() failed.\n");
	CpuResumeIntr(state);
	do_set_rpc_queue(qd, GetThreadId());
	sd->func_buff = funcbuf;
	sd->base = qd;
	sd->next = 0;
	sd->sentry = msgboxdat->m_msg2.m_mserve_entry;
	qd->link = sd;
	qd->sleep = 0;
	CpuSuspendIntr(&state);
	fpacket = (SifMRpcCallPkt_t *)sif_mrpc_get_fpacket(msgboxdat->m_msg2.m_msif_data);
	CpuResumeIntr(state);
	fpacket->pkt_addr = msgboxdat->m_msg2.m_pkt_addr;
	fpacket->rpc_number = 0x80000019;
	fpacket->sd = sd;
	fpacket->recvbuf = funcbuf;
	fpacket->recv_size = 0;
	fpacket->cd = msgboxdat->m_msg2.m_eebuf_cd;
	while ( !sceSifSendCmd(0x80000018, fpacket, 64, 0, 0, 0) )
		DelayThread(0xF000);
	CpuSuspendIntr(&state);
	FreeSysMemory(msgboxdat);
	CpuResumeIntr(state);
	do_msif_rpc_loop(qd);
}

// Removed unused func

void sceSifMEntryLoop(sceSifMServeEntry *se, int request, sceSifMRpcFunc func, sceSifMRpcFunc cfunc)
{
	sceSifMServeEntry *g_mserv_entries_ll;
	int thid_1;
	int termthread_1;
	int delthread_1;
	SifMRpcBindPkt_t *fpacket2;
	iop_mbx_t mbxparam;
	iop_thread_t thparam_1;
	iop_thread_info_t thinfo;
	int state;
	struct msif_msgbox_msg *arg;

	ReferThreadStatus(0, &thinfo);
	ChangeThreadPriority(0, 16);
	se->command = request;
	se->func = func;
	se->cfunc = cfunc;
	se->next = 0;
	se->serve_list = 0;
	mbxparam.attr = MBA_THFIFO;
	while ( 1 )
	{
		se->mbxid = CreateMbx(&mbxparam);
		if ( se->mbxid )
			break;
		DelayThread(0xF000);
	}
	CpuSuspendIntr(&state);
	g_mserv_entries_ll = g_msif_data.g_mserv_entries_ll;
	if ( g_mserv_entries_ll )
	{
		while ( g_mserv_entries_ll->next )
			g_mserv_entries_ll = g_mserv_entries_ll->next;
		g_mserv_entries_ll->next = se;
	}
	else
	{
		g_msif_data.g_mserv_entries_ll = se;
	}
	CpuResumeIntr(state);
	while ( 1 )
	{
		int mbxrecv;

		mbxrecv = ReceiveMbx((void **)&arg, se->mbxid);
		if ( mbxrecv )
		{
			if ( mbxrecv != KE_WAIT_DELETE )
			{
				printf("ReceiveMbx() failed.\n");
			}
			else
			{
				printf("msifrpc: quit\n");
				ChangeThreadPriority(0, thinfo.currentPriority);
				break;
			}
		}
		switch ( arg->m_in_cmd )
		{
			case 0x80000019:
				thparam_1.thread = thread_proc_80000019;
				thparam_1.attr = TH_C;
				thparam_1.stacksize = arg->m_msg2.m_stacksize;
				thparam_1.priority = arg->m_msg2.m_priority;
				thid_1 = CreateThread(&thparam_1);
				if ( thid_1 < 0 )
					printf("StartThread() failed.\n");
				else
					StartThread(thid_1, arg);
				break;
			case 0x8000001D:
				termthread_1 = TerminateThread(arg->m_msg2.m_sd->base->key);
				if ( termthread_1 )
					Kprintf("TerminateThread(): ret = %d\n", termthread_1);
				delthread_1 = DeleteThread(arg->m_msg2.m_sd->base->key);
				if ( delthread_1 )
				{
					if ( delthread_1 == KE_NOT_DORMANT )
						delthread_1 = 2;
					else
						Kprintf("DeleteThread(): ret = %d\n", delthread_1);
				}
				else
				{
					delthread_1 = 1;
					do_sif_remove_rpc_queue(arg->m_msg2.m_sd->base);
					CpuSuspendIntr(&state);
					FreeSysMemory(arg->m_msg2.m_sd->base);
					FreeSysMemory(arg->m_msg2.m_sd->func_buff);
					CpuResumeIntr(state);
					do_msif_remove_rpc(arg->m_msg2.m_sd);
					CpuSuspendIntr(&state);
					FreeSysMemory(arg->m_msg2.m_sd);
					CpuResumeIntr(state);
				}
				CpuSuspendIntr(&state);
				fpacket2 =
					(arg->m_msg2.m_sd->rid & 4) ?
						(SifMRpcBindPkt_t *)sif_mrpc_get_fpacket2(arg->m_msg2.m_msif_data, (arg->m_msg2.m_sd->rid >> 16) & 0xFFFF) :
						(SifMRpcBindPkt_t *)sif_mrpc_get_fpacket(arg->m_msg2.m_msif_data);
				CpuResumeIntr(state);
				fpacket2->pkt_addr = arg->m_msg2.m_pkt_addr;
				fpacket2->sid = 0x8000001D;
				fpacket2->m_eebuf_or_threadstate = delthread_1;
				fpacket2->m_eeserver = 0;
				fpacket2->m_toee_unkxb = 0;
				fpacket2->cd = arg->m_msg2.m_eebuf_cd;
				while ( !sceSifSendCmd(0x80000018, fpacket2, 64, 0, 0, 0) )
					DelayThread(0xF000);
				CpuSuspendIntr(&state);
				FreeSysMemory(arg);
				CpuResumeIntr(state);
				break;
			default:
				break;
		}
	}
}

int sceSifMTermRpc(int request, int flags)
{
	sceSifMServeEntry *cur_entry;
	sceSifMServeEntry *tmp_entry;
	int state;

	(void)flags;
	CpuSuspendIntr(&state);
	cur_entry = g_msif_data.g_mserv_entries_ll;
	tmp_entry = 0;
	while ( cur_entry && cur_entry->command != request )
	{
		tmp_entry = cur_entry;
		cur_entry = cur_entry->next;
	}
	if ( cur_entry )
	{
		if ( tmp_entry )
			tmp_entry->next = cur_entry->next;
		else
			g_msif_data.g_mserv_entries_ll = cur_entry->next;
	}
	CpuResumeIntr(state);
	if ( cur_entry )
	{
		DeleteMbx(cur_entry->mbxid);
		DelayThread(100000);
	}
	return 0;
}
