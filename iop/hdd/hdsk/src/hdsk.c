/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <atad.h>
#include <errno.h>
#include <iomanX.h>
#include <loadcore.h>
#include <kerr.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <irx.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include "libapa.h"
#include "hdsk-devctl.h"
#include "hdsk.h"
#include "sim.h"
#include "misc_hdsk.h"
#ifdef HDSK_SUPPORT_HDLFS
#include "hdl.h"
#endif

#ifdef _IOP
IRX_ID("hdsk", APA_MODVER_MAJOR, APA_MODVER_MINOR);
#endif

static apa_device_t HddInfo[2] = {
    {0, 0, 0, 3},
    {0, 0, 0, 3},
};

static int hdskEventFlagID; // 0x000276c8
static int hdskThreadID;    // 0x000276cc
static int hdskStopFlag;
static int hdskStatus;
static u32 hdskProgress;

u8 IOBuffer[IOBUFFER_SIZE_SECTORS * 512];

#define HDSK_MIN_PART_SIZE 0x00040000

static int hdskRemoveTmp(int device)
{
    apa_cache_t *clink;
    char partition[APA_IDMAX];
    u32 start;
    int result;

    clink = apaCacheGetHeader(device, 0, APA_IO_MODE_READ, &result);
    memset(partition, 0, sizeof(partition));
    strcpy(partition, "_tmp");

    while (clink != NULL) {
        if (!(clink->header->flags & APA_FLAG_SUB)) {
            if (!memcmp(clink->header->id, partition, sizeof(clink->header->id))) {
                break;
            }
        }

        clink = apaGetNextHeader(clink, &result);
    }

    if (result == 0 && clink != NULL) {
        int sub;

        APA_PRINTF("remove _tmp\n");
        sub                 = clink->header->nsub;
        clink->header->nsub = 0;

        clink->flags |= APA_CACHE_FLAG_DIRTY;
        start = clink->header->start + 0x2000;
        apaCacheFlushAllDirty(device);

        for (--sub; sub != -1; sub--) {
            apa_cache_t *clinkSub;

            if ((clinkSub = apaCacheGetHeader(device, clink->header->subs[sub].start, APA_IO_MODE_READ, &result)) != NULL) {
                if ((result = apaDelete(clinkSub)) != 0) {
                    break;
                }
            }
        }

        apaDelete(clink);
        clink = apaCacheAlloc();
        memset(clink->header, 0, sizeof(apa_header_t));
        sceAtaDmaTransfer(device, clink->header, start, 2, ATA_DIR_WRITE);
        apaCacheFree(clink);
    }

    return result;
}

static apa_cache_t *hdskFindEmptyPartition(int device, u32 size)
{
    int result;
    apa_cache_t *clink;

    clink = apaCacheGetHeader(device, 0, APA_IO_MODE_READ, &result);
    while (clink != NULL) {
        if (clink->header->length != size || clink->header->type != APA_TYPE_FREE) {
            clink = apaGetNextHeader(clink, &result);
            if (hdskStopFlag) {
                apaCacheFree(clink);
                return NULL;
            }
        } else {
            break;
        }
    }

    if (clink != NULL) {
        APA_PRINTF("found empty partition at %08lx, size %08lx.\n", clink->header->start, clink->header->length);
    }

    return clink;
}

static int hdskIsPartitionOfSize(int device, u32 sector, u32 size)
{
    apa_cache_t *clink;
    int result, returnValue;

    returnValue = 1;
    if ((clink = apaCacheGetHeader(device, sector, APA_IO_MODE_READ, &result)) != NULL) {
        if (clink->header->type == APA_TYPE_FREE) {
            returnValue = 0 < (clink->header->length ^ size);
        }

        apaCacheFree(clink);
    }

    return returnValue;
}

static u32 hdskGetPartitionPrev(int device, u32 size, u32 start)
{
    int result;
    apa_cache_t *clink;

    (void)size;
    (void)start;

    if ((clink = apaCacheGetHeader(device, 0, APA_IO_MODE_READ, &result)) != NULL) {
        apaCacheFree(clink);
        return (clink->header->prev);
    }

    return 0;
}

