/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __BITMAP_FSCK_H__
#define __BITMAP_FSCK_H__

typedef struct pfs_bitmap
{
    struct pfs_bitmap *next; // 0x00
    struct pfs_bitmap *prev; // 0x04
    u16 isDirty;             // 0x08
    u16 nused;               // 0x0A
    u32 index;               // 0x0C
    u32 *bitmap;             // 0x10
} pfs_bitmap_t;

extern pfs_cache_t *pfsBitmapReadPartition(pfs_mount_t *mount, u16 subpart, u32 chunk);
extern pfs_bitmap_t *pfsBitmapRead(u32 index);
extern void pfsBitmapFree(pfs_bitmap_t *bitmap);
extern u32 *pfsGetBitmapEntry(u32 index);
extern int pfsBitmapPartInit(u32 size);
extern int pfsBitmapInit(void);

#endif
