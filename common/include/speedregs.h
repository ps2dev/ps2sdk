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
# SPEED (ASIC on SMAP) register definitions.
*/

#ifndef SPEEDREGS_H
#define SPEEDREGS_H

#include "types.h"

#ifdef _EE
#define SPD_REGBASE			0xb4000000
#else
#define SPD_REGBASE			0xb0000000
#endif

#define USE_SPD_REGS	volatile u8 *spd_regbase = (volatile u8 *)SPD_REGBASE

#define	SPD_REG8(offset)	(*(volatile u8 *)(spd_regbase + (offset)))
#define	SPD_REG16(offset)	(*(volatile u16 *)(spd_regbase + (offset)))
#define	SPD_REG32(offset)	(*(volatile u32 *)(spd_regbase + (offset)))

#define SPD_R_REV			0x00
#define SPD_R_REV_1			0x02
#define SPD_R_REV_3			0x04
#define   SPD_CAPS_SMAP			(1<<0)
#define   SPD_CAPS_ATA			(1<<1)
#define   SPD_CAPS_UART			(1<<3)
#define   SPD_CAPS_DVR			(1<<4)
#define   SPD_CAPS_FLASH		(1<<5)
#define SPD_R_REV_8			0x0e

#define SPD_R_DMA_CTRL			0x24
#define SPD_R_INTR_STAT			0x28
#define SPD_R_INTR_MASK			0x2a
#define	  SPD_INTR_ATA0			(1<<0)
#define	  SPD_INTR_ATA1			(1<<1)
#define	  SPD_INTR_ATA			(SPD_INTR_ATA0|SPD_INTR_ATA1)	//mask=0x0003
//see smapregs.h for SMAP_INTR_*(TXDNV|RXDNV|TXEND|RXEND|EMAC3)		//mask=0x007C
#define   SPD_INTR_DVR			(1<<9)				//mask=0x0200
#define   SPD_INTR_UART			(1<<12)				//mask=0x1000
#define SPD_R_PIO_DIR			0x2c
#define SPD_R_PIO_DATA			0x2e
#define	  SPD_PP_DOUT		(1<<4)	/* Data output, read port */
#define	  SPD_PP_DIN		(1<<5)	/* Data input,  write port */
#define	  SPD_PP_SCLK		(1<<6)	/* Clock,       write port */
#define	  SPD_PP_CSEL		(1<<7)	/* Chip select, write port */
/* Operation codes */
#define	  SPD_PP_OP_READ	2
#define	  SPD_PP_OP_WRITE	1
#define	  SPD_PP_OP_EWEN	0
#define	  SPD_PP_OP_EWDS	0

#define SPD_R_XFR_CTRL			0x32
#define SPD_R_IF_CTRL			0x64
#define   SPD_IF_ATA_RESET	0x80
#define   SPD_IF_DMA_ENABLE	0x04
#define SPD_R_PIO_MODE			0x70
#define SPD_R_MWDMA_MODE		0x72
#define SPD_R_UDMA_MODE			0x74

#endif /* SPEEDREGS_H */
