/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SIO2 logging utility.
 */

#include <irx.h>
#include <defs.h>
#include <types.h>

#include <loadcore.h>
#include <intrman.h>
#include <dmacman.h>
#include <stdio.h>
#include <thbase.h>
#include <thevent.h>
#include <ioman.h>

#include "xsio2man.h"

#ifdef SIO2LOG
	#include "log.h"
#endif

#ifdef SIO2LOG
	IRX_ID("sio2man_logger", 2, 1);
#else
#ifndef SIO2MAN_V2
	IRX_ID("sio2man", 2, 1);
#else
	IRX_ID("sio2man", 2, 4);
#endif
#endif

extern struct irx_export_table _exp_sio2man;

#define SIO2_REG_BASE		0xbf808200
#define SIO2_REG_PORT0_CTRL1	0xbf808240
#define SIO2_REG_PORT0_CTRL2	0xbf808244
#define SIO2_REG_DATA_OUT	0xbf808260
#define SIO2_REG_DATA_IN	0xbf808264
#define SIO2_REG_CTRL		0xbf808268
#define SIO2_REG_STAT6C		0xbf80826c
#define SIO2_REG_STAT70		0xbf808270
#define SIO2_REG_STAT74		0xbf808274
#define SIO2_REG_UNKN78		0xbf808278
#define SIO2_REG_UNKN7C		0xbf80827c
#define SIO2_REG_STAT		0xbf808280

/* Event flags */
#define EF_PAD_TRANSFER_INIT	0x00000001
#define EF_PAD_TRANSFER_READY	0x00000002
#define EF_MC_TRANSFER_INIT	0x00000004
#define EF_MC_TRANSFER_READY	0x00000008
#define EF_MTAP_TRANSFER_INIT	0x00000010
#define EF_MTAP_TRANSFER_READY	0x00000020
#ifndef SIO2MAN_V2
	#define EF_TRANSFER_START	0x00000040
	#define EF_TRANSFER_FINISH	0x00000080
	#define EF_TRANSFER_RESET	0x00000100
	#define EF_SIO2_INTR_COMPLETE	0x00000200
#else
	#define EF_RM_TRANSFER_INIT	0x00000040
	#define EF_RM_TRANSFER_READY	0x00000080
	#define EF_UNK_TRANSFER_INIT	0x00000100
	#define EF_UNK_TRANSFER_READY	0x00000200
	#define EF_TRANSFER_START	0x00000400
	#define EF_TRANSFER_FINISH	0x00000800
	#define EF_TRANSFER_RESET	0x00001000
	#define EF_SIO2_INTR_COMPLETE	0x00002000
#endif

#define EPRINTF(format, args...) printf("%s: " format, _irx_id.n , ## args)

int init = 0;
int event_flag = -1;
int thid = -1;
sio2_transfer_data_t *transfer_data = NULL;
int (*mtap_change_slot_cb)(s32 *) = NULL;
int (*mtap_get_slot_max_cb)(int) = NULL;
int (*mtap_get_slot_max2_cb)(int) = NULL;
void (*mtap_update_slots_cb)(void) = NULL;

int sio2_intr_handler(void *arg)
{
	int ef = *(int *)arg;

	sio2_stat_set(sio2_stat_get());

	iSetEventFlag(ef, EF_SIO2_INTR_COMPLETE);

	return 1;
}

void send_td(sio2_transfer_data_t *td)
{
	int i;

#ifdef SIO2LOG
	log_default(LOG_TRS);
#endif

	for (i = 0; i < 4; i++) {
		sio2_portN_ctrl1_set(i, td->port_ctrl1[i]);
		sio2_portN_ctrl2_set(i, td->port_ctrl2[i]);
	}

#ifdef SIO2LOG
	log_portdata(td->port_ctrl1, td->port_ctrl2);
#endif

	for (i = 0; i < 16; i++)
		sio2_regN_set(i, td->regdata[i]);

#ifdef SIO2LOG
	log_regdata(td->regdata);
#endif

	if (td->in_size) {
		for (i = 0; i < td->in_size; i++)
			sio2_data_out(td->in[i]);
#ifdef SIO2LOG
		log_data(LOG_TRS_DATA, td->in, td->in_size);
#endif
	}

	if (td->in_dma.addr) {
		dmac_request(IOP_DMAC_SIO2in, td->in_dma.addr, td->in_dma.size,
				td->in_dma.count, DMAC_FROM_MEM);
		dmac_transfer(IOP_DMAC_SIO2in);

#ifdef SIO2LOG
		log_dma(LOG_TRS_DMA_IN, &td->in_dma);
#endif
	}

	if (td->out_dma.addr) {
		dmac_request(IOP_DMAC_SIO2out, td->out_dma.addr, td->out_dma.size,
				td->out_dma.count, DMAC_TO_MEM);
		dmac_transfer(IOP_DMAC_SIO2out);

#ifdef SIO2LOG
		log_dma(LOG_TRS_DMA_OUT, &td->out_dma);
#endif
	}
}

