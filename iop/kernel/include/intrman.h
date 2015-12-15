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
# Kernel-based threads.
*/

#ifndef IOP_INTRMAN_H
#define IOP_INTRMAN_H

#include <types.h>
#include <irx.h>

enum iop_irq_list {
	IOP_IRQ_VBLANK = 0,
	IOP_IRQ_SBUS,	//INUM_GM
	IOP_IRQ_CDVD,	//INUM_CDROM
	IOP_IRQ_DMA,	//Original DMA controller interrupt (DMA channels 0-6)
	IOP_IRQ_RTC0,
	IOP_IRQ_RTC1,
	IOP_IRQ_RTC2,
	IOP_IRQ_SIO0,
	IOP_IRQ_SIO1,
	IOP_IRQ_SPU,
	IOP_IRQ_PIO,
	IOP_IRQ_EVBLANK,
	IOP_IRQ_DVD,	//Not sure what this is, if INUM_CDROM is the interrupt CDVDMAN uses.
	IOP_IRQ_DEV9,	//INUM_PCMCIA
	IOP_IRQ_RTC3,
	IOP_IRQ_RTC4,
	IOP_IRQ_RTC5,
	IOP_IRQ_SIO2,
	IOP_IRQ_HTR0,
	IOP_IRQ_HTR1,
	IOP_IRQ_HTR2,
	IOP_IRQ_HTR3,
	IOP_IRQ_USB,
	IOP_IRQ_EXTR,	/*	Expansion interface on the mainboard. Unused (and unpopulated) on most retail mainboards, but used by DECI2 in the TOOL unit as the MRP interface.
				In PS mode, INUM_PIO seems to be triggered instead.	*/
	IOP_IRQ_ILINK,	//INUM_FWRE
	IOP_IRQ_FDMA,	//Firewire DMA
	//There's a gap in interrupt numbers here.
	IOP_IRQ_DMA_MDEC_IN = 0x20,	//INUM_DMA_0
	IOP_IRQ_DMA_MDEC_OUT,	//INUM_DMA_1
	IOP_IRQ_DMA_SIF2,	//INUM_DMA_2
	IOP_IRQ_DMA_CDVD,	//INUM_DMA_3
	IOP_IRQ_DMA_SPU,	//INUM_DMA_4
	IOP_IRQ_DMA_PIO,	//INUM_DMA_5
	IOP_IRQ_DMA_GPU_OTC,	//INUM_DMA_6
	IOP_IRQ_DMA_BERR,	//INUM_DMA_BERR (DMA bus error)
	IOP_IRQ_DMA_SPU2,	//INUM_DMA_7
	IOP_IRQ_DMA_DEV9,	//INUM_DMA_8
	IOP_IRQ_DMA_SIF0,	//INUM_DMA_9
	IOP_IRQ_DMA_SIF1,	//INUM_DMA_10
	IOP_IRQ_DMA_SIO2_IN,	//INUM_DMA_11
	IOP_IRQ_DMA_SIO2_OUT,	//INUM_DMA_12
};

#define intrman_IMPORTS_start DECLARE_IMPORT_TABLE(intrman, 1, 2)
#define intrman_IMPORTS_end END_IMPORT_TABLE

int RegisterIntrHandler(int irq, int mode, int (*handler)(void *), void *arg);
#define I_RegisterIntrHandler DECLARE_IMPORT(4, RegisterIntrHandler)
int ReleaseIntrHandler(int irq);
#define I_ReleaseIntrHandler DECLARE_IMPORT(5, ReleaseIntrHandler)

int EnableIntr(int irq);
#define I_EnableIntr DECLARE_IMPORT(6, EnableIntr)
int DisableIntr(int irq, int *res);
#define I_DisableIntr DECLARE_IMPORT(7, DisableIntr)

int CpuDisableIntr();
#define I_CpuDisableIntr DECLARE_IMPORT(8, CpuDisableIntr)
int CpuEnableIntr();
#define I_CpuEnableIntr DECLARE_IMPORT(9, CpuEnableIntr)

int CpuSuspendIntr(int *state);
#define I_CpuSuspendIntr DECLARE_IMPORT(17, CpuSuspendIntr)
int CpuResumeIntr(int state);
#define I_CpuResumeIntr DECLARE_IMPORT(18, CpuResumeIntr)

/* Returns 1 if within the interrupt context */
int QueryIntrContext(void);
#define I_QueryIntrContext DECLARE_IMPORT(23, QueryIntrContext)
int QueryIntrStack(void);
#define I_QueryIntrStack DECLARE_IMPORT(24, QueryIntrStack)

#define intrman_IMPORTS \
	intrman_IMPORTS_start \
 \
 	I_RegisterIntrHandler \
	I_ReleaseIntrHandler \
 \
 	I_EnableIntr \
	I_DisableIntr \
 \
 	I_CpuDisableIntr \
	I_CpuEnableIntr \
 \
 	I_CpuSuspendIntr \
	I_CpuResumeIntr \
 \
 	I_QueryIntrContext \
 \
	intrman_IMPORTS_end

#endif /* IOP_INTRMAN_H */
