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
# Hardware breakpoint functions
*/

#ifndef __HWBP_H__
#define __HWBP_H__

#include <tamtypes.h>

/* Instruction address breakpoint enable */
#define BPC_IAE	(1 << 31)
/* Data address (read) breakpoint enable */
#define BPC_DRE (1 << 30)
/* Data address (write) breakpoint enable */
#define BPC_DWE (1 << 29)
/* Data value breakpoint enable */
#define BPC_DVE (1 << 28)

/* Control of instruction breakpoint in user mode */
#define BPC_IUE (1 << 26)
/* Control of instruction breakpoint in supervisor mode */
#define BPC_ISE (1 << 25)
/* Control of instruction breakpoint in kernel mode */
#define BPC_IKE (1 << 24)
/* Control of instruction breakpoint in level 1 exception mode */
#define BPC_IXE (1 << 23)

/* Control of data breakpoint in user mode */
#define BPC_DUE (1 << 21)
/* Control of data breakpoint in supervisor mode */
#define BPC_DSE (1 << 20)
/* Control of data breakpoint in kernel mode */
#define BPC_DKE (1 << 19)
/* Control of data breakpoint in level 1 exception mode */
#define BPC_DXE (1 << 18)

/* Control of instruction trigger signal */
#define BPC_ITE (1 << 17)
/* Control of data trigger signal */
#define BPC_DTE (1 << 16)

/* Breakpoint exception enable */
#define BPC_BED (1 << 15)

/* Data write breakpoint occurred */
#define BPC_DWB (1 << 2)
/* Data read breakpoint occurred */
#define BPC_DRB (1 << 1)
/* Data instruction breakpoint occurred */
#define BPC_IAB (1 << 0)

/* Initialise the Breakpoint controller */
void InitBPC(void);
/* Set an instruction BP */
void SetInstructionBP(u32 addr, u32 mask, u32 options);
/* Set a data address BP */
/* Options is one or more of the BPC settings for data address */
/* Note you must set BPC_DRE and/or BPC_DWE in the options */
void SetDataAddrBP(u32 addr, u32 mask, u32 options);
/* Set a data value BP */
void SetDataValueBP(u32 addr, u32 mask, u32 value, u32 vmask, u32 options);

/* Get the BPC register */
u32  GetBPC(void);
/* Set the BPC register */
void SetBPC(u32 bpc);

/* Get the instruction address register */
u32  GetIAB(void);
/* Set the instruction address register */
void SetIAB(u32 val);
/* Get the instruction address mask register */
u32  GetIABM(void);
/* Set the instruction address mask register */
void SetIABM(u32 val);
/* Get the data address register */
u32  GetDAB(void);
/* Set the data address register */
void SetDAB(u32 val);
/* Get the data address register */
u32  GetDABM(void);
/* Set the data address mask register */
void SetDABM(u32 val);
/* Get the data value mask register */
u32  GetDVB(void);
/* Set the data value register */
void SetDVB(u32 val);
/* Get the data value mask register */
u32  GetDVBM(void);
/* Set the data value mask register */
void SetDVBM(u32 val);

#endif
