/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
#
# $Id$
# SPU definitions header
*/

#ifndef __SPU_H__
#define __SPU_H__

/**
 * \file spu.h
 * \author gawd (Gil Megidish)
 * \date 05-09-05
 */

#define MAX_VOLUME               0x3fff

/* double buffer for spu. Must be twice the size of two channels,
 * 512 bytes each. (2 * 2 * 512 = 2048)
 */
#define SPU_BUF_SZ 2048

#define SPU_IRQ                       9
#define SPU_DMA_CHN0_IRQ             36
#define SPU_DMA_CHN1_IRQ             40

/* SD constants */

#define SD_CORE_0               0
#define SD_CORE_1               1
#define SD_P_MVOLL              ((0x09 << 8) + (0x01 << 7))
#define SD_P_MVOLR              ((0x0a << 8) + (0x01 << 7))
#define SD_P_AVOLL              ((0x0d << 8) + (0x01 << 7))
#define SD_P_AVOLR              ((0x0e << 8) + (0x01 << 7))
#define SD_P_BVOLL              ((0x0f << 8) + (0x01 << 7))
#define SD_P_BVOLR              ((0x10 << 8) + (0x01 << 7))

#define SD_INIT_COLD		0

#define SD_BLOCK_TRANS_WRITE	0x00
#define SD_BLOCK_TRANS_READ	0x01
#define SD_BLOCK_TRANS_STOP	0x02
#define SD_BLOCK_LOOP		0x10

// VoiceTrans
#define SD_VOICE_TRANS_WRITE		0
#define SD_VOICE_TRANS_READ			1
#define SD_VOICE_TRANS_STOP			2
#define	SD_VOICE_TRANS_MODE_DMA		0
#define SD_VOICE_TRANS_MODE_IO		8

#define SD_S_ENDX       (0x17<<8)

#define SD_VOICE_KEYON        (0x15<<8)
#define SD_VOICE_KEYOFF       (0x16<<8)
#define SD_VOICE_START				((0x20<<8)|(0x1<<6))
#define SD_VOICE_PITCH				(0x2<<8)
#define SD_VOICE_VOLL					(0x0<<8)
#define SD_VOICE_VOLR					(0x1<<8)

#endif
