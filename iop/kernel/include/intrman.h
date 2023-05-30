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

    /** R3000A Software Interrupt 1 (Used by DECI2). */
    IOP_IRQ_SW1 = 0x3E,
    /** R3000A Software Interrupt 2 (Used by DRVTIF of DECI2) */
    IOP_IRQ_SW2
};

/**
 * Register an interrupt handler for the specified interrupt.
 * @param irq Interrupt cause to register an interrupt handler for.
 * @param mode Specifies the registers that will be preserved before the interrupt handler is run. The more registers are preserved, the slower the operation.
 *             Mode 0: $at, $v0, $v1, $a0, $a1, $a2, $a3 and $ra can be used.
 *             Mode 1: All mode 0 registers, as well as $t0-$t9, $gp and $fp can be used.
 *             Mode 2: All mode 1 registers, as well as $s0-$s7 can be used.
 * @param handler A pointer to the interrupt handler that will be associated with the interrupt.
 * @param arg An optional pointer to data that will be passed to the interrupt handler, whenever it is to be invoked.
 * @return 0 on success, non-zero error code on failure.
 */
int RegisterIntrHandler(int irq, int mode, int (*handler)(void *), void *arg);

/**
 * Releases (deregisters) the interrupt handler for the specified interrupt.
 * @param irq Interrupt cause to release the interrupt handler for.
 * @return 0 on success, non-zero error code on failure.
 */
int ReleaseIntrHandler(int irq);

/**
 * Enables (unmasks) the specified hardware interrupt cause.
 * @param irq Interrupt cause to enable.
 * @return Returns 0 on success, non-zero error code on failure.
 */
int EnableIntr(int irq);

/**
 * Disables (masks) the specified hardware interrupt cause.
 * @param irq Interrupt cause to disable.
 * @param res Pointer to a variable to receive the interrupt number of the interrupt that was disabled.
 * @return Returns 0 on success, non-zero error code on failure.
 */
int DisableIntr(int irq, int *res);

/**
 * Disables interrupts, regardless of the current statue. This is deprecated.
 * The interrupt mask registers for each interrupt cause will not be changed.
 * May be called from an interrupt or thread context.
 * @return Returns 0 on success, non-zero error code on failure.
 */
int CpuDisableIntr();
/**
 * Enables interrupts, regardless of the current state. This is deprecated.
 * The interrupt mask registers for each interrupt cause will not be changed.
 * May be called from an interrupt or thread context.
 * @return Returns 0 on success, non-zero error code on failure.
 */
int CpuEnableIntr();

/**
 * Disables interrupts.
 * The interrupt mask registers for each interrupt cause will not be changed.
 * May be called from an interrupt or thread context.
 * @param state A pointer to a variable that will store the current interrupt status. Even if KE_CPUDI is returned, state will be set appropriately.
 * @return Returns 0 on success, non-zero error code on failure.
 * @see CpuResumeIntr()
 */
int CpuSuspendIntr(int *state);

/**
 * Enables interrupts.
 * The interrupt mask registers for each interrupt cause will not be changed.
 * May be called from an interrupt or thread context.
 * @param state The previous state of interrupts, as indicated by the preceeding call to CpuSuspendIntr().
 * @return Returns 0 on success, non-zero error code on failure.
 * @see CpuSuspendIntr()
 */
int CpuResumeIntr(int state);

/**
 * Invokes a function in kernel mode via a syscall handler. This is usually used for synchronization between interrupt and DECI2 contexts.
 * @param function A pointer to the function to call. Specify other arguments for the function, after function.
 * @return The return value of the function.
 */
int CpuInvokeInKmode(void *function, ...);

/**
 * Disables dispatching of the interrupt handler, by INTRMAN. Used by DECI2, when DECI2RS is to manage the SIF2 and SBUS interrupts.
 * This does not change the interrupt mask status.
 * @param irq The interrupt to mask in software.
 */
void DisableDispatchIntr(int irq);
/**
 * Enables dispatching of the interrupt handler, by INTRMAN.
 * This does not change the interrupt mask status.
 * @param irq The interrupt to unmask in software.
 */
void EnableDispatchIntr(int irq);

/**
 * Indicates whether execution is currently within an interrupt or thread context.
 * @return 1 if within the interrupt context, 0 if not.
 */
int QueryIntrContext(void);

/**
 * Indicates whether the specified stack pointer is within the interrupt stack.
 * @param sp The stack pointer to check.
 * @return 1 if the specified stack pointer is within the interrupt stack, 0 if not.
 */
int QueryIntrStack(void *sp);

int iCatchMultiIntr(void);

/**
 * These set callback functions for thread support
 * Only the thread manager should set these
 */
int SetNewCtxCb(void *cb);
int SetShouldPreemptCb(void *cb);

#define intrman_IMPORTS_start DECLARE_IMPORT_TABLE(intrman, 1, 2)
#define intrman_IMPORTS_end   END_IMPORT_TABLE

#define I_RegisterIntrHandler DECLARE_IMPORT(4, RegisterIntrHandler)
#define I_ReleaseIntrHandler  DECLARE_IMPORT(5, ReleaseIntrHandler)
#define I_EnableIntr          DECLARE_IMPORT(6, EnableIntr)
#define I_DisableIntr         DECLARE_IMPORT(7, DisableIntr)
#define I_CpuDisableIntr      DECLARE_IMPORT(8, CpuDisableIntr)
#define I_CpuEnableIntr       DECLARE_IMPORT(9, CpuEnableIntr)
#define I_CpuInvokeInKmode    DECLARE_IMPORT(14, CpuInvokeInKmode);
#define I_DisableDispatchIntr DECLARE_IMPORT(15, DisableDispatchIntr);
#define I_EnableDispatchIntr  DECLARE_IMPORT(16, EnableDispatchIntr);
#define I_CpuSuspendIntr      DECLARE_IMPORT(17, CpuSuspendIntr)
#define I_CpuResumeIntr       DECLARE_IMPORT(18, CpuResumeIntr)
#define I_QueryIntrContext    DECLARE_IMPORT(23, QueryIntrContext)
#define I_QueryIntrStack      DECLARE_IMPORT(24, QueryIntrStack)
#define I_iCatchMultiIntr     DECLARE_IMPORT(25, iCatchMultiIntr)
#define I_SetNewCtxCb         DECLARE_IMPORT(28, SetNewCtxCb)
#define I_SetShouldPreemptCb  DECLARE_IMPORT(30, SetShouldPreemptCb)

#endif /* __INTRMAN_H__ */
