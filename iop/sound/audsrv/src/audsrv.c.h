/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# audsrv IOP server.
*/

#ifndef __AUDSRV_C_H__
#define __AUDSRV_C_H__

/**
 * \file audsrv.c.h
 * \author gawd (Gil Megidish)
 * \date 04-24-05
 */

#define MAX_VOLUME               0x3fff

/* double buffer for spu. Must be twice the size of two channels,
 * 512 bytes each. (2 * 2 * 512 = 2048)
 */
#define SPU_BUF_SZ 2048

#define SPU_IRQ                       9
#define SPU_DMA_CHN0_IRQ             36
#define SPU_DMA_CHN1_IRQ             40
#define	AUDSRV_IRX            0x870884d

/* SD constants */

#define SD_CORE_0               0
#define SD_CORE_1               1
#define SD_P_BVOLL              ((0x0f << 8) + (0x01 << 7))
#define SD_P_BVOLR              ((0x10 << 8) + (0x01 << 7))
#define SD_P_MVOLL              ((0x09 << 8) + (0x01 << 7))
#define SD_P_MVOLR              ((0x0a << 8) + (0x01 << 7))

#define SD_INIT_COLD		0
//#define SD_C_NOISE_CLK		(4<<1)

#define SD_BLOCK_TRANS_WRITE	0x00
#define SD_BLOCK_TRANS_READ	0x01
#define SD_BLOCK_TRANS_STOP	0x02
#define SD_BLOCK_LOOP		0x10

#endif
