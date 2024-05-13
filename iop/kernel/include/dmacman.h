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
 * DMACMAN definitions and imports.
 */

#ifndef __DMACMAN_H__
#define __DMACMAN_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _iop_dmac_chan {
	u32	madr;
	u32	bcr;
	u32	chcr;
} iop_dmac_chan_t;

/* CHCR flags */
#define DMAC_CHCR_30 (1<<30)
/** TRansfer */
#define DMAC_CHCR_TR (1<<24)
/** LInked list (GPU, SPU and SIF0) */
#define DMAC_CHCR_LI (1<<10)
/** COntinuous (?) */
#define DMAC_CHCR_CO (1<<9)
#define DMAC_CHCR_08 (1<<8)
/** DiRection; 0 = to RAM, 1 = from RAM */
#define DMAC_CHCR_DR (1<<0)

#define DMAC_TO_MEM	0
#define DMAC_FROM_MEM	1

//SIF2 DMA ch 2 (GPU)
#define IOP_DMAC_SIF2_MADR		(*(volatile int*)0xBF8010A0)
#define IOP_DMAC_SIF2_BCR		(*(volatile int*)0xBF8010A4)
#define IOP_DMAC_SIF2_BCR_size	(*(volatile short*)0xBF8010A4)
#define IOP_DMAC_SIF2_BCR_count	(*(volatile short*)0xBF8010A6)
#define IOP_DMAC_SIF2_CHCR		(*(volatile int*)0xBF8010A8)
//SIF0 DMA ch 9
#define IOP_DMAC_SIF9_MADR		(*(volatile int*)0xBF801520)
#define IOP_DMAC_SIF9_BCR		(*(volatile int*)0xBF801524)
#define IOP_DMAC_SIF9_BCR_size	(*(volatile short*)0xBF801524)
#define IOP_DMAC_SIF9_BCR_count	(*(volatile short*)0xBF801526)
#define IOP_DMAC_SIF9_CHCR		(*(volatile int*)0xBF801528)
#define IOP_DMAC_SIF9_TADR		(*(volatile int*)0xBF80152C)
//SIF1 DMA ch 10 (0xA)
#define IOP_DMAC_SIFA_MADR		(*(volatile int*)0xBF801530)
#define IOP_DMAC_SIFA_BCR		(*(volatile int*)0xBF801534)
#define IOP_DMAC_SIFA_BCR_size	(*(volatile short*)0xBF801534)
#define IOP_DMAC_SIFA_BCR_count	(*(volatile short*)0xBF801536)
#define IOP_DMAC_SIFA_CHCR		(*(volatile int*)0xBF801538)

#define IOP_DMAC_DPCR		(*(volatile int*)0xBF8010F0)
#define IOP_DMAC_DPCR2		(*(volatile int*)0xBF801570)

enum _iop_dmac_ch {
	IOP_DMAC_MDECin,	IOP_DMAC_MDECout,	IOP_DMAC_SIF2,	IOP_DMAC_CDVD,
	IOP_DMAC_SPU,		IOP_DMAC_PIO,		IOP_DMAC_OTC,	IOP_DMAC_SPU2,
	IOP_DMAC_DEV9,		IOP_DMAC_SIF0,		IOP_DMAC_SIF1,	IOP_DMAC_SIO2in,
	IOP_DMAC_SIO2out,	IOP_DMAC_FDMA0,		IOP_DMAC_FDMA1,	IOP_DMAC_FDMA2,
	IOP_DMAC_CPU = 67,	//'C'
	IOP_DMAC_USB = 85,	//'U'
};

/* Note that these are far from official names.  */

/* Memory ADdRess  */
void dmac_ch_set_madr(u32 channel, u32 val);
u32 dmac_ch_get_madr(u32 channel);

/* Block Control Register  */
void dmac_ch_set_bcr(u32 channel, u32 val);
u32 dmac_ch_get_bcr(u32 channel);

/* CHannel Control Register  */
void dmac_ch_set_chcr(u32 channel, u32 val);
u32 dmac_ch_get_chcr(u32 channel);

/* Tag ADdRess */
void dmac_ch_set_tadr(u32 channel, u32 val);
u32 dmac_ch_get_tadr(u32 channel);

void dmac_set_4_9_a(u32 channel, u32 val);
u32 dmac_get_4_9_a(u32 channel);

/* Dmac P??? Control Registers  */
void dmac_set_dpcr(u32 val);
u32 dmac_get_dpcr(void);
void dmac_set_dpcr2(u32 val);
u32 dmac_get_dpcr2(void);
void dmac_set_dpcr3(u32 val);
u32 dmac_get_dpcr3(void);

/* Dmac Interrupt Control Registers  */
void dmac_set_dicr(u32 val);
u32 dmac_get_dicr(void);
void dmac_set_dicr2(u32 val);
u32 dmac_get_dicr2(void);

void dmac_set_BF80157C(u32 val);
u32 dmac_get_BF80157C(void);

void dmac_set_BF801578(u32 val);
u32 dmac_get_BF801578(void);

/* Initialize the given channel and start the transfer.  Returns 1 if the
   transfer was started, and 0 on error.  */