static apa_cache_t *hdskFindLastUsedPartition(int device, u32 size, u32 start, int mode)
{
    int result;
    apa_cache_t *clink;
    u32 previous, DoubleSize, partition;

    clink      = NULL;
    previous   = hdskGetPartitionPrev(device, size, start);
    DoubleSize = size * 2;
    while (start < previous) {
        if ((clink = apaCacheGetHeader(device, previous, APA_IO_MODE_READ, &result)) != NULL) {
            if (clink->header->length == size && clink->header->type != APA_TYPE_FREE) {
                if (mode) {
                    if (clink->header->start % DoubleSize == 0) {
                        if ((partition = clink->header->next) == 0) {
                            break;
                        }
                    } else {
                        partition = clink->header->prev;
                    }

                    if (!hdskIsPartitionOfSize(device, partition, size)) {
                        break;
                    }
                } else {
                    break;
                }
            }
        } else {
            break;
        }

        previous = clink->header->prev;
        apaCacheFree(clink);

        if (hdskStopFlag) {
            break;
        }
        clink = NULL;
    }

    if (clink != NULL) {
        APA_PRINTF("found last used partition at %08lx, size %08lx.\n", clink->header->start, clink->header->length);
    }

    return clink;
}

static int hdskFindPartitionOfSize(int device, apa_header_t *start, u32 MinSize, int mode)
{
    u32 next;
    u32 BlockSize;
    int result;
    apa_cache_t *clink;

    next      = start->next;
    BlockSize = start->length;
    while (BlockSize < MinSize && next != 0) {
        if ((clink = apaCacheGetHeader(device, next, APA_IO_MODE_READ, &result)) != NULL) {
            if (clink->header->type == APA_TYPE_FREE) {
                break;
            }

            next = clink->header->next;
            BlockSize += clink->header->length;
            apaCacheFree(clink);
        } else {
            break;
        }
    }

    if (BlockSize == MinSize) {
        if (mode != 0) {
            if (start->start % (MinSize * 2) != 0) {
                return ((hdskIsPartitionOfSize(device, start->prev, MinSize) != 0) ? 0 : 1);
            } else {
                if (next != 0) {
                    return ((hdskIsPartitionOfSize(device, next, MinSize) != 0) ? 0 : 1);
                }
            }
        } else {
            return 1;
        }
    }

    return 0;
}

static apa_cache_t *hdskFindLastUsedBlock(int device, u32 MinSize, u32 start, int mode)
{
    int result;
    apa_cache_t *last;
    u32 NextStart, sector;

    last   = NULL;
    sector = start;
    if ((NextStart = hdskGetPartitionPrev(device, MinSize, start)) != 0) {
        while (sector < NextStart) {
            if ((last = apaCacheGetHeader(device, NextStart, 0, &result)) != NULL) {
                if (last->header->start % MinSize != 0) {
                    if (last->header->length < MinSize && last->header->type != APA_TYPE_FREE) {
                        if (hdskFindPartitionOfSize(device, last->header, MinSize, mode) != 0) {
                            break;
                        }
                    }
                }

                // 0x000006bc
                NextStart = last->header->prev;
                apaCacheFree(last);
                if (hdskStopFlag) {
                    break;
                }
                last = NULL;
            } else {
                return last;
            }
        }
    }
    if (last != NULL) {
        APA_PRINTF("found last used block of partitions at %08lx, size %08lx.\n", last->header->start, last->header->length);
    }

    return last;
}

static int CopyPartition(int device, apa_header_t *dest, apa_header_t *start)
{
    u32 blocks, i;
    int result;

    blocks = dest->length / IOBUFFER_SIZE_SECTORS;
    APA_PRINTF("copy start...");

    // Copy data, but skip the APA header.
    result = sceAtaDmaTransfer(device, IOBuffer, start->start + 2, IOBUFFER_SIZE_SECTORS - 2, ATA_DIR_READ) == 0 ? 0 : -EIO;

    if (result == 0) {
        result = sceAtaDmaTransfer(device, IOBuffer, dest->start + 2, IOBUFFER_SIZE_SECTORS - 2, ATA_DIR_WRITE) == 0 ? 0 : -EIO;

        if (result == 0) {
            hdskProgress += IOBUFFER_SIZE_SECTORS;

            for (i = 1; i < blocks; i++) {
                result = sceAtaDmaTransfer(device, IOBuffer, start->start + i * IOBUFFER_SIZE_SECTORS, IOBUFFER_SIZE_SECTORS, ATA_DIR_READ) == 0 ? 0 : -EIO;

                if (result == 0) {
                    result = sceAtaDmaTransfer(device, IOBuffer, dest->start + i * IOBUFFER_SIZE_SECTORS, IOBUFFER_SIZE_SECTORS, ATA_DIR_WRITE) == 0 ? 0 : -EIO;

                    if (result == 0) {
                        hdskProgress += IOBUFFER_SIZE_SECTORS;
                        if (hdskStopFlag) {
                            break;
                        }
                    } else {
                        APA_PRINTF("error: write failed at %08lx.\n", dest->start + i * IOBUFFER_SIZE_SECTORS);
                        break;
                    }
                } else {
                    APA_PRINTF("error: read failed at %08lx.\n", start->start + i * IOBUFFER_SIZE_SECTORS);
                    break;
                }
            }

            printf("done\n");
        }
    }

    return result;
}

