/*
 * dmacman.h - DMACMAN definitions and imports.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 *
 * Portions of this file are based off of the work [RO]man, Herben, and many
 * others involved in the "modules" project at http://ps2dev.pgamers.com/. 
 */

#ifndef IOP_DMACMAN_H
#define IOP_DMACMAN_H

#include "types.h"
#include "irx.h"

typedef struct _iop_dmac_chan {
	u32	madr;
	u32	bcr;
	u32	chcr;
} iop_dmac_chan_t;

/* CHCR flags */
#define DMAC_CHCR_30 (1<<30)
#define DMAC_CHCR_TR (1<<24)		/* TRansfer */
#define DMAC_CHCR_CO (1<<9)		/* COntinuous (?) */
#define DMAC_CHCR_DR (1<<0)		/* DiRection; 0 = to RAM, 1 = from RAM */

#define DMAC_TO_MEM	0
#define DMAC_FROM_MEM	1

enum _iop_dmac_ch {
	IOP_DMAC_MDECin, IOP_DMAC_MDECout, IOP_DMAC_SIF2, IOP_DMAC_CDVD,
	IOP_DMAC_SPU, IOP_DMAC_SPU2 = 7, IOP_DMAC_SIF0 = 9, IOP_DMAC_SIF1,
	IOP_DMA_SIO2in = 11, IOP_DMA_SIO2out = 12,
};

#define dmacman_IMPORTS_start DECLARE_IMPORT_TABLE(dmacman, 1, 1)
#define dmacman_IMPORTS_end END_IMPORT_TABLE

/* Note that these are far from official names.  */

/* Memory ADdRess  */
void dmac_ch_set_madr(u32 channel, u32 val);
#define I_dmac_ch_set_madr DECLARE_IMPORT(4, dmac_ch_set_madr)
u32 dmac_ch_get_madr(u32 channel);
#define I_dmac_ch_get_madr DECLARE_IMPORT(5, dmac_ch_get_madr)

/* Block Control Register  */
void dmac_ch_set_bcr(u32 channel, u32 val);
#define I_dmac_ch_set_bcr DECLARE_IMPORT(6, dmac_ch_set_bcr)
u32 dmac_ch_get_bcr(u32 channel);
#define I_dmac_ch_get_bcr DECLARE_IMPORT(7, dmac_ch_get_bcr)

/* CHannel Control Register  */
void dmac_ch_set_chcr(u32 channel, u32 val);
#define I_dmac_ch_set_chcr DECLARE_IMPORT(8, dmac_ch_set_chcr)
u32 dmac_ch_get_chcr(u32 channel);
#define I_dmac_ch_get_chcr DECLARE_IMPORT(9, dmac_ch_get_chcr)

/* Tag ADdRess */
void dmac_ch_set_tadr(u32 channel, u32 val);
#define I_dmac_ch_set_tadr DECLARE_IMPORT(10, dmac_ch_set_tadr)
u32 dmac_ch_get_tadr(u32 channel);
#define I_dmac_ch_get_tadr DECLARE_IMPORT(11, dmac_ch_get_tadr)

/* Dmac P??? Control Registers  */
void dmac_set_dpcr(u32 val);
#define I_dmac_set_dpcr DECLARE_IMPORT(14, dmac_set_dpcr)
u32 dmac_get_dpcr(void);
#define I_dmac_get_dpcr DECLARE_IMPORT(15, dmac_get_dpcr)
void dmac_set_dpcr2(u32 val);
#define I_dmac_set_dpcr2 DECLARE_IMPORT(16, dmac_set_dpcr2)
u32 dmac_get_dpcr2(void);
#define I_dmac_get_dpcr2 DECLARE_IMPORT(17, dmac_get_dpcr2)
void dmac_set_dpcr3(u32 val);
#define I_dmac_set_dpcr3 DECLARE_IMPORT(18, dmac_set_dpcr3)
u32 dmac_get_dpcr3(void);
#define I_dmac_get_dpcr3 DECLARE_IMPORT(19, dmac_get_dpcr3)

/* Dmac Interrupt Control Registers  */
void dmac_set_dicr(u32 val);
#define I_dmac_set_dicr DECLARE_IMPORT(20, dmac_set_dicr)
u32 dmac_get_dicr(void);
#define I_dmac_get_dicr DECLARE_IMPORT(21, dmac_get_dicr)
void dmac_set_dicr2(u32 val);
#define I_dmac_set_dicr2 DECLARE_IMPORT(22, dmac_set_dicr2)
u32 dmac_get_dicr2(void);
#define I_dmac_get_dicr2 DECLARE_IMPORT(23, dmac_get_dicr2)

/* Initialize the given channel and start the transfer.  Returns 1 if the
   transfer was started, and 0 on error.  */
int dmac_request(u32 channel, void * addr, u32 size, u32 count, int dir);
#define I_dmac_request DECLARE_IMPORT(28, dmac_request)

/* Start a transfer on the given channel.  */
void dmac_transfer(u32 channel);
#define I_dmac_transfer DECLARE_IMPORT(32, dmac_transfer)

/* Set the DPCRn value for a specific channel.  */
void dmac_ch_set_dpcr(u32 channel, u32 val);
#define I_dmac_ch_set_dpcr DECLARE_IMPORT(33, dmac_ch_set_dpcr)

void dmac_enable(u32 channel);
#define I_dmac_enable DECLARE_IMPORT(34, dmac_enable)
void dmac_disable(u32 channel);
#define I_dmac_disable DECLARE_IMPORT(35, dmac_disable)

#endif /* DMACMAN_H */
