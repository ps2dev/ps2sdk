/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Kernel-based threads.
*/

#ifndef IOP_INTRMAN_H
#define IOP_INTRMAN_H

#include "types.h"
#include "irx.h"

enum iop_irq_list {
	IOP_IRQ_VBLANK = 0,
	IOP_IRQ_SBUS,

	IOP_IRQ_DEV9 = 0x0d,
	IOP_IRQ_SIO2 = 0x11,
	IOP_IRQ_USB = 0x16,
	IOP_IRQ_ILINK = 0x18,

	IOP_IRQ_DMA2 = 0x22,
	IOP_IRQ_DMA_DEV9 = 0x29
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
int QueryIntrContext();
#define I_QueryIntrContext DECLARE_IMPORT(23, QueryIntrContext)
int QueryIntrStack();
#define I_QueryIntrStack DECLARE_IMPORT(24, QueryIntrStack)

#endif /* IOP_INTRMAN_H */
