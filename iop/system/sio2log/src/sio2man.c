/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# SIO2 logging utility.
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

#include "sio2man.h"
#include "log.h"

IRX_ID("sio2man_logger", 2, 1);

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
#define EF_TRANSFER_START	0x00000040
#define EF_TRANSFER_FINISH	0x00000080
#define EF_TRANSFER_RESET	0x00000100
#define EF_SIO2_INTR_COMPLETE	0x00000200

#define EPRINTF(format, args...) printf("%s: " format, _irx_id.n , ## args)

int init = 0;
int event_flag = -1;
int thid = -1;
sio2_transfer_data_t *transfer_data = NULL;
int (*cb1)(u32 *) = NULL;
int (*cb2)(int) = NULL;
int (*cb3)(int) = NULL;
void (*cb4)(void) = NULL;

struct irx_export_table _exp_sio2man;

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

	log_default(LOG_TRS);

	for (i = 0; i < 4; i++) {
		sio2_portN_ctrl1_set(i, td->port_ctrl1[i]);
		sio2_portN_ctrl2_set(i, td->port_ctrl2[i]);
	}
	log_portdata(td->port_ctrl1, td->port_ctrl2);

	for (i = 0; i < 16; i++)
		sio2_regN_set(i, td->regdata[i]);
	log_regdata(td->regdata);

	if (td->in_size) {
		for (i = 0; i < td->in_size; i++)
			sio2_data_out(td->in[i]);
		log_data(LOG_TRS_DATA, td->in, td->in_size);
	}

	if (td->in_dma.addr) {
		dmac_request(IOP_DMAC_SIO2in, td->in_dma.addr, td->in_dma.size,
				td->in_dma.count, DMAC_FROM_MEM);
		dmac_transfer(IOP_DMAC_SIO2in);
		log_dma(LOG_TRS_DMA_IN, &td->in_dma);
	}

	if (td->out_dma.addr) {
		dmac_request(IOP_DMAC_SIO2out, td->out_dma.addr, td->out_dma.size,
				td->out_dma.count, DMAC_TO_MEM);
		dmac_transfer(IOP_DMAC_SIO2out);
		log_dma(LOG_TRS_DMA_OUT, &td->out_dma);
	}
}

void recv_td(sio2_transfer_data_t *td)
{
	int i;

	log_default(LOG_TRR);

	td->stat6c = sio2_stat6c_get();
	td->stat70 = sio2_stat70_get();
	td->stat74 = sio2_stat74_get();
	log_stat(td->stat6c, td->stat70, td->stat74);

	if (td->out_size) {
		for (i = 0; i < td->out_size; i++)
			td->out[i] = sio2_data_in();
		log_data(LOG_TRR_DATA, td->out, td->out_size);
	}
}

void main_thread(void *unused)
{
	u32 resbits[4];

	while (1) {
		log_flush(0);
		WaitEventFlag(event_flag, EF_PAD_TRANSFER_INIT |
				EF_MC_TRANSFER_INIT | EF_MTAP_TRANSFER_INIT,
				1, resbits);

		if (resbits[0] & EF_PAD_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_PAD_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_PAD_TRANSFER_READY);
			log_default(LOG_PAD_READY);
		} else if (resbits[0] & EF_MC_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_MC_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_MC_TRANSFER_READY);
			log_default(LOG_MC_READY);
		} else if (resbits[0] & EF_MTAP_TRANSFER_INIT) {
			ClearEventFlag(event_flag, ~EF_MTAP_TRANSFER_INIT);
			SetEventFlag(event_flag, EF_MTAP_TRANSFER_READY);
			log_default(LOG_MTAP_READY);
		} else {
			EPRINTF("Unknown event %08x. Exiting.\n", resbits[0]);
			return;
		}

transfer_loop:
		WaitEventFlag(event_flag, EF_TRANSFER_START | EF_TRANSFER_RESET, 1, resbits);

		if (resbits[0] & EF_TRANSFER_RESET) {
			ClearEventFlag(event_flag, ~EF_TRANSFER_RESET);
			log_default(LOG_RESET);
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

	log_flush(1);

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

	cb1 = NULL;  cb2 = NULL;  cb3 = NULL; cb4 = NULL;
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

	EPRINTF("Logging started.\n");
	return 0;
}

/* 23 */
void sio2_pad_transfer_init()
{
	SetEventFlag(event_flag, EF_PAD_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_PAD_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_PAD_TRANSFER_READY);
}

/* 24 */
void sio2_mc_transfer_init()
{
	SetEventFlag(event_flag, EF_MC_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_MC_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_MC_TRANSFER_READY);
}