static int SwapPartition(int device, apa_cache_t *dest, apa_cache_t *start)
{
    apa_cache_t *clink[64];
    int result, i;
    u32 StartSector, next, prev;

    StartSector = dest->header->start;
    next        = dest->header->next;
    prev        = dest->header->prev;

    APA_PRINTF("swap %s partition start...", (start->header->flags & APA_FLAG_SUB) ? "sub" : "main");

    memcpy(dest->header, start->header, sizeof(apa_header_t));
    dest->header->start = StartSector;
    dest->header->next  = next;
    dest->header->prev  = prev;
    StartSector         = start->header->start;
    next                = start->header->next;
    prev                = start->header->prev;
    memset(start->header, 0, sizeof(apa_header_t));

    start->header->magic  = APA_MAGIC;
    start->header->start  = StartSector;
    start->header->next   = next;
    start->header->prev   = prev;
    start->header->length = dest->header->length;
    start->header->modver = APA_MODVER;
    start->header->type   = dest->header->type;
    strcpy(start->header->id, "_tmp");

    memset(clink, 0, sizeof(clink));
    if (dest->header->flags & APA_FLAG_SUB) {
        if ((clink[0] = apaCacheGetHeader(device, dest->header->main, APA_IO_MODE_READ, &result)) != NULL) {
            for (i = 0; (u32)i < clink[0]->header->nsub; i++) {
                if (start->header->start == clink[0]->header->subs[i].start) {
                    clink[0]->header->subs[i].start = dest->header->start;
                    clink[0]->flags |= APA_CACHE_FLAG_DIRTY;
#ifdef HDSK_SUPPORT_HDLFS
                    if (dest->header->type == APA_TYPE_HDLFS) {
                        hdlUpdateGameSliceInfo(device, dest->header->main, i, start->header->start, dest->header->start);
                    }
#endif
                    break;
                }
            }

            if ((u32)i == clink[0]->header->nsub) {
                apaCacheFree(clink[0]);
                return -1;
            }
        } else {
            return result;
        }
    } else {
        // 0x00000944
        for (i = 0; (u32)i < dest->header->nsub; i++) {
            if ((clink[i] = apaCacheGetHeader(device, dest->header->subs[i].start, APA_IO_MODE_READ, &result)) == NULL) {
                for (--i; i >= 0; i--) {
                    apaCacheFree(clink[i]);
                }

                return result;
            }
        }

        for (i = 0; (u32)i < dest->header->nsub; i++) {
            clink[i]->header->main = dest->header->start;
            clink[i]->flags |= APA_CACHE_FLAG_DIRTY;
        }

#ifdef HDSK_SUPPORT_HDLFS
        if (dest->header->type == APA_TYPE_HDLFS) {
            hdlUpdateGameSliceInfo(device, dest->header->start, 0, start->header->start, dest->header->start);
        }
#endif
    }

    dest->flags |= APA_CACHE_FLAG_DIRTY;
    start->flags |= APA_CACHE_FLAG_DIRTY;
    apaCacheFlushAllDirty(device);
    for (i = 0; i < 64; i++) {
        if (clink[i] != NULL) {
            apaCacheFree(clink[i]);
        }
    }

    printf("done\n");

    return result;
}

