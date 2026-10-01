/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "sifman.h"
#include "irx_imports.h"
#include <mipscopaccess.h>

#include "iop_mmio_hwport.h"
#include "sif_mmio_hwport.h"

extern struct irx_export_table _exp_sifman;

#ifdef _IOP
IRX_ID("IOP_SIF_manager", 2, 5);
#endif
// Based on the module from SCE SDK 3.1.0.

typedef struct sif_info_
{
	int m_data;
	int m_words;
	int m_count;
	int m_addr;
} sif_info_t;

typedef struct sif_info_buf_
{
	sif_info_t m_info[32];
} sif_info_buf_t;

typedef struct sif_completion_cb_info_
{
	void (*m_func)(void *userdata);
	void *m_userdata;
} sif_completion_cb_info_t;

typedef struct sif_completion_cb_info_arr_
{
	sif_completion_cb_info_t m_info[32];
	int m_count;
} sif_completion_cb_info_arr_t;

typedef struct sifman_internals_
{
	u16 m_dma_count;
	// cppcheck-suppress unusedStructMember
	char m_unused_002[2];
	int m_dmatag_index;
	sif_info_buf_t *m_sif_curbuf;
	sif_info_buf_t m_sif_buf[2];
	sif_completion_cb_info_arr_t *m_sif_curbufcom[2];
	sif_completion_cb_info_arr_t m_sif_bufcom[2];
	void (*m_dma_intr_handler)(void *userdata);
	void *m_dma_intr_handler_userdata;
	// cppcheck-suppress unusedStructMember
	char m_unused_624[4];
	sif_info_t m_one;
	// cppcheck-suppress unusedStructMember
	char m_unused_638[8];
} sifman_internals_t;

static sifman_internals_t g_sifman_internals;
static u32 g_sif_dma2_inited = 0;
static u32 g_sif_inited = 0;

int _start(int ac, char **av)
{
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	(void)ac;
	(void)av;

	if ( get_mips_cop_reg(0, COP0_REG_PRId) < 16 || (iop_mmio_hwport->iop_sbus_ctrl[0] & 8) )
		return MODULE_NO_RESIDENT_END;
	return (sif_mmio_hwport->unk60 == 0x1D000060 || !(sif_mmio_hwport->unk60 & 0xFFFFF000)) ?
					 (RegisterLibraryEntries(&_exp_sifman) ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END) :
					 MODULE_NO_RESIDENT_END;
}

static u32 get_msflag()
{
	u32 result;
	USE_SIF_MMIO_HWPORT();

	for ( result = sif_mmio_hwport->msflag; result != sif_mmio_hwport->msflag; result = sif_mmio_hwport->msflag )
		;
	return result;
}

static u32 get_smflag()
{
	u32 result;
	USE_SIF_MMIO_HWPORT();

	for ( result = sif_mmio_hwport->smflag; result != sif_mmio_hwport->smflag; result = sif_mmio_hwport->smflag )
		;
	return result;
}

void sceSifDma2Init()
{
	USE_IOP_MMIO_HWPORT();

	if ( g_sif_dma2_inited )
		return;
	iop_mmio_hwport->dmac1.oldch[2].chcr = 0;
	iop_mmio_hwport->dmac1.dpcr1 |= 0x800;
	g_sif_dma2_inited = 1;
}

static void sif_dma_init(void);

void sceSifInit()
{
	u32 msflag;
	int state;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	if ( g_sif_inited )
		return;
	iop_mmio_hwport->dmac2.dpcr2 |= 0x8800;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0;
	sceSifDma2Init();
	if ( (iop_mmio_hwport->iop_sbus_ctrl[0] & 0x10) )
		iop_mmio_hwport->iop_sbus_ctrl[0] |= 0x10;
	iop_mmio_hwport->iop_sbus_ctrl[0] |= 1;
	sif_dma_init();
	msflag = 0;
	while ( !(msflag & SIF_STAT_SIFINIT) )
	{
		CpuSuspendIntr(&state);
		msflag = get_msflag();
		CpuResumeIntr(state);
	}
	sceSifSetDChain();
	sceSifSetSubAddr(0);
	sif_mmio_hwport->smflag = SIF_STAT_SIFINIT;
	g_sif_inited = 1;
}

