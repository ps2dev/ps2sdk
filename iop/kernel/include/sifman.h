/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Low-level SIF DMA.
*/

#ifndef IOP_SIFMAN_H
#define IOP_SIFMAN_H

#include "types.h"
#include "irx.h"

typedef struct t_SifDmaTransfer
{
	void	*src;
	void	*dest;
	int	size;
	int	attr;
} SifDmaTransfer_t;

/* Modes for DMA transfers */
#define SIF_DMA_FROM_IOP	0x0
#define SIF_DMA_TO_IOP		0x1
#define SIF_DMA_FROM_EE		0x0
#define SIF_DMA_TO_EE		0x1

#define SIF_DMA_INT_I		0x2
#define SIF_DMA_INT_O		0x4
#define SIF_DMA_SPR		0x8
#define SIF_DMA_BSN		0x10 /* ? what is this? */
#define SIF_DMA_TAG		0x20

#define sifman_IMPORTS_start DECLARE_IMPORT_TABLE(sifman, 1, 1)
#define sifman_IMPORTS_end END_IMPORT_TABLE

/*
 * Note: A lot of these names are _arbitrary_, since there was no debug info
 * that gave away the "real" names.  Even though I dislike the SCEI naming
 * convention, I believe routines within the same module should be similarily
 * named, so I've followed that convention.
 *
 * Perhaps someday, someone will come forward with the proper names, but I
 * don't really care.
 */

void sceSifDma2Init();
#define I_sceSifDma2Init DECLARE_IMPORT(4, sceSifDma2Init)
void sceSifInit();
#define I_sceSifInit DECLARE_IMPORT(5, sceSifInit)

void sceSifSetDChain();
#define I_sceSifSetDChain DECLARE_IMPORT(6, sceSifSetDChain)

int sceSifSetDma(SifDmaTransfer_t *dmat, int count);
#define I_sceSifSetDma DECLARE_IMPORT(7, sceSifSetDma)
int sceSifDmaStat(int trid);
#define I_sceSifDmaStat DECLARE_IMPORT(8, sceSifDmaStat)

/* Compatibility names for use with ps2lib.  */
#define SifSetDChain sceSifSetDChain
#define SifSetDma sceSifSetDma
#define SifDmaStat sceSifDmaStat

/* 
 * All of these except sceSifCheckInit() & sceSifSetDmaIntr() have been
 * aribitrarily named.
 */
void sceSifSetOneDma(SifDmaTransfer_t dmat);
#define I_sceSifSetOneDma DECLARE_IMPORT(9, sceSifSetOneDma);

void sceSifDma0Transfer(void *addr, int size, int mode);
#define I_sceSifDma0Transfer DECLARE_IMPORT(12, sceSifDma0Transfer)
void sceSifDma0Sync();
#define I_sceSifDma0Sync DECLARE_IMPORT(13, sceSifDma0Sync)
int sceSifDma0Sending();
#define I_sceSifDma0Sending DECLARE_IMPORT(14, sceSifDma0Sending)

void sceSifDma1Transfer(void *addr, int size, int mode);
#define I_sceSifDma1Transfer DECLARE_IMPORT(15, sceSifDma1Transfer)
void sceSifDma1Sync();
#define I_sceSifDma1Sync DECLARE_IMPORT(16, sceSifDma1Sync)
int sceSifDma1Sending();
#define I_sceSifDma1Sending DECLARE_IMPORT(17, sceSifDma1Sending)

void sceSifDma2Transfer(void *addr, int size, int mode);
#define I_sceSifDma2Transfer DECLARE_IMPORT(18, sceSifDma2Transfer)
void sceSifDma2Sync();
#define I_sceSifDma2Sync DECLARE_IMPORT(19, sceSifDma2Sync)
int sceSifDma2Sending();
#define I_sceSifDma2Sending DECLARE_IMPORT(20, sceSifDma2Sending)

/*
 * SBUS Main->Sub CPU status register
 */
u32 sceSifGetMSFlag();
#define I_sceSifGetMSFlag DECLARE_IMPORT(21, sceSifGetMSFlag)
u32 sceSifSetMSFlag(u32 val);
#define I_sceSifSetMSFlag DECLARE_IMPORT(22, sceSifSetMSFlag)

/*
 * SBUS Sub->Main CPU status register
 */
u32 sceSifGetSMFlag();
#define I_sceSifGetSMFlag DECLARE_IMPORT(23, sceSifGetSMFlag)
u32 sceSifSetSMFlag(u32 val);
#define I_sceSifSetSMFlag DECLARE_IMPORT(24, sceSifSetSMFlag)

/*
 * SBUS Main CPU DMA receive address
 */
u32 sceSifGetMainAddr();
#define I_sceSifGetMainAddr DECLARE_IMPORT(25, sceSifGetMainAddr)

/*
 * SBUS Sub CPU DMA receive address
 */
u32 sceSifGetSubAddr();
#define I_sceSifGetSubAddr DECLARE_IMPORT(26, sceSifGetSubAddr)
u32 sceSifSetSubAddr(u32 addr);
#define I_sceSifSetSubAddr DECLARE_IMPORT(27, sceSifSetSubAddr)

/*
 * Send a SBUS interrupt to the Main CPU
 */
void sceSifIntrMain();
#define I_sceSifIntrMain DECLARE_IMPORT(28, sceSifIntrMain)

int sceSifCheckInit();
#define I_sceSifCheckInit DECLARE_IMPORT(29, sceSifCheckInit)

void sceSifSetDmaIntrHandler(void (*handler)(void *), void *arg);
#define I_sceSifSetDmaIntrHandler DECLARE_IMPORT(30, sceSifSetDmaIntrHandler)
void sceSifResetDmaIntrHandler();
#define I_sceSifResetDmaIntrHandler DECLARE_IMPORT(31, sceSifResetDmaIntrHandler)

#endif /* IOP_SIFMAN_H */
