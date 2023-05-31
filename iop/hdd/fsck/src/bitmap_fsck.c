/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <errno.h>
#include <stdio.h>
#include <sysclib.h>
#include <iomanX.h>
#include <hdd-ioctl.h>

#include "pfs-opt.h"
#include "libpfs.h"
#include "bitmap_fsck.h"

#define NUM_BITMAP_ENTRIES       5
#define BITMAP_BUFFER_SIZE       256
#define BITMAP_BUFFER_SIZE_BYTES (BITMAP_BUFFER_SIZE * 512)

extern u32 pfsMetaSize;
extern u32 pfsBlockSize;

static pfs_bitmap_t *pfsBitmapData;
static void *pfsBitmapBuffer;
static int PfsTempBitmapFD;

// 0x00004ce8
pfs_cache_t *pfsBitmapReadPartition(pfs_mount_t *mount, u16 subpart, u32 chunk)
{
    int result;
    return pfsCacheGetData(mount, subpart, chunk + (1 << mount->inode_scale) + (0x2000 >> pfsBlockSize), PFS_CACHE_FLAG_BITMAP, &result);
}

// 0x000052c0
static int pfsBitmapTransfer(pfs_bitmap_t *bitmap, int mode)
{
    hddIoctl2Transfer_t xferParams;
    int result;

    xferParams.sub    = 0;
    xferParams.sector = (bitmap->index << 8);
    xferParams.size   = BITMAP_BUFFER_SIZE;
    xferParams.mode   = mode;
    xferParams.buffer = bitmap->bitmap;

    if ((result = iomanX_ioctl2(PfsTempBitmapFD, HIOCTRANSFER, &xferParams, 0, NULL, 0)) < 0) {
        PFS_PRINTF("error: could not read/write bitmap.\n");
    } else {
        bitmap->isDirty = 0;
    }

    return result;
}

// 0x00005380
pfs_bitmap_t *pfsBitmapRead(u32 index)
{
    unsigned int i;
    pfs_bitmap_t *pBitmap;

    for (i = 1, pBitmap = NULL; i < NUM_BITMAP_ENTRIES; i++) {
        if (pfsBitmapData[i].index == index) {
            pBitmap = &pfsBitmapData[i];
            break;
        }
    }

    if (pBitmap != NULL) {
        if (pBitmap->nused == 0) {
            pBitmap = (pfs_bitmap_t *)pfsCacheUnLink((pfs_cache_t *)pBitmap);
        }

        pBitmap->nused++;
    } else {
        pBitmap = pfsBitmapData->next;
        if (pBitmap->isDirty != 0) {
            if (pfsBitmapTransfer(pBitmap, PFS_IO_MODE_WRITE) < 0) {
                return NULL;
            }
        }

        pBitmap->index   = index;
        pBitmap->isDirty = 0;
        pBitmap->nused   = 1;
        if (pfsBitmapTransfer(pBitmap, PFS_IO_MODE_READ) < 0) {
            return NULL;
        }
        pBitmap = (pfs_bitmap_t *)pfsCacheUnLink((pfs_cache_t *)pBitmap);
    }

    return pBitmap;
}

// 0x00005484
void pfsBitmapFree(pfs_bitmap_t *bitmap)
{
    if (bitmap->nused == 0) {
        PFS_PRINTF("error: unused cache returned\n");
    } else {
        bitmap->nused--;
        if (bitmap->nused == 0) {
            pfsCacheLink((pfs_cache_t *)pfsBitmapData->prev, (pfs_cache_t *)bitmap);
        }
    }
}

// 0x000054f0
u32 *pfsGetBitmapEntry(u32 index)
{
    pfs_bitmap_t *bitmap;
    u32 *result;

    if ((bitmap = pfsBitmapRead(index >> 20)) != NULL) {
        pfsBitmapFree(bitmap);
        result = &bitmap->bitmap[(index >> 5) & 0x7FFF];
    } else {
        result = NULL;
    }

    return result;
}

// 0x0000567c
int pfsBitmapPartInit(u32 size)
{
    int i, result;
    unsigned int bitmapCount;

    bitmapCount = (size >> 20) + (0 < ((size >> 3) & 0x0001FFFF));

    for (i = 1; i < NUM_BITMAP_ENTRIES; i++) {
        pfsBitmapData[i].isDirty = 0;
        pfsBitmapData[i].nused   = 0;
        pfsBitmapData[i].index   = i - 1;
        memset(pfsBitmapData[i].bitmap, 0, BITMAP_BUFFER_SIZE_BYTES);
    }

    if (bitmapCount >= NUM_BITMAP_ENTRIES) {
        for (i = 1; i < NUM_BITMAP_ENTRIES; i++) {
            if ((result = pfsBitmapTransfer(&pfsBitmapData[i], PFS_IO_MODE_WRITE)) < 0) {
                PFS_PRINTF("error: could not initialize bitmap.\n");
                return result;
            }
        }

        for (i = NUM_BITMAP_ENTRIES - 1; (u32)i < bitmapCount; i++) {
            pfsBitmapData[NUM_BITMAP_ENTRIES - 1].index = i;
            if ((result = pfsBitmapTransfer(&pfsBitmapData[NUM_BITMAP_ENTRIES - 1], PFS_IO_MODE_WRITE)) < 0) {
                PFS_PRINTF("error: could not initialize bitmap.\n");
                return result;
            }
        }

        result = 0;
    } else {
        result = 0;
    }

    return result;
}

// 0x000057bc
int pfsBitmapInit(void)
{
    u32 *bitmap;
    int i;

#ifdef FSCK100
    iomanX_remove("hdd0:_tmp");

    if ((PfsTempBitmapFD = iomanX_open("hdd0:_tmp,,,128M,PFS", O_CREAT | O_RDWR)) < 0)
        PFS_PRINTF("error: could not create temporary partition.\n");
#else
    if ((PfsTempBitmapFD = iomanX_open("hdd0:__mbr", O_RDWR)) < 0) {
        PFS_PRINTF("error: could not open mbr partition.\n");
        return PfsTempBitmapFD;
    }
#endif

    if ((pfsBitmapBuffer = pfsAllocMem((NUM_BITMAP_ENTRIES - 1) * BITMAP_BUFFER_SIZE_BYTES)) == NULL || (pfsBitmapData = pfsAllocMem(NUM_BITMAP_ENTRIES * sizeof(pfs_bitmap_t))) == NULL) {
        return -ENOMEM;
    }

    memset(pfsBitmapData, 0, NUM_BITMAP_ENTRIES * sizeof(pfs_bitmap_t));

    pfsBitmapData[0].next = pfsBitmapData;
    pfsBitmapData[0].prev = pfsBitmapData;

    bitmap = pfsBitmapBuffer;
    for (i = 1; i < NUM_BITMAP_ENTRIES; i++) {
        pfsBitmapData[i].bitmap = bitmap;
        bitmap                  = (u32 *)((u8 *)bitmap + BITMAP_BUFFER_SIZE_BYTES);
        pfsCacheLink((pfs_cache_t *)pfsBitmapData[0].prev, (pfs_cache_t *)&pfsBitmapData[i]);
    }

    return 0;
}
