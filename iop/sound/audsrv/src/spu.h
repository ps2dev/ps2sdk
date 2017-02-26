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
 * SPU definitions header
 */

#ifndef __SPU_H__
#define __SPU_H__

#define MAX_VOLUME               0x3fff

/* double buffer for spu. Must be twice the size of two channels,
 * 512 bytes each. (2 * 2 * 512 = 2048)
 */
#define SPU_BUF_SZ 2048

/* SD constants */

#define SD_CORE_0               0
#define SD_CORE_1               1

#define SD_INIT_COLD		0

#define SD_VOICE_KEYON        (0x15<<8)
#define SD_VOICE_KEYOFF       (0x16<<8)
#define SD_VOICE_START				((0x20<<8)|(0x1<<6))

#endif
