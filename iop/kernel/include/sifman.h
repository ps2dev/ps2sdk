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
 * Low-level SIF DMA.
 */

#ifndef __SIFMAN_H__
#define __SIFMAN_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#define SIF_REG_ID_SYSTEM	0x80000000

enum _sif_regs {
	/** Main -> sub-CPU command buffer (MSCOM) */
	SIF_REG_MAINADDR = 1,
	/** Sub -> main-CPU command buffer (SMCOM) */
	SIF_REG_SUBADDR,
	/** Main -> sub-CPU flag (MSFLAG) */
	SIF_REG_MSFLAG,
	/** Sub -> main-CPU flag (SMFLAG) */
	SIF_REG_SMFLAG,
};

//Status bits for the SM and MS SIF registers
/** SIF initialized */
#define SIF_STAT_SIFINIT	0x10000
/** SIFCMD initialized */
#define SIF_STAT_CMDINIT	0x20000
/** Bootup completed */
#define SIF_STAT_BOOTEND	0x40000

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
void sceSifInit();

void sceSifSetDChain();

int sceSifSetDma(SifDmaTransfer_t *dmat, int count);
int sceSifDmaStat(int trid);

/* Compatibility names for use with ps2lib.  */
#define SifSetDChain sceSifSetDChain
#define SifSetDma sceSifSetDma
#define SifDmaStat sceSifDmaStat

/*
 * All of these except sceSifCheckInit() & sceSifSetDmaIntr() have been
 * aribitrarily named.
 */
void sceSifSetOneDma(SifDmaTransfer_t dmat);

void sceSifSendSync();
int sceSifIsSending();

void sceSifDma0Transfer(void *addr, int size, int mode);
void sceSifDma0Sync();
int sceSifDma0Sending();

void sceSifDma1Transfer(void *addr, int size, int mode);
void sceSifDma1Sync();
int sceSifDma1Sending();

void sceSifDma2Transfer(void *addr, int size, int mode);
void sceSifDma2Sync();
int sceSifDma2Sending();

/*
 * SBUS Main->Sub CPU status register
 */
u32 sceSifGetMSFlag();
u32 sceSifSetMSFlag(u32 val);

/*
 * SBUS Sub->Main CPU status register
 */
u32 sceSifGetSMFlag();
u32 sceSifSetSMFlag(u32 val);

/*
 * SBUS Main CPU DMA receive address
 */
u32 sceSifGetMainAddr();

/*
 * SBUS Sub CPU DMA receive address
 */
u32 sceSifGetSubAddr();
u32 sceSifSetSubAddr(u32 addr);

/*
 * Send a SBUS interrupt to the Main CPU
 */
void sceSifIntrMain();

int sceSifCheckInit();

void sceSifSetDmaIntrHandler(void (*handler)(void *), void *arg);
void sceSifResetDmaIntrHandler();

unsigned int sceSifSetDmaIntr(SifDmaTransfer_t *dmat, int count, void (*completioncb)(void *userdata), void *userdata);

#define sifman_IMPORTS_start DECLARE_IMPORT_TABLE(sifman, 1, 1)
#define sifman_IMPORTS_end END_IMPORT_TABLE

#define I_sceSifDma2Init DECLARE_IMPORT(4, sceSifDma2Init)
#define I_sceSifInit DECLARE_IMPORT(5, sceSifInit)
#define I_sceSifSetDChain DECLARE_IMPORT(6, sceSifSetDChain)
#define I_sceSifSetDma DECLARE_IMPORT(7, sceSifSetDma)
#define I_sceSifDmaStat DECLARE_IMPORT(8, sceSifDmaStat)
#define I_sceSifSetOneDma DECLARE_IMPORT(9, sceSifSetOneDma);
#define I_sceSifSendSync DECLARE_IMPORT(10, sceSifSendSync);
#define I_sceSifIsSending DECLARE_IMPORT(11, sceSifIsSending);
#define I_sceSifDma0Transfer DECLARE_IMPORT(12, sceSifDma0Transfer)
#define I_sceSifDma0Sync DECLARE_IMPORT(13, sceSifDma0Sync)
#define I_sceSifDma0Sending DECLARE_IMPORT(14, sceSifDma0Sending)
#define I_sceSifDma1Transfer DECLARE_IMPORT(15, sceSifDma1Transfer)
#define I_sceSifDma1Sync DECLARE_IMPORT(16, sceSifDma1Sync)
#define I_sceSifDma1Sending DECLARE_IMPORT(17, sceSifDma1Sending)
#define I_sceSifDma2Transfer DECLARE_IMPORT(18, sceSifDma2Transfer)
#define I_sceSifDma2Sync DECLARE_IMPORT(19, sceSifDma2Sync)
#define I_sceSifDma2Sending DECLARE_IMPORT(20, sceSifDma2Sending)
#define I_sceSifGetMSFlag DECLARE_IMPORT(21, sceSifGetMSFlag)
#define I_sceSifSetMSFlag DECLARE_IMPORT(22, sceSifSetMSFlag)
#define I_sceSifGetSMFlag DECLARE_IMPORT(23, sceSifGetSMFlag)
#define I_sceSifSetSMFlag DECLARE_IMPORT(24, sceSifSetSMFlag)
#define I_sceSifGetMainAddr DECLARE_IMPORT(25, sceSifGetMainAddr)
#define I_sceSifGetSubAddr DECLARE_IMPORT(26, sceSifGetSubAddr)
#define I_sceSifSetSubAddr DECLARE_IMPORT(27, sceSifSetSubAddr)
#define I_sceSifIntrMain DECLARE_IMPORT(28, sceSifIntrMain)
#define I_sceSifCheckInit DECLARE_IMPORT(29, sceSifCheckInit)
#define I_sceSifSetDmaIntrHandler DECLARE_IMPORT(30, sceSifSetDmaIntrHandler)
#define I_sceSifResetDmaIntrHandler DECLARE_IMPORT(31, sceSifResetDmaIntrHandler)
#define I_sceSifSetDmaIntr DECLARE_IMPORT(32, sceSifSetDmaIntr)

#ifdef __cplusplus
}
#endif

#endif /* __SIFMAN_H__ */