static int MovePartition(int device, apa_cache_t *dest, apa_cache_t *start)
{
    int result;

    APA_PRINTF("MovePartition: %08lx to %08lx. sector count = %08lx.\n", start->header->start, dest->header->start, start->header->length);

    if (dest->header->type != APA_TYPE_FREE) {
        APA_PRINTF("error: destination is not empty.\n");
        return -1;
    }

    if (dest->header->length != start->header->length) {
        APA_PRINTF("error: source and destination size are not same.\n");
        return -1;
    }

    if ((result = hdskRemoveTmp(device)) == 0) {
        if ((result = CopyPartition(device, dest->header, start->header)) == 0) {
            if (hdskStopFlag == 0) {
                SwapPartition(device, dest, start);
            }
        }
    }

    APA_PRINTF("MovePartition: done\n");

    return result;
}

static int SplitEmptyPartition(int device, apa_cache_t *partition, u32 length)
{
    apa_cache_t *clink;
    int result;

    result = 0;

    APA_PRINTF("split empty partition.\n");

    while (partition->header->length != length) {
        apa_cache_t *empty;

        if ((empty = apaCacheGetHeader(device, partition->header->next, APA_IO_MODE_READ, &result)) != NULL) {
            partition->header->length /= 2;
            clink = apaRemovePartition(device, partition->header->start + partition->header->length, partition->header->next, partition->header->start, partition->header->length);

            partition->header->next = clink->header->start;
            partition->flags |= APA_CACHE_FLAG_DIRTY;

            empty->header->prev = clink->header->start;
            empty->flags |= APA_CACHE_FLAG_DIRTY;
            apaCacheFlushAllDirty(device);
            apaCacheFree(clink);
            apaCacheFree(empty);
        }
    }

    return result;
}