int sifman_deinit()
{
	int old_irq;
	USE_IOP_MMIO_HWPORT();

	DisableIntr(IOP_IRQ_DMA_SIF0, &old_irq);
	ReleaseIntrHandler(IOP_IRQ_DMA_SIF0);
	iop_mmio_hwport->dmac2.newch[2].chcr = 0;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0;
	if ( iop_mmio_hwport->iop_sbus_ctrl[0] & 0x10 )
		iop_mmio_hwport->iop_sbus_ctrl[0] |= 0x10;
	return 0;
}

int sceSifCheckInit()
{
	return g_sif_inited;
}

void sceSifSetDChain(void)
{
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	if ( !(sif_mmio_hwport->controlreg & 0x40) )
		sif_mmio_hwport->controlreg = 0x40;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0;
	iop_mmio_hwport->dmac2.newch[3].bcr = (iop_mmio_hwport->dmac2.newch[3].bcr & 0xFFFF0000) | 0x20;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0x41000300;
}

static void sif_dma_handler_noop(void *userdata)
{
	(void)userdata;
}

void sceSifSetDmaIntrHandler(void (*handler)(void *userdata), void *arg)
{
	g_sifman_internals.m_dma_intr_handler = handler ? handler : &sif_dma_handler_noop;
	g_sifman_internals.m_dma_intr_handler_userdata = arg;
}

void sceSifResetDmaIntrHandler()
{
	sceSifSetDmaIntrHandler(NULL, NULL);
}

static int sifman_interrupt_handler(void *userdata)
{
	int i;
	sifman_internals_t *smi;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	smi = (sifman_internals_t *)userdata;
	// Unofficial: unconditionally call callback
	smi->m_dma_intr_handler(smi->m_dma_intr_handler_userdata);
	for ( i = 0; i < smi->m_sif_curbufcom[0]->m_count; i += 1 )
		smi->m_sif_curbufcom[0]->m_info[i].m_func(smi->m_sif_curbufcom[0]->m_info[i].m_userdata);
	smi->m_sif_curbufcom[0]->m_count = 0;
	if ( !sceSifIsSending() && smi->m_dmatag_index > 0 )
	{
		iop_mmio_hwport->dmac2.newch[2].chcr = 0;
		iop_mmio_hwport->dmac2.newch[2].tadr = (uiptr)(smi->m_sif_curbuf);
		iop_mmio_hwport->dmac2.newch[2].bcr = (iop_mmio_hwport->dmac2.newch[2].bcr & 0xFFFF0000) | 0x20;
		if ( !(sif_mmio_hwport->controlreg & 0x20) )
			sif_mmio_hwport->controlreg = 0x20;
		smi->m_dma_count += 1;
		smi->m_dmatag_index = 0;
		smi->m_sif_curbufcom[1] = &smi->m_sif_bufcom[(smi->m_sif_curbuf == &smi->m_sif_buf[0]) ? 1 : 0];
		smi->m_sif_curbufcom[0] = &smi->m_sif_bufcom[(smi->m_sif_curbuf == &smi->m_sif_buf[0]) ? 0 : 1];
		smi->m_sif_curbuf = &smi->m_sif_buf[(smi->m_sif_curbuf == &smi->m_sif_buf[0]) ? 1 : 0];
		iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000701;
	}
	return 1;
}

