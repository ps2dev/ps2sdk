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
#include "sifman.h"

#include "iop_mmio_hwport.h"
#include "sif_mmio_hwport.h"

extern struct irx_export_table _exp_sifman;

#ifdef _IOP
IRX_ID("IOP_SIF_manager", 2, 5);
#endif
// Based on the module from SCE SDK 3.1.0.

typedef struct sif_info_
{
	int data;
	int words;
	int count;
	int addr;
} sif_info_t;

typedef struct sif_completion_cb_info_
{
	void (*func)(void *userdata);
	void *userdata;
} sif_completion_cb_info_t;

typedef struct sif_completion_cb_info_arr_
{
	sif_completion_cb_info_t info[32];
	int count;
} sif_completion_cb_info_arr_t;

typedef struct sifman_internals_
{
	u16 dma_count;
	// cppcheck-suppress unusedStructMember
	char unused02[2];
	int dmatag_index;
	sif_info_t *sif_curbuf;
	sif_info_t sif_buf1[32];
	sif_info_t sif_buf2[32];
	sif_completion_cb_info_arr_t *sif_otherbufcom;
	sif_completion_cb_info_arr_t *sif_curbufcom;
	sif_completion_cb_info_arr_t sif_bufcom1;
	sif_completion_cb_info_arr_t sif_bufcom2;
	void (*dma_intr_handler)(void *userdata);
	void *dma_intr_handler_userdata;
	// cppcheck-suppress unusedStructMember
	char field_624[4];
	sif_info_t one;
	// cppcheck-suppress unusedStructMember
	char field_638[8];
} sifman_internals_t;

static sifman_internals_t sifman_internals;
static u32 sif_dma2_inited = 0;
static u32 sif_inited = 0;

#define PRID $15