void recv_td(sio2_transfer_data_t *td)
{
	int i;
#ifdef SIO2LOG
	log_default(LOG_TRR);
#endif
	td->stat6c = sio2_stat6c_get();
	td->stat70 = sio2_stat70_get();
	td->stat74 = sio2_stat74_get();
#ifdef SIO2LOG
	log_stat(td->stat6c, td->stat70, td->stat74);
#endif
	if (td->out_size) {
		for (i = 0; i < td->out_size; i++)
			td->out[i] = sio2_data_in();
#ifdef SIO2LOG
		log_data(LOG_TRR_DATA, td->out, td->out_size);
#endif
	}
}

void main_thread(void *unused)
{
	u32 resbits[4];

	while (1) {
	#ifdef SIO2LOG
		log_flush(0);
	#endif
		WaitEventFlag(event_flag, EF_PAD_TRANSFER_INIT |
				EF_MC_TRANSFER_INIT | EF_MTAP_TRANSFER_INIT
#ifdef SIO2MAN_V2
				| EF_RM_TRANSFER_INIT | EF_UNK_TRANSFER_INIT
#endif
				, 1, resbits);

		if (resbits[0] & EF_PAD_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_PAD_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_PAD_TRANSFER_READY);
#ifdef SIO2LOG
			log_default(LOG_PAD_READY);
#endif
		} else if (resbits[0] & EF_MC_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_MC_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_MC_TRANSFER_READY);
#ifdef SIO2LOG
			log_default(LOG_MC_READY);
#endif
		} else if (resbits[0] & EF_MTAP_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_MTAP_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_MTAP_TRANSFER_READY);
#ifdef SIO2LOG
			log_default(LOG_MTAP_READY);
#endif
#ifdef SIO2MAN_V2
		} else if (resbits[0] & EF_RM_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_RM_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_RM_TRANSFER_READY);
#ifdef SIO2LOG
			log_default(LOG_RM_READY);
#endif
		} else if (resbits[0] & EF_UNK_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_UNK_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_UNK_TRANSFER_READY);
#ifdef SIO2LOG
			log_default(LOG_UNK_READY);
#endif
#endif
		} else {
			EPRINTF("Unknown event %08lx. Exiting.\n", resbits[0]);
			return;
		}

transfer_loop:
		WaitEventFlag(event_flag, EF_TRANSFER_START | EF_TRANSFER_RESET, 1, resbits);

		if (resbits[0] & EF_TRANSFER_RESET) {
			ClearEventFlag(event_flag, ~EF_TRANSFER_RESET);
#ifdef SIO2LOG
			log_default(LOG_RESET);
#endif
			continue;
		}

		ClearEventFlag(event_flag, ~EF_TRANSFER_START);

		sio2_ctrl_set(sio2_ctrl_get() | 0xc);
		send_td(transfer_data);
		sio2_ctrl_set(sio2_ctrl_get() | 1);

		WaitEventFlag(event_flag, EF_SIO2_INTR_COMPLETE, 0, NULL);
		ClearEventFlag(event_flag, ~EF_SIO2_INTR_COMPLETE);

		recv_td(transfer_data);
		SetEventFlag(event_flag, EF_TRANSFER_FINISH);

		/* Bah... this is needed to get the initial dump from XMCMAN,
		   but it will kill the IOP when XPADMAN is spamming...

		   TODO
		   I guess the correct solution is to do all logging in a
		   dedicated thread.  */
//		log_flush(1);

		goto transfer_loop;
	}
}

int create_main_thread(void)
{
	iop_thread_t thread;

	thread.attr = 0x2000000;
	thread.option = 0;
	thread.thread = main_thread;
	thread.stacksize = 0x8000;
	thread.priority = 0x18;
	return CreateThread(&thread);
}

int create_event_flag(void)
{
	iop_event_t event;

	event.attr = 2;
	event.option = 0;
	event.bits = 0;
	return CreateEventFlag(&event);
}

void shutdown(void)
{
	int state;
#ifdef SIO2LOG
	log_flush(1);
#endif
	CpuSuspendIntr(&state);
	DisableIntr(IOP_IRQ_SIO2, 0);
	ReleaseIntrHandler(IOP_IRQ_SIO2);
	CpuResumeIntr(state);

	dmac_disable(IOP_DMAC_SIO2in);
	dmac_disable(IOP_DMAC_SIO2out);
}

int _start(int argc, const char **argv)
{
	int state;

	shutdown();

	if (RegisterLibraryEntries(&_exp_sio2man) != 0)
		return 1;

	if (init)
		return 1;

	init = 1;

	sio2_ctrl_set(0x3bc);

	mtap_change_slot_cb = NULL;  mtap_get_slot_max_cb = NULL;  mtap_get_slot_max2_cb = NULL; mtap_update_slots_cb = NULL;
	event_flag = create_event_flag();
	thid = create_main_thread();

	CpuSuspendIntr(&state);
	RegisterIntrHandler(IOP_IRQ_SIO2, 1, sio2_intr_handler, &event_flag);
	EnableIntr(IOP_IRQ_SIO2);
	CpuResumeIntr(state);

	dmac_ch_set_dpcr(IOP_DMAC_SIO2in, 3);
	dmac_ch_set_dpcr(IOP_DMAC_SIO2out, 3);
	dmac_enable(IOP_DMAC_SIO2in);
	dmac_enable(IOP_DMAC_SIO2out);

	StartThread(thid, NULL);
#ifdef SIO2LOG
	EPRINTF("Logging started.\n");
#endif
	return 0;
}