static void sif_dma_init(void)
{
	int state;

	g_sifman_internals.m_sif_curbufcom[1] = &g_sifman_internals.m_sif_bufcom[0];
	g_sifman_internals.m_sif_curbufcom[0] = &g_sifman_internals.m_sif_bufcom[1];
	g_sifman_internals.m_sif_curbuf = &g_sifman_internals.m_sif_buf[0];
	g_sifman_internals.m_dmatag_index = 0;
	g_sifman_internals.m_sif_bufcom[0].m_count = 0;
	g_sifman_internals.m_sif_bufcom[1].m_count = 0;
	sceSifResetDmaIntrHandler();
	CpuSuspendIntr(&state);
	RegisterIntrHandler(IOP_IRQ_DMA_SIF0, 1, sifman_interrupt_handler, &g_sifman_internals);
	EnableIntr(IOP_IRQ_DMA_SIF0);
	CpuResumeIntr(state);
}

static void sif_dma_setup_tag(const SifDmaTransfer_t *dmat)
{
	sif_info_t *si;
	unsigned int size_rounded;

	si = &g_sifman_internals.m_sif_curbuf->m_info[g_sifman_internals.m_dmatag_index];
	si->m_data = ((uiptr)dmat->src & 0xFFFFFF) | ((dmat->attr & SIF_DMA_INT_I) ? 0x40000000 : 0);
	size_rounded = dmat->size + 3;
	si->m_words = (size_rounded >> 2) & 0xFFFFFF;
	si->m_count =
		((size_rounded >> 4) + !!(si->m_words & 3)) | 0x10000000 | ((dmat->attr & SIF_DMA_INT_O) ? 0x80000000 : 0);
	si->m_addr = (uiptr)dmat->dest & 0x1FFFFFFF;
	g_sifman_internals.m_dmatag_index += 1;
}

static int set_dma_inner(const SifDmaTransfer_t *dmat, int count, void (*completioncb)(void *userdata), void *data)
{
	u8 dmatag_index;
	u16 dma_count;
	int i;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	dmatag_index = g_sifman_internals.m_dmatag_index;
	if (
		dmatag_index + count
		> (int)(sizeof(g_sifman_internals.m_sif_curbuf->m_info) / sizeof(g_sifman_internals.m_sif_curbuf->m_info[0])) )
		return 0;
	dma_count = g_sifman_internals.m_dma_count;
	if ( dmatag_index )
		g_sifman_internals.m_sif_curbuf->m_info[dmatag_index - 1].m_data &= ~0x80000000;
	for ( i = 0; i < count; i += 1 )
		sif_dma_setup_tag(&dmat[i]);
	g_sifman_internals.m_sif_curbuf->m_info[g_sifman_internals.m_dmatag_index - 1].m_data |= 0x80000000;
	if ( completioncb )
	{
		g_sifman_internals.m_sif_curbufcom[1]->m_info[g_sifman_internals.m_sif_curbufcom[1]->m_count].m_func = completioncb;
		g_sifman_internals.m_sif_curbufcom[1]->m_info[g_sifman_internals.m_sif_curbufcom[1]->m_count].m_userdata = data;
		g_sifman_internals.m_sif_curbufcom[1]->m_count += 1;
	}
	if ( !sceSifIsSending() && !iop_mmio_hwport->dmac2.new_unusedch.madr && !(iop_mmio_hwport->dmac2.dicr2 & 0x4000000) )
	{
		iop_mmio_hwport->dmac2.newch[2].chcr = 0;
		iop_mmio_hwport->dmac2.newch[2].tadr = (uiptr)(g_sifman_internals.m_sif_curbuf);
		if ( !(sif_mmio_hwport->controlreg & 0x20) )
			sif_mmio_hwport->controlreg = 0x20;
		iop_mmio_hwport->dmac2.newch[2].bcr = (iop_mmio_hwport->dmac2.newch[2].bcr & 0xFFFF0000) | 0x20;
		g_sifman_internals.m_dmatag_index = 0;
		g_sifman_internals.m_dma_count += 1;
		g_sifman_internals.m_sif_curbufcom[1] =
			&g_sifman_internals.m_sif_bufcom[(g_sifman_internals.m_sif_curbuf == &g_sifman_internals.m_sif_buf[0]) ? 1 : 0];
		g_sifman_internals.m_sif_curbufcom[0] =
			&g_sifman_internals.m_sif_bufcom[(g_sifman_internals.m_sif_curbuf == &g_sifman_internals.m_sif_buf[0]) ? 0 : 1];
		g_sifman_internals.m_sif_curbuf =
			&g_sifman_internals.m_sif_buf[(g_sifman_internals.m_sif_curbuf == &g_sifman_internals.m_sif_buf[0]) ? 1 : 0];
		iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000701;
	}
	return (dma_count << 16) | (dmatag_index << 8) | (count & 0xFF);
}

