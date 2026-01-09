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

/**
 * @file
 * SIO2 interface manager and logging utility.
 */

#include <iop_mmio_hwport.h>
#include <rsio2man.h>

#ifdef SIO2LOG
#include "log.h"
#endif

#ifdef SIO2LOG
IRX_ID("sio2man_logger", 3, 17);
#else
IRX_ID("sio2man", 3, 17);
#endif
// Based on the module from SDK 3.1.0.

extern struct irx_export_table _exp_sio2man;
extern struct irx_export_table _exp_sio2man1;

// Unofficial: remove unused structure members
struct sio2man_internal_data
{
	// Unofficial: move inited into bss section
	int m_inited;
	int m_intr_sema;
	int m_transfer_semaphore;
	// Unofficial: backwards compatibility for libraries using 1.3 SDK
	int m_sdk13x_curflag;
	int m_sdk13x_totalflag;
	sio2_mtap_change_slot_cb_t m_mtap_change_slot_cb;
	sio2_mtap_get_slot_max_cb_t m_mtap_get_slot_max_cb;
	sio2_mtap_get_slot_max2_cb_t m_mtap_get_slot_max2_cb;
	sio2_mtap_update_slots_t m_mtap_update_slots_cb;
};

#define EPRINTF(format, args...) printf("%s: " format, _irx_id.n, ##args)

static struct sio2man_internal_data g_sio2man_data;

#ifdef SIO2MAN_NANO
#define sio2_ctrl_set inl_sio2_ctrl_set
#define sio2_ctrl_get inl_sio2_ctrl_get
#define sio2_stat6c_get inl_sio2_stat6c_get
#define sio2_portN_ctrl1_set inl_sio2_portN_ctrl1_set
#define sio2_portN_ctrl2_set inl_sio2_portN_ctrl2_set
#define sio2_regN_set inl_sio2_regN_set
#define sio2_stat74_get inl_sio2_stat74_get
#define sio2_data_out inl_sio2_data_out
#define sio2_data_in inl_sio2_data_in
#define sio2_stat_set inl_sio2_stat_set
#define sio2_stat_get inl_sio2_stat_get
#define sio2_set_ctrl_c inl_sio2_set_ctrl_c
#define sio2_set_ctrl_1 inl_sio2_set_ctrl_1
#define NANO_STATIC static inline
#else
#define NANO_STATIC
#endif

NANO_STATIC void sio2_ctrl_set(u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.ctrl = val;
}

NANO_STATIC u32 sio2_ctrl_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.ctrl;
}

NANO_STATIC u32 sio2_stat6c_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.recv1;
}

NANO_STATIC void sio2_portN_ctrl1_set(int N, u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.send1_2_buf[N * 2] = val;
}

#ifndef SIO2MAN_NANO
u32 sio2_portN_ctrl1_get(int N)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.send1_2_buf[N * 2];
}
#endif

NANO_STATIC void sio2_portN_ctrl2_set(int N, u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.send1_2_buf[(N * 2) + 1] = val;
}

#ifndef SIO2MAN_NANO
u32 sio2_portN_ctrl2_get(int N)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.send1_2_buf[(N * 2) + 1];
}
#endif

u32 sio2_stat70_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.recv2;
}

NANO_STATIC void sio2_regN_set(int N, u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.send3_buf[N] = val;
}

#ifndef SIO2MAN_NANO
u32 sio2_regN_get(int N)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.send3_buf[N];
}
#endif

NANO_STATIC u32 sio2_stat74_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.recv3;
}

#ifndef SIO2MAN_NANO
void sio2_unkn78_set(u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.unk_78 = val;
}
#endif

#ifndef SIO2MAN_NANO
u32 sio2_unkn78_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.unk_78;
}
#endif

#ifndef SIO2MAN_NANO
void sio2_unkn7c_set(u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.unk_7c = val;
}
#endif

#ifndef SIO2MAN_NANO
u32 sio2_unkn7c_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.unk_7c;
}
#endif

NANO_STATIC void sio2_data_out(u8 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.out_fifo = val;
}

NANO_STATIC u8 sio2_data_in(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.in_fifo;
}

NANO_STATIC void sio2_stat_set(u32 val)
{
	USE_IOP_MMIO_HWPORT();

	iop_mmio_hwport->sio2.stat = val;
}

NANO_STATIC u32 sio2_stat_get(void)
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->sio2.stat;
}

