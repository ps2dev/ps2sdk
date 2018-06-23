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
 * Kernel-based threads.
 */

#ifndef __INTRMAN_H__
#define __INTRMAN_H__

#include <types.h>
#include <irx.h>

enum iop_irq_list {
	IOP_IRQ_VBLANK = 0,
	/** INUM_GM */
	IOP_IRQ_SBUS,
	/** INUM_CDROM */
	IOP_IRQ_CDVD,
	/** Original DMA controller interrupt (DMA channels 0-6) */
	IOP_IRQ_DMA,
	IOP_IRQ_RTC0,
	IOP_IRQ_RTC1,
	IOP_IRQ_RTC2,
	IOP_IRQ_SIO0,
	IOP_IRQ_SIO1,
	IOP_IRQ_SPU,
	IOP_IRQ_PIO,
	IOP_IRQ_EVBLANK,
	/** Not sure what this is, if INUM_CDROM is the interrupt CDVDMAN uses. */
	IOP_IRQ_DVD,
	/** INUM_PCMCIA */
	IOP_IRQ_DEV9,
	IOP_IRQ_RTC3,
	IOP_IRQ_RTC4,
	IOP_IRQ_RTC5,
	IOP_IRQ_SIO2,
	IOP_IRQ_HTR0,
	IOP_IRQ_HTR1,
	IOP_IRQ_HTR2,
	IOP_IRQ_HTR3,
	IOP_IRQ_USB,
	/** Expansion interface on the mainboard. Unused (and unpopulated) on most retail mainboards, but used by DECI2 in the TOOL unit as the MRP interface. In PS mode, INUM_PIO seems to be triggered instead.	*/
	IOP_IRQ_EXTR,
	/** INUM_FWRE */
	IOP_IRQ_ILINK,
	/** Firewire DMA */
	IOP_IRQ_FDMA,
	//There's a gap in interrupt numbers here.
	/** INUM_DMA_0 */	
	IOP_IRQ_DMA_MDEC_IN = 0x20,	
	/** INUM_DMA_1 */
	IOP_IRQ_DMA_MDEC_OUT,	
	/** INUM_DMA_2 */
	IOP_IRQ_DMA_SIF2,	
	/** INUM_DMA_3 */
	IOP_IRQ_DMA_CDVD,	
	/** INUM_DMA_4 */
	IOP_IRQ_DMA_SPU,	
	/** INUM_DMA_5 */
	IOP_IRQ_DMA_PIO,	
	/** INUM_DMA_6 */
	IOP_IRQ_DMA_GPU_OTC,	
	/** INUM_DMA_BERR (DMA bus error) */
	IOP_IRQ_DMA_BERR,	
	/** INUM_DMA_7 */
	IOP_IRQ_DMA_SPU2,	
	/** INUM_DMA_8 */
	IOP_IRQ_DMA_DEV9,	
	/** INUM_DMA_9 */
	IOP_IRQ_DMA_SIF0,	
	/** INUM_DMA_10 */
	IOP_IRQ_DMA_SIF1,	
	/** INUM_DMA_11 */
	IOP_IRQ_DMA_SIO2_IN,	
	/** INUM_DMA_12 */
	IOP_IRQ_DMA_SIO2_OUT,	

};

int RegisterIntrHandler(int irq, int mode, int (*handler)(void *), void *arg);
int ReleaseIntrHandler(int irq);

int EnableIntr(int irq);
int DisableIntr(int irq, int *res);

int CpuDisableIntr();
int CpuEnableIntr();

int CpuSuspendIntr(int *state);
int CpuResumeIntr(int state);

//Invokes a function in kernel mode via a syscall.
int CpuInvokeInKmode(void *function, ...);

//These are used to allow DECI2 to indicate that INTRMAN should not manage the interrupt.
//Disables interrupt handler dispatching for the specified interrupt (interrupt status is independent).
void DisableDispatchIntr(int irq);
//Enables interrupt handler dispatching for the specified interrupt (interrupt status is independent).
void EnableDispatchIntr(int irq);

/** 
 * @return 1 if within the interrupt context 
 */
int QueryIntrContext(void);
int QueryIntrStack(void);

int iCatchMultiIntr(void);

#define intrman_IMPORTS_start DECLARE_IMPORT_TABLE(intrman, 1, 2)
#define intrman_IMPORTS_end END_IMPORT_TABLE

#define I_RegisterIntrHandler DECLARE_IMPORT(4, RegisterIntrHandler)
#define I_ReleaseIntrHandler DECLARE_IMPORT(5, ReleaseIntrHandler)
#define I_EnableIntr DECLARE_IMPORT(6, EnableIntr)
#define I_DisableIntr DECLARE_IMPORT(7, DisableIntr)
#define I_CpuDisableIntr DECLARE_IMPORT(8, CpuDisableIntr)
#define I_CpuEnableIntr DECLARE_IMPORT(9, CpuEnableIntr)
#define I_CpuInvokeInKmode DECLARE_IMPORT(14, CpuInvokeInKmode);
#define I_DisableDispatchIntr DECLARE_IMPORT(15, DisableDispatchIntr);
#define I_EnableDispatchIntr DECLARE_IMPORT(16, EnableDispatchIntr);
#define I_CpuSuspendIntr DECLARE_IMPORT(17, CpuSuspendIntr)
#define I_CpuResumeIntr DECLARE_IMPORT(18, CpuResumeIntr)
#define I_QueryIntrContext DECLARE_IMPORT(23, QueryIntrContext)
#define I_QueryIntrStack DECLARE_IMPORT(24, QueryIntrStack)
#define I_iCatchMultiIntr DECLARE_IMPORT(25, iCatchMultiIntr)

#endif /* __INTRMAN_H__ */