#define _mfc0(reg)                                                                                                     \
	({                                                                                                                   \
		u32 val;                                                                                                           \
		__asm__ volatile("mfc0 %0, " #reg : "=r"(val));                                                                    \
		val;                                                                                                               \
	})

#define mfc0(reg) _mfc0(reg)

int _start(int ac, char **av)
{
	s32 prid;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	(void)ac;
	(void)av;

	prid = mfc0(PRID);
	if ( prid >= 16 )
	{
		if ( (iop_mmio_hwport->iop_sbus_ctrl[0] & 8) == 0 )
		{
			if ( sif_mmio_hwport->unk60 == 0x1D000060 )
				return RegisterLibraryEntries(&_exp_sifman) != 0;
			if ( (sif_mmio_hwport->unk60 & 0xFFFFF000) == 0 )
				return RegisterLibraryEntries(&_exp_sifman) != 0;
		}
	}
	return 1;
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

	if ( !sif_dma2_inited )
	{
		iop_mmio_hwport->dmac1.oldch[2].chcr = 0;
		iop_mmio_hwport->dmac1.dpcr1 |= 0x800;
		sif_dma2_inited = 1;
	}
}

static int sif_dma_init(void);

void sceSifInit()
{
	u32 msflag;
	int state;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	if ( !sif_inited )
	{
		iop_mmio_hwport->dmac2.dpcr2 |= 0x8800;
		iop_mmio_hwport->dmac2.newch[2].chcr = 0;
		iop_mmio_hwport->dmac2.newch[3].chcr = 0;
		sceSifDma2Init();
		if ( (iop_mmio_hwport->iop_sbus_ctrl[0] & 0x10) != 0 )
		{
			iop_mmio_hwport->iop_sbus_ctrl[0] |= 0x10;
		}
		iop_mmio_hwport->iop_sbus_ctrl[0] |= 1;
		sif_dma_init();
		do
		{
			CpuSuspendIntr(&state);
			msflag = get_msflag();
			CpuResumeIntr(state);
		} while ( (msflag & SIF_STAT_SIFINIT) == 0 );
		sceSifSetDChain();
		sceSifSetSubAddr(0);
		sif_mmio_hwport->smflag = SIF_STAT_SIFINIT;
		sif_inited = 1;
	}
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
	{
		iop_mmio_hwport->iop_sbus_ctrl[0] |= 0x10;
	}
	return 0;
}

int sceSifCheckInit()
{
	return sif_inited;
}

void sceSifSetDChain()
{
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	if ( (sif_mmio_hwport->controlreg & 0x40) == 0 )
		sif_mmio_hwport->controlreg = 64;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0;
	iop_mmio_hwport->dmac2.newch[3].bcr = (iop_mmio_hwport->dmac2.newch[3].bcr & 0xFFFF0000) | 32;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0x41000300;
}

void sceSifSetDmaIntrHandler(void (*handler)(void *userdata), void *arg)
{
	sifman_internals.dma_intr_handler = handler;
	sifman_internals.dma_intr_handler_userdata = arg;
}

void sceSifResetDmaIntrHandler()
{
	sifman_internals.dma_intr_handler = NULL;
	sifman_internals.dma_intr_handler_userdata = NULL;
}

static int sifman_interrupt_handler(sifman_internals_t *smi)
{
	void (*dma_intr_handler)(void *);
	sif_completion_cb_info_arr_t *sif_otherbufcom;
	int v4;
	sif_completion_cb_info_arr_t *p_sif_bufcom1;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	dma_intr_handler = smi->dma_intr_handler;
	if ( dma_intr_handler )
		dma_intr_handler(smi->dma_intr_handler_userdata);
	sif_otherbufcom = smi->sif_otherbufcom;
	v4 = 0;
	if ( sif_otherbufcom->count > 0 )
	{
		int v5;

		v5 = 0;
		do
		{
			++v4;
			sif_otherbufcom->info[v5].func(sif_otherbufcom->info[v5].userdata);
			sif_otherbufcom = smi->sif_otherbufcom;
			v5 = v4;
		} while ( v4 < sif_otherbufcom->count );
	}
	smi->sif_otherbufcom->count = 0;
	if ( (iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000) == 0 && smi->dmatag_index > 0 )
	{
		iop_mmio_hwport->dmac2.newch[2].chcr = 0;
		iop_mmio_hwport->dmac2.newch[2].tadr = (uiptr)(smi->sif_curbuf);
		iop_mmio_hwport->dmac2.newch[2].bcr = (iop_mmio_hwport->dmac2.newch[2].bcr & 0xFFFF0000) | 32;
		if ( (sif_mmio_hwport->controlreg & 0x20) == 0 )
			sif_mmio_hwport->controlreg = 32;
		++smi->dma_count;
		smi->dmatag_index = 0;
		if ( smi->sif_curbuf == smi->sif_buf1 )
		{
			smi->sif_curbuf = smi->sif_buf2;
			smi->sif_curbufcom = &smi->sif_bufcom2;
			p_sif_bufcom1 = &smi->sif_bufcom1;
		}
		else
		{
			smi->sif_curbufcom = &smi->sif_bufcom1;
			p_sif_bufcom1 = &smi->sif_bufcom2;
			smi->sif_curbuf = smi->sif_buf1;
		}
		smi->sif_otherbufcom = p_sif_bufcom1;
		iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000701;
	}
	return 1;
}

static int sif_dma_init(void)
{
	int state;

	sifman_internals.sif_curbuf = sifman_internals.sif_buf1;
	sifman_internals.sif_curbufcom = &sifman_internals.sif_bufcom1;
	sifman_internals.dmatag_index = 0;
	sifman_internals.sif_bufcom1.count = 0;
	sifman_internals.sif_bufcom2.count = 0;
	sifman_internals.sif_otherbufcom = &sifman_internals.sif_bufcom2;
	sifman_internals.dma_intr_handler = 0;
	sifman_internals.dma_intr_handler_userdata = 0;
	CpuSuspendIntr(&state);
	RegisterIntrHandler(IOP_IRQ_DMA_SIF0, 1, (int (*)(void *))sifman_interrupt_handler, &sifman_internals);
	EnableIntr(IOP_IRQ_DMA_SIF0);
	return CpuResumeIntr(state);
}

static int sif_dma_setup_tag(SifDmaTransfer_t *a1)
{
	sif_info_t *v1;
	int v2;
	unsigned int v3;
	unsigned int v4;
	unsigned int v5;

	v1 = &sifman_internals.sif_curbuf[sifman_internals.dmatag_index];
	v2 = (int)a1->src & 0xFFFFFF;
	v3 = a1->size + 3;
	v1->data = v2;
	v4 = v3 >> 2;
	if ( (a1->attr & 2) != 0 )
		v1->data = v2 | 0x40000000;
	v1->words = v4 & 0xFFFFFF;
	v5 = v3 >> 4;
	if ( (v4 & 3) != 0 )
		++v5;
	v1->count = v5 | 0x10000000;
	if ( (a1->attr & 4) != 0 )
		v1->count |= 0x80000000;
	v1->addr = (int)a1->dest & 0x1FFFFFFF;
	return ++sifman_internals.dmatag_index;
}

static int set_dma_inner(SifDmaTransfer_t *dmat, int count, void (*func)(), void *data)
{
	u8 dmatag_index;
	int dma_count;
	int i;
	int v14;
	sif_completion_cb_info_arr_t *p_sif_bufcom1;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	if ( 32 - sifman_internals.dmatag_index < count )
		return 0;
	dmatag_index = sifman_internals.dmatag_index;
	dma_count = sifman_internals.dma_count;
	if ( sifman_internals.dmatag_index )
		sifman_internals.sif_curbuf[sifman_internals.dmatag_index - 1].data &= ~0x80000000;
	for ( i = 0; i < count; ++dmat )
	{
		sif_dma_setup_tag(dmat);
		++i;
	}
	sifman_internals.sif_curbuf[sifman_internals.dmatag_index - 1].data |= 0x80000000;
	if ( func )
	{
		sifman_internals.sif_curbufcom->info[sifman_internals.sif_curbufcom->count].func = func;
		sifman_internals.sif_curbufcom->info[sifman_internals.sif_curbufcom->count++].userdata = data;
	}
	v14 = dma_count << 16;
	if ( (iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000) == 0 )
	{
		v14 = dma_count << 16;
		if ( iop_mmio_hwport->dmac2.new_unusedch.madr == 0 )
		{
			v14 = dma_count << 16;
			if ( (iop_mmio_hwport->dmac2.dicr2 & 0x4000000) == 0 )
			{
				iop_mmio_hwport->dmac2.newch[2].chcr = 0;
				iop_mmio_hwport->dmac2.newch[2].tadr = (uiptr)(sifman_internals.sif_curbuf);
				if ( (sif_mmio_hwport->controlreg & 0x20) == 0 )
					sif_mmio_hwport->controlreg = 32;
				iop_mmio_hwport->dmac2.newch[2].bcr = (iop_mmio_hwport->dmac2.newch[2].bcr & 0xFFFF0000) | 32;
				sifman_internals.dmatag_index = 0;
				++sifman_internals.dma_count;
				if ( sifman_internals.sif_curbuf == sifman_internals.sif_buf1 )
				{
					sifman_internals.sif_curbuf = sifman_internals.sif_buf2;
					sifman_internals.sif_curbufcom = &sifman_internals.sif_bufcom2;
					p_sif_bufcom1 = &sifman_internals.sif_bufcom1;
				}
				else
				{
					sifman_internals.sif_curbuf = sifman_internals.sif_buf1;
					sifman_internals.sif_curbufcom = &sifman_internals.sif_bufcom1;
					p_sif_bufcom1 = &sifman_internals.sif_bufcom2;
				}
				sifman_internals.sif_otherbufcom = p_sif_bufcom1;
				iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000701;
				v14 = dma_count << 16;
			}
		}
	}
	return v14 | (dmatag_index << 8) | (u8)count;
}

int sceSifSetDma(SifDmaTransfer_t *dmat, int count)
{
	return set_dma_inner(dmat, count, 0, 0);
}

unsigned int sceSifSetDmaIntr(SifDmaTransfer_t *dmat, int len, void (*func)(), void *data)
{
	return set_dma_inner(dmat, len, func, data);
}

static int dma_stat_inner(unsigned int a1)
{
	USE_IOP_MMIO_HWPORT();

	if ( (iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000) == 0 && !iop_mmio_hwport->dmac2.new_unusedch.madr )
	{
		if ( (iop_mmio_hwport->dmac2.dicr2 & 0x4000000) == 0 )
			return -1;
	}
	if ( sifman_internals.dma_count != ((a1 >> 16) & 0xFFFF) )
	{
		if ( sifman_internals.dma_count == (u16)(((a1 >> 16) & 0xFFFF) + 1) )
			return 0;
		return -1;
	}
	return 1;
}

int sceSifDmaStat(int trid)
{
	int v2;
	int state;

	if ( QueryIntrContext() )
		return dma_stat_inner(trid);
	CpuSuspendIntr(&state);
	v2 = dma_stat_inner(trid);
	CpuResumeIntr(state);
	return v2;
}

void sceSifSetOneDma(SifDmaTransfer_t dmat)
{
	int v1;
	unsigned int v2;
	unsigned int v3;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	v1 = ((int)dmat.src & 0xFFFFFF) | 0x80000000;
	v2 = ((unsigned int)dmat.size >> 2) + ((dmat.size & 3) != 0);
	sifman_internals.one.data = v1;
	sifman_internals.one.words = v2 & 0xFFFFFF;
	if ( (dmat.attr & 2) != 0 )
		sifman_internals.one.data = v1 | 0x40000000;
	v3 = v2 >> 2;
	if ( (v2 & 3) != 0 )
		++v3;
	sifman_internals.one.count = v3 | 0x10000000;
	if ( (dmat.attr & 4) != 0 )
		sifman_internals.one.count |= 0x80000000;
	sifman_internals.one.addr = (int)dmat.dest & 0xFFFFFFF;
	if ( (sif_mmio_hwport->controlreg & 0x20) == 0 )
		sif_mmio_hwport->controlreg = 32;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0;
	iop_mmio_hwport->dmac2.newch[2].tadr = (uiptr)&sifman_internals.one;
	iop_mmio_hwport->dmac2.newch[2].bcr = (iop_mmio_hwport->dmac2.newch[2].bcr & 0xFFFF0000) | 32;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000701;
}

void sceSifSendSync()
{
	USE_IOP_MMIO_HWPORT();

	while ( iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000 )
		;
}

int sceSifIsSending()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000;
}