// cppcheck-suppress constParameterPointer
int sceSifSetDma(SifDmaTransfer_t *dmat, int count)
{
	return set_dma_inner(dmat, count, NULL, NULL);
}

// cppcheck-suppress constParameterPointer
unsigned int sceSifSetDmaIntr(SifDmaTransfer_t *dmat, int count, void (*completioncb)(void *userdata), void *userdata)
{
	return set_dma_inner(dmat, count, completioncb, userdata);
}

static int dma_stat_inner(unsigned int trid)
{
	USE_IOP_MMIO_HWPORT();

	if ( !sceSifIsSending() && !iop_mmio_hwport->dmac2.new_unusedch.madr && !(iop_mmio_hwport->dmac2.dicr2 & 0x4000000) )
		return -1;
	if ( g_sifman_internals.m_dma_count == (u16)(((trid >> 16) & 0xFFFF) + 1) )
		return 0;
	if ( g_sifman_internals.m_dma_count != ((trid >> 16) & 0xFFFF) )
		return -1;
	return 1;
}

int sceSifDmaStat(int trid)
{
	int statres;
	int state;

	if ( QueryIntrContext() )
		return dma_stat_inner(trid);
	CpuSuspendIntr(&state);
	statres = dma_stat_inner(trid);
	CpuResumeIntr(state);
	return statres;
}

void sceSifSetOneDma(SifDmaTransfer_t dmat)
{
	unsigned int size_rounded;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	size_rounded = ((unsigned int)dmat.size >> 2) + !!(dmat.size & 3);
	g_sifman_internals.m_one.m_data =
		(((uiptr)dmat.src & 0xFFFFFF) | 0x80000000) | ((dmat.attr & SIF_DMA_INT_I) ? 0x40000000 : 0);
	g_sifman_internals.m_one.m_words = size_rounded & 0xFFFFFF;
	g_sifman_internals.m_one.m_count = ((size_rounded >> 2) + (!!(size_rounded & 3))) | 0x10000000;
	g_sifman_internals.m_one.m_count |= (dmat.attr & SIF_DMA_INT_O) ? 0x80000000 : 0;
	g_sifman_internals.m_one.m_addr = (uiptr)dmat.dest & 0xFFFFFFF;
	if ( !(sif_mmio_hwport->controlreg & 0x20) )
		sif_mmio_hwport->controlreg = 0x20;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0;
	iop_mmio_hwport->dmac2.newch[2].tadr = (uiptr)&g_sifman_internals.m_one;
	iop_mmio_hwport->dmac2.newch[2].bcr = (iop_mmio_hwport->dmac2.newch[2].bcr & 0xFFFF0000) | 0x20;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000701;
}

void sceSifSendSync()
{
	sceSifDma0Sync();
}

int sceSifIsSending()
{
	return sceSifDma0Sending();
}

// cppcheck-suppress constParameterPointer
void sceSifDma0Transfer(void *addr, int size, int mode)
{
	unsigned int size_rounded;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	(void)mode;

	size_rounded = ((unsigned int)size >> 2) + !!(size & 3);
	if ( !(sif_mmio_hwport->controlreg & 0x20) )
		sif_mmio_hwport->controlreg = 0x20;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0;
	iop_mmio_hwport->dmac2.newch[2].madr = (uiptr)addr & 0xFFFFFF;
	iop_mmio_hwport->dmac2.newch[2].bcr = ((((size_rounded >> 5) + !!(size_rounded & 0x1F)) & 0xFFFF) << 16) | 0x20;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000201;
}

