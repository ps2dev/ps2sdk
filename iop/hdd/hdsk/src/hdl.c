/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/*  Unofficial add-on to HDSK to support HDLoader games.
    The HDLoader format stores the LBAs and sizes of each partition at offset 0x100000
    of the main partition's extended attribute area. If the main partition or its sub-partitions
    are moved, then the game is broken.    */

#include <atad.h>
#include <errno.h>
#include <iomanX.h>
#include <stdio.h>
#include <sysclib.h>
#include <irx.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include "libapa.h"
#include "hdsk.h"

#define HDL_INFO_MAGIC       0xDEADFEED
#define HDL_GAME_DATA_OFFSET 0x100000

#define HDLFS_GAME_TITLE_LEN  160
#define HDLFS_STARTUP_PTH_LEN 60

typedef struct
{
    u32 part_offset; // In CD-ROM (2048-byte) sectors
    u32 data_start;  // In 512-byte HDD sectors
    u32 part_size;   // In bytes
} part_specs_t;

typedef struct // size = 1024 bytes
{
    u32 magic;
    u16 reserved;
    u16 version;
    s8 gamename[HDLFS_GAME_TITLE_LEN];
    u8 hdl_compat_flags;
    u8 ops2l_compat_flags;
    u8 dma_type;
    u8 dma_mode;
    s8 startup[HDLFS_STARTUP_PTH_LEN]; /* NOTE: The startup file name here must be without the ";1" suffix. */
    u32 layer1_start;
    u32 discType;
    s32 num_partitions;
    part_specs_t part_specs[65];
} hdl_game_info_t;

extern u8 IOBuffer[IOBUFFER_SIZE_SECTORS * 512];

int hdlUpdateGameSliceInfo(int device, u32 main, int part, u32 OldPartStart, u32 NewPartStart)
{
    u32 InfoLBA, DataOffset;
    int result;

    /* Note:    The APA specification states that there is a 4KB area used for storing the partition's information,
            before the extended attribute area. */
    InfoLBA = main + (HDL_GAME_DATA_OFFSET + 4096) / 512;

    if ((result = sceAtaDmaTransfer(device, IOBuffer, InfoLBA, sizeof(hdl_game_info_t) / 512, ATA_DIR_READ)) == 0) {
        if (((hdl_game_info_t *)IOBuffer)->magic == HDL_INFO_MAGIC) {
            DataOffset                                                 = ((hdl_game_info_t *)IOBuffer)->part_specs[part].data_start - OldPartStart;
            ((hdl_game_info_t *)IOBuffer)->part_specs[part].data_start = NewPartStart + DataOffset;
            result                                                     = sceAtaDmaTransfer(device, IOBuffer, InfoLBA, sizeof(hdl_game_info_t) / 512, ATA_DIR_WRITE);
        } else {
            result = -1;
        }
    }

    return result;
}