void sceSifDma0Transfer(void *addr, int size, int mode)
{
	unsigned int v4;
	int v5;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	(void)mode;

	v4 = ((unsigned int)size >> 2) + ((size & 3) != 0);
	if ( (sif_mmio_hwport->controlreg & 0x20) == 0 )
		sif_mmio_hwport->controlreg = 32;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0;
	iop_mmio_hwport->dmac2.newch[2].madr = (unsigned int)addr & 0xFFFFFF;
	if ( (v4 & 0x1F) != 0 )
		v5 = (v4 >> 5) + 1;
	else
		v5 = v4 >> 5;
	iop_mmio_hwport->dmac2.newch[2].bcr = ((v5 & 0xFFFF) << 16) | 32;
	iop_mmio_hwport->dmac2.newch[2].chcr = 0x1000201;
}

void sceSifDma0Sync()
{
	USE_IOP_MMIO_HWPORT();

	while ( iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000 )
		;
}

int sceSifDma0Sending()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->dmac2.newch[2].chcr & 0x1000000;
}

void sceSifDma1Transfer(void *addr, int size, int mode)
{
	unsigned int v4;
	int v5;
	int v6;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	v4 = ((unsigned int)size >> 2) + ((size & 3) != 0);
	if ( (sif_mmio_hwport->controlreg & 0x40) == 0 )
		sif_mmio_hwport->controlreg = 64;
	iop_mmio_hwport->dmac2.newch[3].chcr = 0;
	iop_mmio_hwport->dmac2.newch[3].madr = (unsigned int)addr & 0xFFFFFF;
	if ( (v4 & 0x1F) != 0 )
		v5 = (v4 >> 5) + 1;
	else
		v5 = v4 >> 5;
	iop_mmio_hwport->dmac2.newch[3].bcr = ((v5 & 0xFFFF) << 16) | 32;
	if ( (mode & 0x10) != 0 )
		v6 = 0x41000000;
	else
		v6 = 0x1000000;
	iop_mmio_hwport->dmac2.newch[3].chcr = v6 | 0x200;
}

