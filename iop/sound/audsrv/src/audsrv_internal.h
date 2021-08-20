/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
*/

/**
 * @file
 * audsrv IOP server
 */

#ifndef __AUDSRV_C_H__
#define __AUDSRV_C_H__

/** Find the minimum value between A and B */
#define MIN(a,b) ((a) <= (b)) ? (a) : (b)

//RPC service ID
#define	AUDSRV_IRX            0x870884e

//DMA channel allocation
#define AUDSRV_VOICE_DMA_CH	0
#define AUDSRV_BLOCK_DMA_CH	1

#endif