// Move a group of partitions.
static int MovePartitionsBlock(int device, apa_cache_t *dest, apa_cache_t *start)
{
    u32 remaining;
    int result, stat;

    remaining = dest->header->length;
    APA_PRINTF("MovePartitionsBlock: %08lx to %08lx. sector count = %08lx.\n", start->header->start, dest->header->start, dest->header->length);

    while (1) {
        if ((result = SplitEmptyPartition(device, dest, start->header->length)) == 0) {
            if ((result = MovePartition(device, dest, start)) == 0) {
                remaining -= dest->header->length;
                if (hdskStopFlag == 0 && remaining != 0) {
                    // Stop if there are no more partitions to copy (either to or from).
                    if ((dest = apaGetNextHeader(dest, &stat)) == NULL || (start = apaGetNextHeader(start, &stat)) == NULL) {
                        break;
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }

    apaCacheFree(dest);
    apaCacheFree(start);

    APA_PRINTF("MovePartitionsBlock: done\n");

    return result;
}

static void HdskThread(int device)
{
    u32 PartSize;
    apa_cache_t *clink, *clink2;

    hdskProgress = 0;
    hdskStatus   = 0;
    hdskStopFlag = 0;

    if ((hdskStatus = hdskRemoveTmp(device)) >= 0) {
        for (PartSize = HDSK_MIN_PART_SIZE; PartSize < HddInfo[device].partitionMaxSize; PartSize *= 2) {
            while ((hdskStatus = hdskRemoveTmp(device)) >= 0) {
                if ((clink = hdskFindEmptyPartition(device, PartSize)) != NULL) {
                    if ((clink2 = hdskFindLastUsedPartition(device, PartSize, clink->header->start, 1)) == NULL) {
                        if ((HDSK_MIN_PART_SIZE < PartSize) && (clink2 = hdskFindLastUsedBlock(device, PartSize, clink->header->start, 1)) != NULL) {
                            goto move_partition_block;
                        }

                        // 0x0000110c
                        if ((clink2 = hdskFindLastUsedPartition(device, PartSize, clink->header->start, 0)) == NULL) {
                            // 0x00001160
                            if (HDSK_MIN_PART_SIZE < PartSize) {
                                if ((clink2 = hdskFindLastUsedBlock(device, PartSize, clink->header->start, 0)) != NULL) {
                                    goto move_partition_block;
                                }
                            }

                            APA_PRINTF("there is no copyable partition/partitions block.\n");
                            apaCacheFree(clink);
                            break;
                        } else {
                            goto move_partition;
                        }

                    move_partition_block:
                        // 0x00001190
                        if ((hdskStatus = MovePartitionsBlock(device, clink, clink2)) != 0) {
                            goto hdsk_thread_end;
                        }
                    } else {
                    move_partition:
                        // 0x00001120
                        hdskStatus = MovePartition(device, clink, clink2);
                        // apaCacheFree(clink); // BUGBUG: SONY original frees clink here, and again below ("unused cache returned"). However, clink must be freed here before the user terminates the defrag operation.
                        apaCacheFree(clink2);
                        if (hdskStatus != 0) {
                            apaCacheFree(clink); // Move this here. See comment above.
                            goto hdsk_thread_end;
                        }
                    }

                    // 0x000011d4
                    apaCacheFree(clink);
                } else
                    break;
            }

            // 0x000011f4
            if (hdskStatus != 0 || hdskStopFlag != 0) {
                break;
            }
        }
    }

hdsk_thread_end:
    hdskRemoveTmp(device);
    SetEventFlag(hdskEventFlagID, 1);
}

static int HdskInit(iomanX_iop_device_t *device)
{
    (void)device;

    return 0;
}

static int HdskUnsupported(void)
{
    return -1;
}

int BitmapUsed;
u32 TotalCopied;
struct hdskBitmap hdskBitmap[HDSK_BITMAP_SIZE];

static int hdskBitmapInit(int device)
{
    apa_cache_t *clink;
    int result;

    clink = apaCacheGetHeader(device, 0, APA_IO_MODE_READ, &result);
    memset(hdskBitmap, 0, sizeof(hdskBitmap));

    hdskBitmap[0].next = hdskBitmap;
    hdskBitmap[0].prev = hdskBitmap;

    for (BitmapUsed = 0; clink != NULL;) {
        struct hdskBitmap *pBitmap;

        pBitmap         = &hdskBitmap[BitmapUsed + 1];
        pBitmap->start  = clink->header->start;
        pBitmap->length = clink->header->length;
        pBitmap->type   = clink->header->type;

        apaCacheLink((apa_cache_t *)hdskBitmap[0].prev, (apa_cache_t *)pBitmap);

        BitmapUsed++;
        clink = apaGetNextHeader(clink, &result);
    }

    return result;
}

static int hdskGetStat(int device, struct hdskStat *buf, apa_device_t *deviceInfo)
{
    int result;

    u32 PartSize;

    TotalCopied = 0;

    PartSize = HDSK_MIN_PART_SIZE;
    if ((result = hdskBitmapInit(device)) == 0) {
        apa_device_t *pDeviceInfo;

        hdskSimGetFreeSectors(device, buf, deviceInfo);

        for (pDeviceInfo = &deviceInfo[device]; PartSize < pDeviceInfo->partitionMaxSize; PartSize *= 2) {
            int IsValidPartSize;
            struct hdskBitmap *pPartBitmap;

            IsValidPartSize = HDSK_MIN_PART_SIZE < PartSize;

            while ((pPartBitmap = hdskSimFindEmptyPartition(PartSize)) != NULL) {
                struct hdskBitmap *pSelEmptyPartBM;

                if (((pSelEmptyPartBM = hdskSimFindLastUsedPartition(PartSize, pPartBitmap->start, 1)) == NULL) &&
                    (!IsValidPartSize || (pSelEmptyPartBM = hdskSimFindLastUsedBlock(PartSize, pPartBitmap->start, 1)) == NULL) &&
                    ((pSelEmptyPartBM = hdskSimFindLastUsedPartition(PartSize, pPartBitmap->start, 0)) == NULL)) {
                    if (IsValidPartSize && (pSelEmptyPartBM = hdskSimFindLastUsedBlock(PartSize, pPartBitmap->start, 0)) != NULL) {
                        APA_PRINTF("found last used block of partitions at %08lx, size %08lx.\n", pSelEmptyPartBM->start, pSelEmptyPartBM->length);
                        hdskSimMovePartitionsBlock(pPartBitmap, pSelEmptyPartBM);
                    } else {
                        APA_PRINTF("there is no copyable partition/partitions block.\n");
                        break;
                    }
                } else {
                    APA_PRINTF("found last used partition at %08lx, size %08lx.\n", pSelEmptyPartBM->start, pSelEmptyPartBM->length);
                    hdskSimMovePartition(pPartBitmap, pSelEmptyPartBM);
                }
            }
        }

        // 0x00004368
        hdskSimGetFreeSectors(device, buf, deviceInfo);
        printf("copy total %08lx sectors\n", TotalCopied);
        buf->total = TotalCopied;
    }

    return result;
}

static int HdskDevctl(iomanX_iop_file_t *fd, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    u32 bits;
    int result;

    (void)name;
    (void)arg;
    (void)arglen;
    (void)buflen;

    if (HddInfo[fd->unit].status != 0) {
        return -ENXIO;
    }

    switch (cmd) {
        case HDSK_DEVCTL_GET_FREE:
            result = apaGetFreeSectors(fd->unit, buf, HddInfo);
            break;
        case HDSK_DEVCTL_GET_HDD_STAT:
            if ((result = hdskRemoveTmp(fd->unit)) == 0)
                result = hdskGetStat(fd->unit, buf, HddInfo);
            break;
        case HDSK_DEVCTL_START:
            result = StartThread(hdskThreadID, (void *)(uiptr)fd->unit);
            break;
        case HDSK_DEVCTL_WAIT:
            result = WaitEventFlag(hdskEventFlagID, 1, WEF_CLEAR | WEF_OR, &bits);
            break;
        case HDSK_DEVCTL_POLL:
            result = PollEventFlag(hdskEventFlagID, 1, WEF_CLEAR | WEF_OR, &bits);
            if (result == KE_EVF_COND)
                result = 1;
            break;
        case HDSK_DEVCTL_GET_STATUS:
            result = hdskStatus;
            break;
        case HDSK_DEVCTL_STOP:
            hdskStopFlag = 1;
            result       = -EINVAL;
            break;
        case HDSK_DEVCTL_GET_PROGRESS:
            result = (int)hdskProgress;
            break;
        default:
            result = -EINVAL;
    }

    return result;
}

static iomanX_iop_device_ops_t HdskDeviceOps = {
    &HdskInit,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    &HdskDevctl,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
    (void *)&HdskUnsupported,
};

static iomanX_iop_device_t HdskDevice = {
    "hdsk",
    IOP_DT_FSEXT | IOP_DT_FS,
    1,
    "HDSK",
    &HdskDeviceOps};

int APA_ENTRYPOINT(int argc, char **argv)
{
    apa_ps2time_t time;
    ata_devinfo_t *pDevInfo;
    int i;

    (void)argc;
    (void)argv;

    if (apaGetTime(&time) != 0) {
        APA_PRINTF("error: could not get date\n");
        return MODULE_NO_RESIDENT_END;
    }

    APA_PRINTF("%02d:%02d:%02d %02d/%02d/%d\n", time.hour, time.min, time.sec, time.month, time.day, time.year);

    for (i = 0; i < 2; i++) {
        if ((pDevInfo = sceAtaInit(i)) == NULL) {
            APA_PRINTF("error: ata initialization failed.\n");
            return MODULE_NO_RESIDENT_END;
        }

        if (pDevInfo->exists && !pDevInfo->has_packet) {
            HddInfo[i].status--;
            HddInfo[i].totalLBA = pDevInfo->total_sectors;

            HddInfo[i].partitionMaxSize = apaGetPartitionMax(pDevInfo->total_sectors);
            if (HdskUnlockHdd(i) == 0) {
                HddInfo[i].status--;
            }

            APA_PRINTF("disk%d: 0x%08lx sectors, max 0x%08lx\n", i, HddInfo[i].totalLBA, HddInfo[i].partitionMaxSize);
        }
    }

    // 0x00001440
    apaCacheInit(128);
    for (i = 0; i < 2; i++) {
        if (HddInfo[i].status < 2) {
            if (apaJournalRestore(i) != 0) {
                return MODULE_NO_RESIDENT_END;
            }

            if (apaGetFormat(i, &HddInfo[i].format)) {
                HddInfo[i].status--;
            }
        }
    }

    if ((hdskEventFlagID = HdskCreateEventFlag()) < 0) {
        return MODULE_NO_RESIDENT_END;
    }

    if ((hdskThreadID = HdskCreateThread((void *)&HdskThread, 0x2080)) < 0) {
        return MODULE_NO_RESIDENT_END;
    }

    iomanX_DelDrv(HdskDevice.name);
    if (iomanX_AddDrv(&HdskDevice) == 0) {
        APA_PRINTF("driver start.\n");
        return MODULE_RESIDENT_END;
    }

    return MODULE_NO_RESIDENT_END;
}