void sceSifDma0Sync()
{
	while ( sceSifDma0Sending() )
		;
}

int sceSifDma0Sending()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000;
}

// cppcheck-suppress constParameterPointer
void sceSifDma1Transfer(void *addr, int size, int mode)
{
	unsigned int size_rounded;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	size_rounded = ((unsigned int)size >> 2) + !!(size & 3);
	if ( !(sif_mmio_hwport->controlreg & 0x40) )
		sif_mmio_hwport->controlreg = 0x40;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0;
	iop_mmio_hwport->dmac2.newch[3].madr = (uiptr)addr & 0xFFFFFF;
	iop_mmio_hwport->dmac2.newch[3].bcr = ((((size_rounded >> 5) + (!!(size_rounded & 0x1F))) & 0xFFFF) << 16) | 0x20;
	iop_mmio_hwport->dmac2.newch[3].chcr = (0x1000000 | ((mode & 0x10) ? 0x40000000 : 0)) | 0x200;
}

void sceSifDma1Sync()
{
	while ( sceSifDma1Sending() )
		;
}

int sceSifDma1Sending()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->dmac2.newch[3].chcr & 0x1000000;
}

// cppcheck-suppress constParameterPointer
void sceSifDma2Transfer(void *addr, int size, int mode)
{
	unsigned int size_rounded;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	size_rounded = ((unsigned int)size >> 2) + !!(size & 3);
	if ( !(sif_mmio_hwport->controlreg & 0x80) )
		sif_mmio_hwport->controlreg = 0x80;
	iop_mmio_hwport->dmac1.oldch[2].chcr = 0;
	iop_mmio_hwport->dmac1.oldch[2].madr = (uiptr)addr & 0xFFFFFF;
	iop_mmio_hwport->dmac1.oldch[2].bcr = ((((size_rounded >> 5) + (!!(size_rounded & 0x1F))) & 0xFFFF) << 16)
																			| (((size_rounded > 0x20) ? 0x20 : size_rounded) & 0xFFFF);
	iop_mmio_hwport->dmac1.oldch[2].chcr = 0x1000000 | ((mode & 1) ? 1 : ((mode & 0x10) ? 0x40000000 : 0)) | 0x200;
}

void sceSifDma2Sync()
{
	while ( sceSifDma2Sending() )
		;
}

int sceSifDma2Sending()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->dmac1.oldch[2].chcr & 0x1000000;
}

u32 sceSifGetMSFlag()
{
	return get_msflag();
}

u32 sceSifSetMSFlag(u32 val)
{
	USE_SIF_MMIO_HWPORT();

	sif_mmio_hwport->msflag = val;
	return sceSifGetMSFlag();
}

u32 sceSifGetSMFlag()
{
	return get_smflag();
}

u32 sceSifSetSMFlag(u32 val)
{
	USE_SIF_MMIO_HWPORT();

	sif_mmio_hwport->smflag = val;
	return sceSifGetSMFlag();
}

u32 sceSifGetMainAddr()
{
	USE_SIF_MMIO_HWPORT();

	return sif_mmio_hwport->mscom;
}

u32 sceSifGetSubAddr()
{
	USE_SIF_MMIO_HWPORT();

	return sif_mmio_hwport->smcom;
}

u32 sceSifSetSubAddr(u32 addr)
{
	USE_SIF_MMIO_HWPORT();

	sif_mmio_hwport->smcom = addr;
	return sceSifGetSubAddr();
}

void sceSifIntrMain()
{
	u32 sbus_ctrl_0;
	USE_IOP_MMIO_HWPORT();

	sbus_ctrl_0 = iop_mmio_hwport->iop_sbus_ctrl[0];
	iop_mmio_hwport->iop_sbus_ctrl[0] = sbus_ctrl_0 | 2;
	// cppcheck-suppress redundantAssignment
	iop_mmio_hwport->iop_sbus_ctrl[0] = sbus_ctrl_0 & ~2;
}