void sio2_pad_transfer_init(void)
{
	SetEventFlag(event_flag, EF_PAD_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_PAD_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_PAD_TRANSFER_READY);
}

void sio2_mc_transfer_init(void)
{
	SetEventFlag(event_flag, EF_MC_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_MC_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_MC_TRANSFER_READY);
}

void sio2_mtap_transfer_init(void)
{
	SetEventFlag(event_flag, EF_MTAP_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_MTAP_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_MTAP_TRANSFER_READY);
}

int sio2_transfer(sio2_transfer_data_t *td)
{
	transfer_data = td;
	SetEventFlag(event_flag, EF_TRANSFER_START);

	WaitEventFlag(event_flag, EF_TRANSFER_FINISH, 0, NULL);
	ClearEventFlag(event_flag, ~EF_TRANSFER_FINISH);
	return 1;
}

#ifdef SIO2MAN_V2
void sio2_rm_transfer_init(void)
{
	SetEventFlag(event_flag, EF_RM_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_RM_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_RM_TRANSFER_READY);
}

void sio2_unk_transfer_init(void)
{
	SetEventFlag(event_flag, EF_UNK_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_UNK_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_UNK_TRANSFER_READY);
}
#endif

void sio2_transfer_reset(void)
{
	SetEventFlag(event_flag, EF_TRANSFER_RESET);
}

int sio2_mtap_change_slot(s32 *status)
{
	int i, ret = 1;

	if (mtap_change_slot_cb)
		return mtap_change_slot_cb(status);

	for (i = 0; i < 4; i++, status++) {
		if ((*status + 1) < 2)
			status[4] = 1;
		else {
			status[4] = 0;
			ret = 0;
		}
	}

	return ret;
}

int sio2_mtap_get_slot_max(int port)
{
	if (mtap_get_slot_max_cb)
		return mtap_get_slot_max_cb(port);

	return 1;
}

int sio2_mtap_get_slot_max2(int port)
{
	if (mtap_get_slot_max2_cb)
		return mtap_get_slot_max2_cb(port);

	return 1;
}

void sio2_mtap_update_slots(void)
{
	if (mtap_update_slots_cb)
		mtap_update_slots_cb();
}

void sio2_mtap_change_slot_set(sio2_mtap_change_slot_cb_t cb) { mtap_change_slot_cb = cb; }
void sio2_mtap_get_slot_max_set(sio2_mtap_get_slot_max_cb_t cb) { mtap_get_slot_max_cb = cb; }
void sio2_mtap_get_slot_max2_set(sio2_mtap_get_slot_max2_cb_t cb) { mtap_get_slot_max2_cb = cb; }
void sio2_mtap_update_slots_set(sio2_mtap_update_slots_t cb) { mtap_update_slots_cb = cb; }

void sio2_ctrl_set(u32 val) { _sw(val, SIO2_REG_CTRL); }
u32  sio2_ctrl_get() { return _lw(SIO2_REG_CTRL); }
u32  sio2_stat6c_get() { return _lw(SIO2_REG_STAT6C); }
void sio2_portN_ctrl1_set(int N, u32 val) { _sw(val, SIO2_REG_PORT0_CTRL1 + (N * 8)); }
u32  sio2_portN_ctrl1_get(int N) { return _lw(SIO2_REG_PORT0_CTRL1 + (N * 8)); }
void sio2_portN_ctrl2_set(int N, u32 val) { _sw(val, SIO2_REG_PORT0_CTRL2 + (N * 8)); }
u32  sio2_portN_ctrl2_get(int N) { return _lw(SIO2_REG_PORT0_CTRL2 + (N * 8)); }
u32  sio2_stat70_get() { return _lw(SIO2_REG_STAT70); }
void sio2_regN_set(int N, u32 val) { _sw(val, SIO2_REG_BASE + (N * 4)); }
u32  sio2_regN_get(int N) { return _lw(SIO2_REG_BASE + (N * 4)); }
u32  sio2_stat74_get() { return _lw(SIO2_REG_STAT74); }
void sio2_unkn78_set(u32 val) { _sw(val, SIO2_REG_UNKN78); }
u32  sio2_unkn78_get() { return _lw(SIO2_REG_UNKN78); }
void sio2_unkn7c_set(u32 val) { _sw(val, SIO2_REG_UNKN7C); }
u32  sio2_unkn7c_get() { return _lw(SIO2_REG_UNKN7C); }
void sio2_data_out(u8 val) { _sb(val, SIO2_REG_DATA_OUT); }
u8   sio2_data_in() { return _lb(SIO2_REG_DATA_IN); }
void sio2_stat_set(u32 val) { _sw(val, SIO2_REG_STAT); }
u32  sio2_stat_get() { return _lw(SIO2_REG_STAT); }