/* 48 */
void sio2_mtap_transfer_init()
{
	SetEventFlag(event_flag, EF_MTAP_TRANSFER_INIT);

	WaitEventFlag(event_flag, EF_MTAP_TRANSFER_READY, 0, NULL);
	ClearEventFlag(event_flag, ~EF_MTAP_TRANSFER_READY);
}

/* 25 */
int sio2_transfer(sio2_transfer_data_t *td)
{
	transfer_data = td;
	SetEventFlag(event_flag, EF_TRANSFER_START);

	WaitEventFlag(event_flag, EF_TRANSFER_FINISH, 0, NULL);
	ClearEventFlag(event_flag, ~EF_TRANSFER_FINISH);
	return 1;
}

/* 26 */
void sio2_transfer_reset()
{
	SetEventFlag(event_flag, EF_TRANSFER_RESET);
}

/* 55 */
int sio2_func1(u32 *arg)
{
	int i, ret = 1;

	if (cb1)
		return cb1(arg);

	for (i = 0; i < 4; i++, arg++) {
		if ((*arg + 1) < 2)
			arg[4] = 1;
		else {
			arg[4] = 0;
			ret = 0;
		}
	}

	return ret;
}

/* 56 */
int sio2_func2(int arg)
{
	if (cb2)
		return cb2(arg);

	return 1;
}

/* 57 */
int sio2_func3(int arg)
{
	if (cb3)
		return cb3(arg);

	return 1;
}

/* 58 */
void sio2_func4()
{
	if (cb4)
		cb4();
}

/* 51 */ void sio2_cb1_set(int (*cb)(u32 *)) { cb1 = cb; }
/* 52 */ void sio2_cb2_set(int (*cb)(int)) { cb2 = cb; }
/* 53 */ void sio2_cb3_set(int (*cb)(int)) { cb3 = cb; }
/* 54 */ void sio2_cb4_set(void (*cb)(void)) { cb4 = cb; }

/* 04 */ void sio2_ctrl_set(u32 val) { _sw(val, SIO2_REG_CTRL); }
/* 05 */ u32  sio2_ctrl_get() { return _lw(SIO2_REG_CTRL); }
/* 06 */ u32  sio2_stat6c_get() { return _lw(SIO2_REG_STAT6C); }
/* 07 */ void sio2_portN_ctrl1_set(int N, u32 val) { _sw(val, SIO2_REG_PORT0_CTRL1 + (N * 8)); }
/* 08 */ u32  sio2_portN_ctrl1_get(int N) { return _lw(SIO2_REG_PORT0_CTRL1 + (N * 8)); }
/* 09 */ void sio2_portN_ctrl2_set(int N, u32 val) { _sw(val, SIO2_REG_PORT0_CTRL2 + (N * 8)); }
/* 10 */ u32  sio2_portN_ctrl2_get(int N) { return _lw(SIO2_REG_PORT0_CTRL2 + (N * 8)); }
/* 11 */ u32  sio2_stat70_get() { return _lw(SIO2_REG_STAT70); }
/* 12 */ void sio2_regN_set(int N, u32 val) { _sw(val, SIO2_REG_BASE + (N * 4)); }
/* 13 */ u32  sio2_regN_get(int N) { return _lw(SIO2_REG_BASE + (N * 4)); }
/* 14 */ u32  sio2_stat74_get() { return _lw(SIO2_REG_STAT74); }
/* 15 */ void sio2_unkn78_set(u32 val) { _sw(val, SIO2_REG_UNKN78); }
/* 16 */ u32  sio2_unkn78_get() { return _lw(SIO2_REG_UNKN78); }
/* 17 */ void sio2_unkn7c_set(u32 val) { _sw(val, SIO2_REG_UNKN7C); }
/* 18 */ u32  sio2_unkn7c_get() { return _lw(SIO2_REG_UNKN7C); }
/* 19 */ void sio2_data_out(u8 val) { _sb(val, SIO2_REG_DATA_OUT); }
/* 20 */ u8   sio2_data_in() { return _lb(SIO2_REG_DATA_IN); }
/* 21 */ void sio2_stat_set(u32 val) { _sw(val, SIO2_REG_STAT); }
/* 22 */ u32  sio2_stat_get() { return _lw(SIO2_REG_STAT); }

