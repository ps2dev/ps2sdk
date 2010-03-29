/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Dan Peori <peori@oopo.net>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#ifndef __DMA_REGISTERS_H__
#define __DMA_REGISTERS_H__

#include <tamtypes.h>

#define DMA_REG_CTRL		(volatile u32 *)0x1000E000	// DMA Control Register
#define DMA_REG_STAT		(volatile u32 *)0x1000E010	// Interrupt Status Register
#define DMA_REG_PCR			(volatile u32 *)0x1000E020	// Priority Control Register
#define DMA_REG_SQWC		(volatile u32 *)0x1000E030	// Interleave Size Register
#define DMA_REG_RBSR		(volatile u32 *)0x1000E040	// Ring Buffer Size Register
#define DMA_REG_RBOR		(volatile u32 *)0x1000E050	// Ring Buffer Address Register
#define DMA_REG_STADR		(volatile u32 *)0x1000E060	// Stall Address Register
#define DMA_REG_ENABLER		(volatile u32 *)0x1000F520	// DMA Hold State Register
#define DMA_REG_ENABLEW		(volatile u32 *)0x1000F590	// DMA Hold Control Register

// Enable DMA Controller
#define DMAE_DISABLE 0
#define DMAE_ENABLE  1

// Enable Cycle Stealing
#define RELE_OFF 0
#define RELE_ON  1

// fifo drain?
#define MFD_OFF  0
#define MFD_RES  1
#define MFD_VIF  2
#define MFD_GIF  3

// stall source?
#define STS_UNSPEC 0
#define STS_SIF    1
#define STS_SPR    2
#define STS_IPU    3

// stall drain?
#define STD_OFF 0
#define STD_VIF 1
#define STD_GIF 2
#define STD_SIF 3

// Cycles to release control
#define RCYC_8   0
#define RCYC_16  1
#define RCYC_32  2
#define RCYC_64  3
#define RCYC_128 4
#define RCYC_256 5

#define DMA_SET_CTRL(DMAE,RELE,MFD,STS,STD,RCYC) \
	(u32)(A   & 0x00000001) <<  0 | (u32)(RELE & 0x00000001) <<  1 | \
	(u32)(MFD & 0x00000003) <<  2 | (u32)(STS  & 0x00000003) <<  4 | \
	(u32)(STD & 0x00000003) <<  6 | (u32)(RCYC & 0x00000007) <<  8

#define DMA_SET_STAT(CIS,SIS,MEIS,BEIS,CIM,SIM,MEIM) \
	(u32)((CIS)  & 0x000003FF) <<  0  | (u32)((SIS)  & 0x00000001) <<  13 | \
	(u32)((MEIS) & 0x00000001) <<  14 | (u32)((BEIS) & 0x00000001) <<  15 | \
	(u32)((CIM)  & 0x000003FF) <<  16 | (u32)((SIM)  & 0x00000001) <<  29 | \
	(u32)((MEIM) & 0x00000001) <<  30

#define DMA_SET_PCR(CPCOND,CDE,PCE) \
	(u32)((CPCOND) & 0x000003FF) <<  0 | (u32)((CDE) & 0x000003FF) << 16 | \
	(u32)((PCE)    & 0x00000001) << 31

#define DMA_SET_SQWC(SQWC,TQWC) \
	(u32)((SQWC) & 0x000000FF) <<  0 | (u32)((TQWC) & 0x000000FF) << 16

#define DMA_SET_RBOR(ADDR)  (u32)((ADDR) & 0x00007FFF)

#define DMA_SET_RBSR(RMSK)  (u32)((RMSK) & 0x00007FFF)

#define DMA_SET_STADR(ADDR) (u32)((ADDR) & 0x00007FFF)

#define DMA_SET_ENABLEW(A)  (u32)((A) & 0x00000001) << 16

#define DMA_SET_ENABLER(A)  (u32)((A) & 0x00000001) << 16


// Per-dma channel registers

#define DMA_SET_CHCR(DIR,MODE,ASP,TTE,TIE,STR,TAG) \
	(u32)((DIR) & 0x00000001) <<  0 | (u32)((MODE) & 0x00000003) <<  2 | \
	(u32)((ASP) & 0x00000003) <<  4 | (u32)((TTE ) & 0x00000001) <<  6 | \
	(u32)((TIE) & 0x00000001) <<  7 | (u32)((STR ) & 0x00000001) <<  8 | \
	(u32)((TAG) & 0x0000FFFF) << 16

#define DMA_SET_MADR(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_TADR(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_ASR0(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_ASR1(ADDR,SPR) \
	(u32)((ADDR) & 0x7FFFFFFF) <<  0 | (u32)((SPR) & 0x00000001) << 31

#define DMA_SET_SADR(ADDR) (u32)((ADDR) & 0x00003FFF)

#define DMA_SET_QWC(QWC)   (u32)((QWC)  & 0x0000FFFF)

#endif
