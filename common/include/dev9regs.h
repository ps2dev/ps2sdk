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
# DEV9 register definitions.
*/

#ifndef DEV9REGS_H
#define DEV9REGS_H

#include "types.h"

enum _dev9_regnames { 
       DEV9_R_1460, DEV9_R_1462, DEV9_R_1464, DEV9_R_1466, DEV9_R_1468,
       DEV9_R_146A, DEV9_R_146C, DEV9_R_REV,  DEV9_R_1470, DEV9_R_1472,
       DEV9_R_1474, DEV9_R_1476, DEV9_R_1478, DEV9_R_147A, DEV9_R_147C,
       DEV9_R_147E,
       DEV9_R_MAX };

typedef struct _dev9_regs {
	u16	val[DEV9_R_MAX];
} dev9_regs_t;

#define DEV9_REGBASE		0xbf801460
#define USE_DEV9_REGS		volatile dev9_regs_t *dev9_regs = \
	(volatile dev9_regs_t *)DEV9_REGBASE
#define DEV9_REG(reg)		dev9_regs->val[(reg)]

/* DEV9 DMAC registers.  */
#define DEV9_DMAC_BASE		0xbf801510
#define DEV9_DMAC_MADR		DEV9_DMAC_BASE
#define DEV9_DMAC_BCR		(DEV9_DMAC_BASE + 0x04)
#define DEV9_DMAC_CHCR		(DEV9_DMAC_BASE + 0x08)

#define DEV9_DEV9C_9566		0x20
#define DEV9_DEV9C_9611		0x30

#endif /* DEV9REGS_H */