static void send_td(sio2_transfer_data_t *td)
{
	int i;

#ifdef SIO2LOG
	log_default(LOG_TRS);
#endif

	for ( i = 0; i < 4; i += 1 )
	{
		sio2_portN_ctrl1_set(i, td->port_ctrl1[i]);
		sio2_portN_ctrl2_set(i, td->port_ctrl2[i]);
	}

#ifdef SIO2LOG
	log_portdata(td->port_ctrl1, td->port_ctrl2);
#endif

	for ( i = 0; i < 16; i += 1 )
		sio2_regN_set(i, td->regdata[i]);

#ifdef SIO2LOG
	log_regdata(td->regdata);
#endif

	for ( i = 0; i < (int)td->in_size; i += 1 )
		sio2_data_out(td->in[i]);
#ifdef SIO2LOG
	if ( td->in_size )
		log_data(LOG_TRS_DATA, td->in, td->in_size);
#endif
	if ( td->in_dma.addr )
	{
		sceSetSliceDMA(IOP_DMAC_SIO2in, td->in_dma.addr, td->in_dma.size, td->in_dma.count, DMAC_FROM_MEM);
		sceStartDMA(IOP_DMAC_SIO2in);
#ifdef SIO2LOG
		log_dma(LOG_TRS_DMA_IN, &td->in_dma);
#endif
	}
	if ( td->out_dma.addr )
	{
		sceSetSliceDMA(IOP_DMAC_SIO2out, td->out_dma.addr, td->out_dma.size, td->out_dma.count, DMAC_TO_MEM);
		sceStartDMA(IOP_DMAC_SIO2out);
#ifdef SIO2LOG
		log_dma(LOG_TRS_DMA_OUT, &td->out_dma);
#endif
	}
}

static void recv_td(sio2_transfer_data_t *td)
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
	for ( i = 0; i < (int)td->out_size; i += 1 )
		td->out[i] = sio2_data_in();
#ifdef SIO2LOG
	if ( td->out_size )
		log_data(LOG_TRR_DATA, td->out, td->out_size);
#endif
}

static int sio2_intr_handler(void *userdata)
{
	const struct sio2man_internal_data *arg;

	arg = (const struct sio2man_internal_data *)userdata;
	sio2_stat_set(sio2_stat_get());
	iSignalSema(arg->m_intr_sema);
	return 1;
}

int _start(int ac, char **av)
{
	iop_sema_t semaparam;
	int state;

	(void)ac;
	(void)av;

	if ( RegisterLibraryEntries(&_exp_sio2man) != 0 )
		return 1;
	// Unofficial: register same name but older major version for backwards compatibility
	_exp_sio2man1.name[7] = '\x00';
	if ( RegisterLibraryEntries(&_exp_sio2man1) != 0 )
		return 1;
	if ( g_sio2man_data.m_inited )
		return 1;
	g_sio2man_data.m_inited = 1;
	g_sio2man_data.m_sdk13x_curflag = 0;
	g_sio2man_data.m_sdk13x_totalflag = 3;
	// Unofficial: remove unneeded thread priority argument handler
	// Unofficial: use setters instead of setting variable directly
	sio2_mtap_change_slot_set(NULL);
	sio2_mtap_get_slot_max_set(NULL);
	sio2_mtap_get_slot_max2_set(NULL);
	sio2_mtap_update_slots_set(NULL);
	// Unofficial: inlined
	sio2_ctrl_set(0x3BC);
	CpuSuspendIntr(&state);
	RegisterIntrHandler(IOP_IRQ_SIO2, 1, sio2_intr_handler, &g_sio2man_data);
	EnableIntr(IOP_IRQ_SIO2);
	CpuResumeIntr(state);
	sceSetDMAPriority(IOP_DMAC_SIO2in, 3);
	sceSetDMAPriority(IOP_DMAC_SIO2out, 3);
	sceEnableDMAChannel(IOP_DMAC_SIO2in);
	sceEnableDMAChannel(IOP_DMAC_SIO2out);
	semaparam.attr = 0;
	semaparam.option = 0;
	semaparam.initial = 1;
	semaparam.max = 64;
	g_sio2man_data.m_transfer_semaphore = CreateSema(&semaparam);
	semaparam.attr = 0;
	semaparam.option = 0;
	semaparam.initial = 0;
	semaparam.max = 64;
	g_sio2man_data.m_intr_sema = CreateSema(&semaparam);
#ifdef SIO2LOG
	EPRINTF("Logging started.\n");
#endif
	return 0;
}

void _deinit()
{
	int state;

	// Unofficial: check inited
	if ( !g_sio2man_data.m_inited )
		return;
#ifdef SIO2LOG
	log_flush(1);
#endif
	// Unofficial: unset inited
	g_sio2man_data.m_inited = 0;
	// Unofficial: remove unused GetThreadId
	CpuSuspendIntr(&state);
	DisableIntr(IOP_IRQ_SIO2, 0);
	ReleaseIntrHandler(IOP_IRQ_SIO2);
	CpuResumeIntr(state);
	sceDisableDMAChannel(IOP_DMAC_SIO2in);
	sceDisableDMAChannel(IOP_DMAC_SIO2out);
	DeleteSema(g_sio2man_data.m_intr_sema);
	DeleteSema(g_sio2man_data.m_transfer_semaphore);
}

