/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 PS2DEV.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# IOP Co-processor 0(COP0) register definitions.
*/

#ifndef _IOP_COP0_DEFS_H_
#define _IOP_COP0_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define M_IOP_GET_CAUSE_EXCODE(__cause) (((__cause) >> 2) & 0x1F)

// IOP COP0 register names
#define IOP_COP0_BPC $3
#define IOP_COP0_BDA $5
#define IOP_COP0_DCIC $7
#define IOP_COP0_BADVADDR $8
#define IOP_COP0_BDAM $9
#define IOP_COP0_BPCM $11
#define IOP_COP0_STATUS $12

// IOP COP0 DCIC Register
#define IOP_DCIC_TR		0x80000000	/* Trap enable */
#define IOP_DCIC_UD		0x40000000	/* User debug enable */
#define IOP_DCIC_KD		0x20000000	/* Kernel debug enable */
#define IOP_DCIC_TE		0x10000000	/* Trace enable */
#define IOP_DCIC_DW		0x08000000	/* Enable data breakpoints on write */
#define IOP_DCIC_DR		0x04000000	/* Enable data breakpoints on read */
#define IOP_DCIC_DAE	0x02000000	/* Enable data addresss breakpoints */
#define IOP_DCIC_PCE	0x01000000	/* Enable instruction breakpoints */
#define IOP_DCIC_DE		0x00800000	/* Debug enable */
#define IOP_DCIC_DL		0x00008000	/* Data cache line invalidate */
#define IOP_DCIC_IL		0x00004000	/* Instruction cache line invalidate */
#define IOP_DCIC_D		0x00002000	/* Data cache invalidate enable */
#define IOP_DCIC_I		0x00001000	/* Instr. cache invalidate enable */
#define IOP_DCIC_T		0x00000020	/* Trace, set by CPU */
#define IOP_DCIC_W		0x00000010	/* Write reference, set by CPU */
#define IOP_DCIC_R		0x00000008	/* Read reference, set by CPU */
#define IOP_DCIC_DA		0x00000004	/* Data address, set by CPU */
#define IOP_DCIC_PC		0x00000002	/* Program counter, set by CPU */
#define IOP_DCIC_DB		0x00000001	/* Debug, set by CPU */

// IOP COP0 Cause Register
#define IOP_CAUSE_BD (1 << 31)

#ifdef __cplusplus
}
#endif

#endif // _IOP_COP0_DEFS_H_