DECLARE_EXPORT_TABLE(sio2man, 1, 2)
	DECLARE_EXPORT(_start)
	DECLARE_EXPORT(_retonly)
	DECLARE_EXPORT(shutdown)
	DECLARE_EXPORT(_retonly)

	/* Register manipulation 04 - 22 */
	DECLARE_EXPORT(sio2_ctrl_set)
	DECLARE_EXPORT(sio2_ctrl_get)
	DECLARE_EXPORT(sio2_stat6c_get)
	DECLARE_EXPORT(sio2_portN_ctrl1_set)
	DECLARE_EXPORT(sio2_portN_ctrl1_get)
	DECLARE_EXPORT(sio2_portN_ctrl2_set)
	DECLARE_EXPORT(sio2_portN_ctrl2_get)
	DECLARE_EXPORT(sio2_stat70_get)
	DECLARE_EXPORT(sio2_regN_set)
	DECLARE_EXPORT(sio2_regN_get)
	DECLARE_EXPORT(sio2_stat74_get)
	DECLARE_EXPORT(sio2_unkn78_set)
	DECLARE_EXPORT(sio2_unkn78_get)
	DECLARE_EXPORT(sio2_unkn7c_set)
	DECLARE_EXPORT(sio2_unkn7c_get)
	DECLARE_EXPORT(sio2_data_out)
	DECLARE_EXPORT(sio2_data_in)
	DECLARE_EXPORT(sio2_stat_set)
	DECLARE_EXPORT(sio2_stat_get)

	/* Transfer events 23 - 26 */
	DECLARE_EXPORT(sio2_pad_transfer_init)
	DECLARE_EXPORT(sio2_mc_transfer_init)
	DECLARE_EXPORT(sio2_transfer)
	DECLARE_EXPORT(sio2_transfer_reset)

	/* Repeat of register routines 27 - 45 */
	DECLARE_EXPORT(sio2_ctrl_set)
	DECLARE_EXPORT(sio2_ctrl_get)
	DECLARE_EXPORT(sio2_stat6c_get)
	DECLARE_EXPORT(sio2_portN_ctrl1_set)
	DECLARE_EXPORT(sio2_portN_ctrl1_get)
	DECLARE_EXPORT(sio2_portN_ctrl2_set)
	DECLARE_EXPORT(sio2_portN_ctrl2_get)
	DECLARE_EXPORT(sio2_stat70_get)
	DECLARE_EXPORT(sio2_regN_set)
	DECLARE_EXPORT(sio2_regN_get)
	DECLARE_EXPORT(sio2_stat74_get)
	DECLARE_EXPORT(sio2_unkn78_set)
	DECLARE_EXPORT(sio2_unkn78_get)
	DECLARE_EXPORT(sio2_unkn7c_set)
	DECLARE_EXPORT(sio2_unkn7c_get)
	DECLARE_EXPORT(sio2_data_out)
	DECLARE_EXPORT(sio2_data_in)
	DECLARE_EXPORT(sio2_stat_set)
	DECLARE_EXPORT(sio2_stat_get)

	/* Repeat of transfers + mtap 46 - 50 */
	DECLARE_EXPORT(sio2_pad_transfer_init)
	DECLARE_EXPORT(sio2_mc_transfer_init)
	DECLARE_EXPORT(sio2_mtap_transfer_init)
	DECLARE_EXPORT(sio2_transfer)
	DECLARE_EXPORT(sio2_transfer_reset)

	/* Callbacks 51 - 58 */
	DECLARE_EXPORT(sio2_cb1_set)
	DECLARE_EXPORT(sio2_cb2_set)
	DECLARE_EXPORT(sio2_cb3_set)
	DECLARE_EXPORT(sio2_cb4_set)
	DECLARE_EXPORT(sio2_func1)
	DECLARE_EXPORT(sio2_func2)
	DECLARE_EXPORT(sio2_func3)
	DECLARE_EXPORT(sio2_func4)

END_EXPORT_TABLE

void _retonly() {}

loadcore_IMPORTS_start
I_RegisterLibraryEntries
loadcore_IMPORTS_end

intrman_IMPORTS_start
I_RegisterIntrHandler
I_ReleaseIntrHandler
I_EnableIntr
I_DisableIntr
I_CpuSuspendIntr
I_CpuResumeIntr
intrman_IMPORTS_end

dmacman_IMPORTS_start
I_dmac_request
I_dmac_transfer
I_dmac_ch_set_dpcr
I_dmac_enable
I_dmac_disable
dmacman_IMPORTS_end

stdio_IMPORTS_start
I_printf
stdio_IMPORTS_end

thbase_IMPORTS_start
I_CreateThread
I_StartThread
thbase_IMPORTS_end

thevent_IMPORTS_start
I_CreateEventFlag
I_SetEventFlag
I_iSetEventFlag
I_ClearEventFlag
I_WaitEventFlag
thevent_IMPORTS_end

ioman_IMPORTS_start
I_open
I_write
I_close
ioman_IMPORTS_end