void sceSifDma1Sync()
{
	USE_IOP_MMIO_HWPORT();

	while ( iop_mmio_hwport->dmac2.newch[3].chcr & 0x1000000 )
		;
}

int sceSifDma1Sending()
{
	USE_IOP_MMIO_HWPORT();

	return iop_mmio_hwport->dmac2.newch[3].chcr & 0x1000000;
}

void sceSifDma2Transfer(void *addr, int size, int mode)
{
	unsigned int v4;
	u16 v5;
	int v6;
	int v7;
	USE_IOP_MMIO_HWPORT();
	USE_SIF_MMIO_HWPORT();

	v4 = ((unsigned int)size >> 2) + ((size & 3) != 0);
	if ( (sif_mmio_hwport->controlreg & 0x80) == 0 )
		sif_mmio_hwport->controlreg = 128;
	v5 = v4;
	iop_mmio_hwport->dmac1.oldch[2].chcr = 0;
	iop_mmio_hwport->dmac1.oldch[2].madr = (unsigned int)addr & 0xFFFFFF;
	if ( v4 >= 0x21 )
		v5 = 32;
	if ( (v4 & 0x1F) != 0 )
		v6 = (v4 >> 5) + 1;
	else
		v6 = v4 >> 5;
	iop_mmio_hwport->dmac1.oldch[2].bcr = ((v6 & 0xFFFF) << 16) | (v5 & 0xFFFF);
	if ( (mode & 1) != 0 )
	{
		v7 = 0x1000201;
	}
	else
	{
		int v8;

		if ( (mode & 0x10) != 0 )
			v8 = 0x41000000;
		else
			v8 = 0x1000000;
		v7 = v8 | 0x200;
	}
	iop_mmio_hwport->dmac1.oldch[2].chcr = v7;
}

void sceSifDma2Sync()
{
	USE_IOP_MMIO_HWPORT();

	while ( iop_mmio_hwport->dmac1.oldch[2].chcr & 0x1000000 )
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
	return get_msflag();
}

u32 sceSifGetSMFlag()
{
	return get_smflag();
}

u32 sceSifSetSMFlag(u32 val)
{
	USE_SIF_MMIO_HWPORT();

	sif_mmio_hwport->smflag = val;
	return get_smflag();
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
	return sif_mmio_hwport->smcom;
}

void sceSifIntrMain()
{
	u32 v0;
	USE_IOP_MMIO_HWPORT();

	v0 = iop_mmio_hwport->iop_sbus_ctrl[0];
	iop_mmio_hwport->iop_sbus_ctrl[0] = v0 | 2;
	// cppcheck-suppress redundantAssignment
	iop_mmio_hwport->iop_sbus_ctrl[0] = v0 & 0xFFFFFFFD;
}