#ifndef SIO2MAN_NANO
void sio2_set_intr_handler(int (*handler)(void *userdata), void *userdata)
{
	int state;

	CpuSuspendIntr(&state);
	DisableIntr(IOP_IRQ_SIO2, 0);
	ReleaseIntrHandler(IOP_IRQ_SIO2);
	RegisterIntrHandler(
		IOP_IRQ_SIO2, 1, handler ? handler : sio2_intr_handler, handler ? userdata : &g_sio2man_data);
	EnableIntr(IOP_IRQ_SIO2);
	CpuResumeIntr(state);
}
#endif

NANO_STATIC void sio2_set_ctrl_c()
{
	// Unofficial: inlined
	sio2_ctrl_set(sio2_ctrl_get() | 0xC);
}

NANO_STATIC void sio2_set_ctrl_1()
{
	// Unofficial: inlined
	sio2_ctrl_set(sio2_ctrl_get() | 1);
}

void sio2_wait_for_intr()
{
	WaitSema(g_sio2man_data.m_intr_sema);
}

int sio2_transfer(sio2_transfer_data_t *td)
{
	// Unofficial: remove unused transfer data global
	// Unofficial: replace with inlined func
	sio2_set_ctrl_c();
	send_td(td);
	// Unofficial: replace with inlined func
	sio2_set_ctrl_1();
	sio2_wait_for_intr();
	recv_td(td);
	if ( g_sio2man_data.m_sdk13x_curflag )
		sio2_transfer_reset();
#ifdef SIO2LOG
	log_flush(0);
#endif
	return 1;
}

void sio2_pad_transfer_init(void)
{
#ifdef SIO2LOG
	log_flush(0);
#endif
	WaitSema(g_sio2man_data.m_transfer_semaphore);
#ifdef SIO2LOG
	log_default(LOG_PAD_READY);
#endif
	g_sio2man_data.m_sdk13x_curflag = 0;
}

void sio2_pad_transfer_init_possiblysdk13x(void)
{
	sio2_pad_transfer_init();
	g_sio2man_data.m_sdk13x_curflag |= g_sio2man_data.m_sdk13x_totalflag & 1;
}

void sio2_mc_transfer_init_possiblysdk13x(void)
{
	sio2_pad_transfer_init();
	g_sio2man_data.m_sdk13x_curflag |= g_sio2man_data.m_sdk13x_totalflag & 2;
}

void sio2_transfer_reset(void)
{
	g_sio2man_data.m_sdk13x_curflag = 0;
	SignalSema(g_sio2man_data.m_transfer_semaphore);
#ifdef SIO2LOG
	log_default(LOG_RESET);
#endif
}

static int sio2_mtap_change_slot_default(s32 *arg)
{
	int sum;
	int i;

	sum = 0;
	for ( i = 0; i < 4; i += 1 )
	{
		arg[i + 4] = ((arg[i] + 1) < 2);
		sum += arg[i + 4];
	}
	return sum == 4;
}

static int sio2_mtap_get_slot_max_default(int port)
{
	return 1;
}

static void sio2_mtap_update_slots_default(void) {}

void sio2_mtap_change_slot_set(sio2_mtap_change_slot_cb_t cb)
{
	// Unofficial: use default callback if NULL
	g_sio2man_data.m_mtap_change_slot_cb = cb ? cb : sio2_mtap_change_slot_default;
}

void sio2_mtap_get_slot_max_set(sio2_mtap_get_slot_max_cb_t cb)
{
	// Unofficial: use default callback if NULL
	g_sio2man_data.m_mtap_get_slot_max_cb = cb ? cb : sio2_mtap_get_slot_max_default;
}

void sio2_mtap_get_slot_max2_set(sio2_mtap_get_slot_max2_cb_t cb)
{
	// Unofficial: use default callback if NULL
	g_sio2man_data.m_mtap_get_slot_max2_cb = cb ? cb : sio2_mtap_get_slot_max_default;
}

void sio2_mtap_update_slots_set(sio2_mtap_update_slots_t cb)
{
	// Unofficial: use default callback if NULL
	g_sio2man_data.m_mtap_update_slots_cb = cb ? cb : sio2_mtap_update_slots_default;
}

int sio2_mtap_change_slot(s32 *arg)
{
	g_sio2man_data.m_sdk13x_curflag &= ~g_sio2man_data.m_sdk13x_totalflag;
	// Unofficial: unconditionally call callback
	return g_sio2man_data.m_mtap_change_slot_cb(arg);
}

int sio2_mtap_get_slot_max(int port)
{
	// Unofficial: unconditionally call callback
	return g_sio2man_data.m_mtap_get_slot_max_cb(port);
}

int sio2_mtap_get_slot_max2(int port)
{
	// Unofficial: unconditionally call callback
	return g_sio2man_data.m_mtap_get_slot_max2_cb(port);
}

void sio2_mtap_update_slots(void)
{
	// Unofficial: unconditionally call callback
	g_sio2man_data.m_mtap_update_slots_cb();
}
