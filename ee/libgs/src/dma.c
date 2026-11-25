/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <errno.h>
#include <stdio.h>
#include <kernel.h>
#include <libgs.h>
#include <ee_regs.h>

#include "internal.h"

// Miscellaneous

/** GIF Channel Control Register */
#define gif_chcr	0x1000a000
/** Transfer Address Register */
#define gif_madr	0x1000a010
/** Transfer Size Register (in qwords) */
#define gif_qwc		0x1000a020
#define gif_tadr	0x1000a030

 #define DMA_TAG_REFE	0x00
 #define DMA_TAG_CNT	0x01
 #define DMA_TAG_NEXT	0x02
 #define DMA_TAG_REF	0x03
 #define DMA_TAG_REFS	0x04
 #define DMA_TAG_CALL	0x05
 #define DMA_TAG_RET	0x06
 #define DMA_TAG_END	0x07

typedef struct {
	/** Direction */
	unsigned direction	:1;
	/** Pad with zeros */
	unsigned pad1		:1; 
	/** Mode */
	unsigned mode		:2;
	/** Address stack pointer */
	unsigned asp		:2;
	/** Tag trasfer enable */
	unsigned tte		:1;
	/** Tag interrupt enable */
	unsigned tie		:1;
	/** start */
	unsigned start_flag	:1;
	/** Pad with more zeros */
	unsigned pad2		:7; 
	/** DMAtag */
	unsigned tag		:16;
}DMA_CHCR;

void GsDmaInit(void)
{
	/* This appears to have been based on code from Sony that initializes DMA channels 0-9, in bulk.
           Reset/init DMA CH 2 (GIF) only. */
	*((vu32 *)0x1000A080) = 0; // D2_SADR = 0. Documented to not exist, but is done.
	*R_EE_D2_CHCR = 0;
	*R_EE_D2_TADR = 0;
	*R_EE_D2_MADR = 0;
	*R_EE_D2_ASR1 = 0;
	*R_EE_D2_ASR0 = 0;
	*R_EE_D_STAT = 0xFF1F; // Clear all interrupt status under D_STAT, other than SIF0, SIF1 & SIF2.
	*R_EE_D_STAT &= 0xFF1F << 16; // Clear all interrupt masks under D_STAT, other SIF0, SIF1 & SIF2. Writing a 1 reverses the bit.
	*R_EE_D_CTRL = 0;
	*R_EE_D_PCR = 0;
	*R_EE_D_SQWC = 0;
	*R_EE_D_RBOR = 0;
	*R_EE_D_RBSR = 0;
	*R_EE_D_CTRL |= 1; // (DMAE 1)
}

void GsDmaSend(const void *addr, u32 qwords)
{
	DMA_CHCR		chcr;
	static char		spr;

	spr = ((u32)addr >= 0x70000000 && (u32)addr <= 0x70003fff) ? 1 : 0;

	*((vu32 *)(gif_madr)) = ( u32 )((( u32 )addr) & 0x7FFFFFFF) << 0 | (u32)((spr) & 0x00000001) << 31;;

	*((vu32 *)(gif_qwc)) = qwords;

	chcr.direction	=1;
	chcr.mode		=0;
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;

	// This prevents the compiler from assuming the values in addr are unused,
	// and that the writes to addr can be delayed until after this function call
	__asm__ ("":::"memory");

	*((volatile DMA_CHCR *)(gif_chcr)) = chcr;
}

void GsDmaSend_tag(const void *addr, u32 qwords, const GS_GIF_DMACHAIN_TAG *tag)
{
	DMA_CHCR		chcr;
	static char		spr;

	spr = ((u32)addr >= 0x70000000 && (u32)addr <= 0x70003fff) ? 1 : 0;

	*((vu32 *)(gif_madr)) = ( u32 )((( u32 )addr) & 0x7FFFFFFF) << 0 | (u32)((spr) & 0x00000001) << 31;
	*((vu32 *)(gif_qwc)) = qwords;
	*((vu32 *)(gif_tadr)) = ( u32 )((( u32 )tag) & 0x7FFFFFFF) << 0 | (u32)((0) & 0x00000001) << 31;

	chcr.direction	=1;
	chcr.mode		=1; //chain
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;

	// This prevents the compiler from assuming the values in addr are unused,
	// and that the writes to addr can be delayed until after this function call
	__asm__ ("":::"memory");

	*((volatile DMA_CHCR *)(gif_chcr)) = chcr;
}

void GsDmaWait(void)
{
	while(*R_EE_D2_CHCR & ((u32)1<<8));
	__asm__ ("":::"memory");
}