int sceSetSliceDMA(u32 channel, void * addr, u32 size, u32 count, int dir);

int dmac_set_dma_chained_spu_sif0(u32 channel, u32 size, u32 tadr);
int dmac_set_dma_sif0(u32 channel, u32 size, u32 tadr);
int dmac_set_dma_sif1(u32 ch, u32 size);

/* Start a transfer on the given channel.  */
void sceStartDMA(u32 channel);

/* Set the DPCRn value for a specific channel.  */
void sceSetDMAPriority(u32 channel, u32 val);

void sceEnableDMAChannel(u32 channel);
void sceDisableDMAChannel(u32 channel);

#define dmacman_IMPORTS_start DECLARE_IMPORT_TABLE(dmacman, 1, 1)
#define dmacman_IMPORTS_end END_IMPORT_TABLE

#define I_dmac_ch_set_madr DECLARE_IMPORT(4, dmac_ch_set_madr)
#define I_dmac_ch_get_madr DECLARE_IMPORT(5, dmac_ch_get_madr)
#define I_dmac_ch_set_bcr DECLARE_IMPORT(6, dmac_ch_set_bcr)
#define I_dmac_ch_get_bcr DECLARE_IMPORT(7, dmac_ch_get_bcr)
#define I_dmac_ch_set_chcr DECLARE_IMPORT(8, dmac_ch_set_chcr)
#define I_dmac_ch_get_chcr DECLARE_IMPORT(9, dmac_ch_get_chcr)
#define I_dmac_ch_set_tadr DECLARE_IMPORT(10, dmac_ch_set_tadr)
#define I_dmac_ch_get_tadr DECLARE_IMPORT(11, dmac_ch_get_tadr)
#define I_dmac_set_4_9_a DECLARE_IMPORT(12, dmac_set_4_9_a)
#define I_dmac_get_4_9_a DECLARE_IMPORT(13, dmac_get_4_9_a)
#define I_dmac_set_dpcr DECLARE_IMPORT(14, dmac_set_dpcr)
#define I_dmac_get_dpcr DECLARE_IMPORT(15, dmac_get_dpcr)
#define I_dmac_set_dpcr2 DECLARE_IMPORT(16, dmac_set_dpcr2)
#define I_dmac_get_dpcr2 DECLARE_IMPORT(17, dmac_get_dpcr2)
#define I_dmac_set_dpcr3 DECLARE_IMPORT(18, dmac_set_dpcr3)
#define I_dmac_get_dpcr3 DECLARE_IMPORT(19, dmac_get_dpcr3)
#define I_dmac_set_dicr DECLARE_IMPORT(20, dmac_set_dicr)
#define I_dmac_get_dicr DECLARE_IMPORT(21, dmac_get_dicr)
#define I_dmac_set_dicr2 DECLARE_IMPORT(22, dmac_set_dicr2)
#define I_dmac_get_dicr2 DECLARE_IMPORT(23, dmac_get_dicr2)
#define I_dmac_set_BF80157C DECLARE_IMPORT(24, dmac_set_BF80157C)
#define I_dmac_get_BF80157C DECLARE_IMPORT(25, dmac_get_BF80157C)
#define I_dmac_set_BF801578 DECLARE_IMPORT(26, dmac_set_BF801578)
#define I_dmac_get_BF801578 DECLARE_IMPORT(27, dmac_get_BF801578)
#define I_sceSetSliceDMA DECLARE_IMPORT(28, sceSetSliceDMA)
#define I_dmac_set_dma_chained_spu_sif0 DECLARE_IMPORT(29, dmac_set_dma_chained_spu_sif0)
#define I_dmac_set_dma_sif0 DECLARE_IMPORT(30, dmac_set_dma_sif0)
#define I_dmac_set_dma_sif1 DECLARE_IMPORT(31, dmac_set_dma_sif1)
#define I_sceStartDMA DECLARE_IMPORT(32, sceStartDMA)
#define I_sceSetDMAPriority DECLARE_IMPORT(33, sceSetDMAPriority)
#define I_sceEnableDMAChannel DECLARE_IMPORT(34, sceEnableDMAChannel)
#define I_sceDisableDMAChannel DECLARE_IMPORT(35, sceDisableDMAChannel)

// Backwards-compatibility defines

#define dmac_request(...) sceSetSliceDMA(__VA_ARGS__)
#define dmac_transfer(...) sceStartDMA(__VA_ARGS__)
#define dmac_ch_set_dpcr(...) sceSetDMAPriority(__VA_ARGS__)
#define dmac_enable(...) sceEnableDMAChannel(__VA_ARGS__)
#define dmac_disable(...) sceDisableDMAChannel(__VA_ARGS__)

#define I_dmac_request I_sceSetSliceDMA
#define I_dmac_transfer I_sceStartDMA
#define I_dmac_ch_set_dpcr I_sceSetDMAPriority
#define I_dmac_enable I_sceEnableDMAChannel
#define I_dmac_disable I_sceDisableDMAChannel

#ifdef __cplusplus
}
#endif

#endif /* __DMACMAN_H__ */
