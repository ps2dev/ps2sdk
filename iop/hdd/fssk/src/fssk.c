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
#include <iomanX.h>
#include <loadcore.h>
#include <kerr.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <irx.h>
#include <hdd-ioctl.h>

#include "pfs-opt.h"
#include "libpfs.h"
#include "fssk-ioctl.h"
#include "fssk.h"
#include "misc_fssk.h"

#ifdef _IOP
IRX_ID("fssk", PFS_MAJOR, PFS_MINOR);
#endif

struct fsskRuntimeData
{
    struct fsskStatus status;
    int minFree;  // 0x1C
    int stopFlag; // 0x20
};

#define FSSK_MAX_PATH_LEVELS     64
#define FSSK_MAX_PATH_SEG_LENGTH 256

static int fsskEventFlagID;
static int fsskThreadID;
static char fsskPathBuffer[FSSK_MAX_PATH_LEVELS][FSSK_MAX_PATH_SEG_LENGTH];
static int fsskVerbosityLevel = 2;

static pfs_mount_t MainPFSMount = {0}; // FIXME: if not explicitly initialized to 0, the generated IRX would somehow have garbage in this structure.

#define IO_BUFFER_SIZE       256
#define IO_BUFFER_SIZE_BYTES (IO_BUFFER_SIZE * 512)

static struct fsskRuntimeData fsskRuntimeData;
static u8 IOBuffer[IO_BUFFER_SIZE_BYTES];

static void fsskPrintPWD(void)
{
    int i;
    char *pName;

    for (i = 0, pName = fsskPathBuffer[0]; (u32)i < fsskRuntimeData.status.PWDLevel; i++, pName += FSSK_MAX_PATH_SEG_LENGTH) {
        printf("/%s", pName);
    }

    if (i == 0) {
        printf("/");
    }
}

static int fsskHasSpaceToFree(pfs_cache_t *clink)
{
    int result;
    u32 i;
    pfs_cache_t *cinode;

    result = 0;
    cinode = pfsCacheUsedAdd(clink);

    for (i = 0; i < clink->u.inode->number_data; i++) {
        if (i != 0) {
            if (pfsFixIndex(i) == 0) {
                if ((cinode = pfsBlockGetNextSegment(cinode, &result)) == NULL) {
                    break;
                }
            }
        }

        if (clink->pfsMount->num_subs - fsskRuntimeData.status.partsDeleted < cinode->u.inode->data[pfsFixIndex(i)].subpart) {
            result = 1;
            break;
        }
    }

    fsskRuntimeData.status.inodeBlockCount += clink->u.inode->number_blocks;
    pfsCacheFree(cinode);

    return result;
}

static int fsskCalculateSpaceToRemove(pfs_mount_t *mount)
{
    u32 PartsToRemove, i, ratio, SizeOfSub, SizeOfSubZones, free, zfree, total_zones;

    PartsToRemove = 0;
    zfree         = mount->zfree;
    total_zones   = mount->total_zones;
    for (i = mount->num_subs; i != 0; i--, PartsToRemove++) {
        SizeOfSub      = mount->blockDev->getSize(mount->fd, i);
        SizeOfSubZones = SizeOfSub >> mount->sector_scale;
        total_zones -= SizeOfSubZones;
        free = SizeOfSubZones - mount->free_zone[i] - pfsGetBitmapSizeBlocks(mount->sector_scale, SizeOfSub) - 1;
        if (zfree >= mount->free_zone[i]) {
            zfree -= mount->free_zone[i];
            if (zfree >= free) {
                zfree -= free;
                ratio = (int)((u64)zfree * 100 / total_zones);
                PFS_PRINTF("free ratio=%ld, freezone=%ld.\n", ratio, zfree);
                if (ratio < (u32)(fsskRuntimeData.minFree)) {
                    break;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }

    PFS_PRINTF("%ld partitions will be removed.\n", PartsToRemove);

    return PartsToRemove;
}

static u16 fsskCalcLastSub(pfs_mount_t *mount)
{
    u32 *zfree, MaxFreeRatio, FreeRatio;
    u16 sub, result;

    for (result = sub = 0, zfree = mount->free_zone, MaxFreeRatio = 0; mount->num_subs - (fsskRuntimeData.status.partsDeleted - 1) != 0; sub++, zfree++) {
        FreeRatio = ((u64)*zfree) * 100 / (mount->blockDev->getSize(mount->fd, sub) >> mount->sector_scale);
        if (MaxFreeRatio < FreeRatio) {
            MaxFreeRatio = FreeRatio;
            result       = sub;
        }
    }

    return result;
}

static int fsskCopyBlock(pfs_mount_t *mount, pfs_blockinfo_t *block1, pfs_blockinfo_t *block2, u32 length)
{
    u32 i;

    for (i = 0; i < length; i++) {
        int result;

        if ((result = mount->blockDev->transfer(mount->fd, IOBuffer, block2->subpart, (block2->number + i) << mount->sector_scale, 1 << mount->sector_scale, PFS_IO_MODE_READ)) < 0 || ((result = mount->blockDev->transfer(mount->fd, IOBuffer, block1->subpart, (block1->number + i) << mount->sector_scale, 1 << mount->sector_scale, PFS_IO_MODE_WRITE)) < 0)) {
            return result;
        }
    }

    return 0;
}

static pfs_cache_t *fsskCreateIndirectSeg(pfs_cache_t *clink, pfs_cache_t *clink2, pfs_blockinfo_t *data, int *result)
{
    pfs_cache_t *clinkfree;
    pfs_blockinfo_t block;

    *result = 0;

    if (!pfsCacheIsFull()) {
        block.subpart = data->subpart;
        block.number  = data->number + data->count;
        if ((*result = pfsBitmapSearchFreeZone(clink->pfsMount, &block, clink->u.inode->number_blocks)) >= 0) {
            clinkfree = pfsCacheGetData(clink->pfsMount, block.subpart, block.number << clink->pfsMount->inode_scale, PFS_CACHE_FLAG_SEGI | PFS_CACHE_FLAG_NOLOAD, result);

            memset(clinkfree->u.inode, 0, sizeof(pfs_inode_t));
            clinkfree->u.inode->magic = PFS_SEGI_MAGIC;
            memcpy(&clinkfree->u.inode->inode_block, &clink->u.inode->inode_block, sizeof(pfs_blockinfo_t));
            memcpy(&clinkfree->u.inode->last_segment, &clink2->u.inode->data[0], sizeof(pfs_blockinfo_t));
            memcpy(&clinkfree->u.inode->data[0], &block, sizeof(pfs_blockinfo_t));
            clinkfree->flags |= PFS_CACHE_FLAG_DIRTY;
            clink->u.inode->number_blocks += block.count;
            clink->u.inode->number_data++;
            memcpy(&clink->u.inode->last_segment, &block, sizeof(pfs_blockinfo_t));
            clink->u.inode->number_segdesg++;
            clink->flags |= PFS_CACHE_FLAG_DIRTY;
            memcpy(&clink2->u.inode->next_segment, &block, sizeof(pfs_blockinfo_t));
            clink2->flags |= PFS_CACHE_FLAG_DIRTY;
            pfsCacheFree(clink2);

            return clinkfree;
        }
    } else {
        *result = -ENOMEM;
    }

    return NULL;
}

// Very similar to pfsBitmapSearchFreeZone().
static int fsckBitmapSearchFreeZoneSpecial(pfs_mount_t *pfsMount, pfs_blockinfo_t *bi, u32 max_count, u32 deleted)
{
    int num;
    u32 count, i;

    deleted--;
    num = pfsMount->num_subs - deleted;
    if (bi->subpart < num) {
        bi->subpart = 0;
    }
    if (bi->number != 0) {
        num++;
    }
    count = (max_count < 33) ? max_count : 32;
    if (count < bi->count) {
        count = bi->count;
    }

    for (--num; num >= 0; num--) {
        for (i = count; i != 0; i /= 2) {
            if ((pfsMount->free_zone[bi->subpart] >= i) && (pfsBitmapAllocZones(pfsMount, bi, i) != 0)) {
                pfsMount->free_zone[bi->subpart] -= bi->count;
                pfsMount->zfree -= bi->count;
                return 0;
            }
        }

        bi->subpart++;
        bi->number = 0;
        if (pfsMount->num_subs - deleted == bi->subpart) {
            bi->subpart = 0;
        }
    }

    return -ENOSPC;
}

// I hate this function. :(
static int fsskMoveInode(pfs_mount_t *mount, pfs_cache_t *dest, pfs_cache_t *start, pfs_dentry_t *dentry)
{
    pfs_cache_t *clink, *clink2, *clink3;
    pfs_blockinfo_t block, block2;
    int result;

    printf("\t========== Move");
    if (FIO_S_ISDIR(start->u.inode->mode)) {
        block.subpart = fsskCalcLastSub(mount);
        block.number  = 0;
    } else {
        memcpy(&block, &dest->u.inode->inode_block, sizeof(pfs_blockinfo_t));
    }

    block.count = 1;
    if ((result = fsckBitmapSearchFreeZoneSpecial(mount, &block, start->u.inode->number_blocks, fsskRuntimeData.status.partsDeleted)) >= 0) {
        if ((result = fsskCopyBlock(mount, &block, &start->u.inode->inode_block, 1)) >= 0 &&
            (clink = pfsCacheGetData(mount, block.subpart, block.number << mount->inode_scale, PFS_CACHE_FLAG_SEGD | PFS_CACHE_FLAG_NOLOAD, &result)) != NULL) {
            int i;

            memcpy(clink->u.inode, start->u.inode, sizeof(pfs_inode_t));
            memcpy(&clink->u.inode->inode_block, &block, sizeof(pfs_blockinfo_t));
            memcpy(&clink->u.inode->last_segment, &block, sizeof(pfs_blockinfo_t));
            memcpy(&clink->u.inode->data[0], &block, sizeof(pfs_blockinfo_t));
            clink->u.inode->number_segdesg = 1;
            clink->u.inode->number_data    = 1;
            clink->u.inode->number_blocks  = 1;
            clink2                         = pfsCacheUsedAdd(start);
            clink3                         = pfsCacheUsedAdd(clink);

            for (i = 1; (u32)i < start->u.inode->number_data && result == 0; i++) {
                if (pfsFixIndex(i) == 0) {
                    if ((clink2 = pfsBlockGetNextSegment(clink2, &result)) == NULL) {
                        break;
                    }
                } else {
                    memcpy(&block2, &clink2->u.inode->data[pfsFixIndex(i)], sizeof(pfs_blockinfo_t));
                    if (pfsFixIndex(clink->u.inode->number_data - 1) != 0) {
                        int value;
                        if ((value = pfsBitmapAllocateAdditionalZones(mount, clink3->u.inode->data, block2.count)) != 0) {
                            block.subpart = clink3->u.inode->data[0].subpart;
                            block.number  = clink3->u.inode->data[0].number + clink3->u.inode->data[0].count;
                            clink3->u.inode->data[0].count += value;

                            if ((result = fsskCopyBlock(mount, &block, &block2, value)) < 0) {
                                break;
                            }

                            clink->u.inode->number_blocks += value;
                            block2.number += value;
                            block2.count -= value;
                        }
                    }
                    while (block2.count != 0) {
                        if (pfsFixIndex(clink->u.inode->number_data) == 0) {
                            if ((clink3 = fsskCreateIndirectSeg(clink, clink3, clink3->u.inode->data, &result)) == NULL) {
                                break;
                            }
                        }

                        block.subpart = clink3->u.inode->data[0].subpart;
                        block.count   = block2.count;
                        block.number  = clink3->u.inode->data[0].number + clink3->u.inode->data[0].count;

                        if ((result = fsckBitmapSearchFreeZoneSpecial(mount, &block, start->u.inode->number_blocks, fsskRuntimeData.status.partsDeleted)) < 0) {
                            break;
                        }

                        memcpy(&clink3->u.inode->data[pfsFixIndex(start->u.inode->number_data)], &block, sizeof(pfs_blockinfo_t));

                        clink->u.inode->number_data++;
                        if ((result = fsskCopyBlock(mount, &block, &block2, block.count)) < 0) {
                            break;
                        }

                        clink->u.inode->number_blocks += block.count;
                        block2.number += block.count;
                        block2.count -= block.count;
                        block2.count--;
                    }
                }
            }

            // 0x000009d0
            pfsCacheFree(clink2);
            if (result == 0) {
                dentry->sub   = clink->u.inode->inode_block.subpart;
                dentry->inode = clink->u.inode->inode_block.number;
                clink3->flags |= PFS_CACHE_FLAG_DIRTY;
                clink->flags |= PFS_CACHE_FLAG_DIRTY;
                pfsCacheFree(clink3);
                pfsCacheFree(clink);
                pfsBitmapFreeInodeBlocks(start);
            } else {
                pfsBitmapFreeInodeBlocks(clink);
                pfsCacheFree(clink3);
                pfsCacheFree(clink);
            }
        } else
            pfsBitmapFreeBlockSegment(mount, &block);
    }

    return result;
}

static void fsskCheckSelfEntry(pfs_cache_t *SelfInodeClink, pfs_cache_t *SelfDEntryClink, pfs_dentry_t *dentry)
{
    if ((SelfInodeClink->sub != dentry->sub) || (SelfInodeClink->u.inode->inode_block.number != dentry->inode)) {
        PFS_PRINTF("'.' point not itself.\n");
        dentry->sub   = SelfInodeClink->u.inode->inode_block.subpart;
        dentry->inode = SelfInodeClink->u.inode->inode_block.number;
        SelfDEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
    }
}

static void fsskCheckParentEntry(pfs_cache_t *ParentInodeClink, pfs_cache_t *SelfDEntryClink, pfs_dentry_t *dentry)
{
    if ((ParentInodeClink->sub != dentry->sub) || (ParentInodeClink->u.inode->inode_block.number != dentry->inode)) {
        PFS_PRINTF("'..' point not parent.\n");
        dentry->sub   = ParentInodeClink->u.inode->inode_block.subpart;
        dentry->inode = ParentInodeClink->u.inode->inode_block.number;
        SelfDEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
    }
}

static int fsskCheckFiles(pfs_cache_t *ParentInodeClink, pfs_cache_t *InodeClink);

static int fsskCheckFile(pfs_cache_t *InodeClink, pfs_cache_t *DEntryClink, pfs_dentry_t *dentry)
{
    pfs_cache_t *FileInodeDataClink;
    int result;

    if ((FileInodeDataClink = pfsInodeGetData(InodeClink->pfsMount, dentry->sub, dentry->inode, &result)) != NULL) {
        if (fsskRuntimeData.status.PWDLevel < FSSK_MAX_PATH_LEVELS - 1) {
            memset(fsskPathBuffer[fsskRuntimeData.status.PWDLevel], 0, FSSK_MAX_PATH_SEG_LENGTH);
            strncpy(fsskPathBuffer[fsskRuntimeData.status.PWDLevel], dentry->path, dentry->pLen);
            fsskRuntimeData.status.PWDLevel++;

            if (fsskVerbosityLevel >= 2) {
                fsskPrintPWD();
                if (FIO_S_ISDIR(dentry->aLen)) {
                    printf(": ");
                }
            }

            if ((result = fsskHasSpaceToFree(FileInodeDataClink)) > 0) {
                if ((result = fsskMoveInode(InodeClink->pfsMount, InodeClink, FileInodeDataClink, dentry)) == 0) {
                    DEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
                    pfsCacheFree(FileInodeDataClink);
                    FileInodeDataClink = pfsInodeGetData(InodeClink->pfsMount, dentry->inode, dentry->sub, &result);
                }
            }

            putchar('\n');

            if (result == 0) {
                if (FIO_S_ISDIR(dentry->aLen)) {
                    fsskRuntimeData.status.directories++;
                    result = fsskCheckFiles(InodeClink, FileInodeDataClink);
                } else {
                    fsskRuntimeData.status.files++;
                }
            }

            fsskRuntimeData.status.PWDLevel--;
        } else {
            PFS_PRINTF("error: exceed max directory depth.\n");
            result = -ENOMEM;
        }
    }

    return result;
}

static int fsskCheckFiles(pfs_cache_t *ParentInodeClink, pfs_cache_t *InodeClink)
{
    pfs_blockpos_t BlockPosition;
    pfs_dentry_t *pDEntry, *pDEntryEnd;
    int result;
    u32 inodeOffset, dEntrySize; // inodeOffset doesn't seem to be 64-bit, even though the inode size field is 64-bits wide.
    pfs_cache_t *DEntryClink;

    if (CheckThreadStack() < 128) {
        PFS_PRINTF("error: there is no stack, giving up.\n");
        return -ENOMEM;
    }

    inodeOffset                 = 0;
    BlockPosition.inode         = pfsCacheUsedAdd(InodeClink);
    BlockPosition.block_segment = 1;
    BlockPosition.block_offset  = 0;
    BlockPosition.byte_offset   = 0;
    if ((DEntryClink = pfsGetDentriesChunk(&BlockPosition, &result)) == NULL) {
        pfsCacheFree(BlockPosition.inode);
        return result;
    }
    pDEntry = DEntryClink->u.dentry;

    while (inodeOffset < InodeClink->u.inode->size) {
        if (pDEntry >= (pfs_dentry_t *)(DEntryClink->u.data + 1024)) {
            pfsCacheFree(DEntryClink);
            if ((result = pfsInodeSync(&BlockPosition, 1024, InodeClink->u.inode->number_data)) != 0 || (DEntryClink = pfsGetDentriesChunk(&BlockPosition, &result)) == NULL) {
                break;
            }
        }

        for (pDEntry = DEntryClink->u.dentry, pDEntryEnd = DEntryClink->u.dentry + 1; pDEntry < pDEntryEnd; pDEntry = (pfs_dentry_t *)((u8 *)pDEntry + dEntrySize), inodeOffset += dEntrySize) {
            if (fsskRuntimeData.stopFlag != 0) {
                goto end;
            }

            dEntrySize = pDEntry->aLen & 0x0FFF;

            if (dEntrySize & 3) {
                PFS_PRINTF("error: directory entry is not aligned.\n");
                result = -EINVAL;
                goto end;
            }

            if (dEntrySize < (u32)((pDEntry->pLen + 11) & ~3)) {
                PFS_PRINTF("error: directory entry is too small.\n");
                result = -EINVAL;
                goto end;
            }

            if (pDEntryEnd < (pfs_dentry_t *)((u8 *)pDEntry + dEntrySize)) {
                PFS_PRINTF("error: directory entry is too long.\n");
                result = -EINVAL;
                goto end;
            }

            if (pDEntry->inode != 0) {
                if ((pDEntry->pLen == 1) && (pDEntry->path[0] == '.')) {
                    fsskCheckSelfEntry(InodeClink, DEntryClink, pDEntry);
                } else if ((pDEntry->pLen == 2) && (pDEntry->path[0] == '.')) {
                    fsskCheckParentEntry(ParentInodeClink, DEntryClink, pDEntry);
                } else {
                    if ((result = fsskCheckFile(InodeClink, DEntryClink, pDEntry)) < 0) {
                        goto end;
                    }
                }
            }
        }

        if (fsskRuntimeData.stopFlag != 0) {
            break;
        }
    }

end:
    pfsCacheFree(DEntryClink);
    pfsCacheFree(BlockPosition.inode);

    return result;
}

static void FsskThread(pfs_mount_t *mount)
{
    pfs_cache_t *clink;
    pfs_dentry_t dentry;
    int result, i;

    if ((fsskRuntimeData.status.partsDeleted = fsskCalculateSpaceToRemove(mount)) != 0) {
        if ((clink = pfsInodeGetData(mount, mount->root_dir.subpart, mount->root_dir.number, &result)) != NULL) {
            if ((result = fsskHasSpaceToFree(clink)) > 0) {
                PFS_PRINTF("root directory will be moved.\n");
                if ((result = fsskMoveInode(mount, clink, clink, &dentry)) == 0) {
                    pfsCacheFree(clink);
                    if ((clink = pfsInodeGetData(mount, dentry.sub, dentry.inode, &result)) != NULL) {
                        if ((result = mount->blockDev->transfer(mount->fd, IOBuffer, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_READ)) == 0) {
                            mount->root_dir.subpart = dentry.sub;
                            mount->root_dir.number  = dentry.inode;
                            memcpy(&((pfs_super_block_t *)IOBuffer)->root, &mount->root_dir, sizeof(pfs_blockinfo_t));
                            if ((result = mount->blockDev->transfer(mount->fd, IOBuffer, 0, PFS_SUPER_BACKUP_SECTOR, 1, PFS_IO_MODE_WRITE)) == 0) {
                                result = mount->blockDev->transfer(mount->fd, IOBuffer, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_WRITE);
                            }
                            mount->blockDev->flushCache(mount->fd);
                        }
                    }
                }
            }

            if (result >= 0) {
                fsskRuntimeData.status.directories++;
                result = fsskCheckFiles(clink, clink);
                pfsCacheFree(clink);
                pfsCacheFlushAllDirty(mount);
                if (result == 0) {
                    if (fsskRuntimeData.stopFlag == 0) {
                        if ((result = mount->blockDev->transfer(mount->fd, IOBuffer, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_READ)) == 0) {
                            ((pfs_super_block_t *)IOBuffer)->num_subs -= fsskRuntimeData.status.partsDeleted;
                            mount->num_subs -= fsskRuntimeData.status.partsDeleted;
                            if ((result = mount->blockDev->transfer(mount->fd, IOBuffer, 0, PFS_SUPER_BACKUP_SECTOR, 1, PFS_IO_MODE_WRITE)) == 0) {
                                result = mount->blockDev->transfer(mount->fd, IOBuffer, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_WRITE);
                            }

                            mount->blockDev->flushCache(mount->fd);
                        }

                        for (i = 0; (u32)i < fsskRuntimeData.status.partsDeleted && result == 0; i++) {
                            iomanX_ioctl2(mount->fd, HIOCDELSUB, NULL, 0, NULL, 0);
                        }
                    }
                }

                if (result < 0) {
                    fsskRuntimeData.status.hasError = 1;
                }
            } else {
                pfsCacheFree(clink);
                fsskRuntimeData.status.hasError = 1;
            }
        } else {
            PFS_PRINTF("error: cannot read root directory.\n");
            fsskRuntimeData.status.hasError = 1;
            goto end2;
        }
    } else {
        PFS_PRINTF("error: there is no space to remove.\n");
        goto end2;
    }

end2:
    SetEventFlag(fsskEventFlagID, 1);
}

static int FsskUnsupported(void)
{
    return 0;
}

static int FsskOpen(iomanX_iop_file_t *fd, const char *name, int flags, int mode)
{
    int blockfd, result;
    iox_stat_t StatData;
    pfs_block_device_t *pblockDevData;

    (void)flags;

    fsskVerbosityLevel = (mode & 0xF0) >> 4;

    if (MainPFSMount.fd) {
        return -EBUSY;
    }

    if ((result = iomanX_getstat(name, &StatData)) < 0) {
        PFS_PRINTF("error: could not get status.\n");
        return result;
    }

    if (StatData.mode != APA_TYPE_PFS) {
        PFS_PRINTF("error: not PFS.\n");
        return -EINVAL;
    }

    if ((pblockDevData = pfsGetBlockDeviceTable(name)) == NULL)
        return -ENXIO;

    if ((blockfd = iomanX_open(name, O_RDWR)) < 0) {
        PFS_PRINTF("error: cannot open.\n");
        return blockfd;
    }

    memset(&MainPFSMount, 0, sizeof(MainPFSMount));
    MainPFSMount.fd       = blockfd;
    MainPFSMount.blockDev = pblockDevData;

    if (pfsMountSuperBlock(&MainPFSMount) >= 0) {
        memset(&fsskRuntimeData, 0, sizeof(fsskRuntimeData));

        fsskRuntimeData.status.zoneUsed = MainPFSMount.total_zones - MainPFSMount.zfree;
        fsskRuntimeData.minFree         = 3;
        fd->privdata                    = &MainPFSMount;
        result                          = 0;
    } else {
        MainPFSMount.fd = 0;
        PFS_PRINTF("error: cannot continue.\n");
    }

    return result;
}

static int FsskClose(iomanX_iop_file_t *fd)
{
    iomanX_close(((pfs_mount_t *)fd->privdata)->fd);
    pfsCacheClose((pfs_mount_t *)fd->privdata);
    memset(fd->privdata, 0, sizeof(pfs_mount_t));

    return 0;
}

static int fsskSimGetStat(pfs_mount_t *mount)
{
    int result;

    fsskRuntimeData.status.partsDeleted = fsskCalculateSpaceToRemove(mount);
    result                              = 0;
    if (mount->num_subs - fsskRuntimeData.status.partsDeleted < mount->num_subs) {
        int i;
        for (i = mount->num_subs; i != 0; i--) {
            result += mount->blockDev->getSize(mount->fd, i);
        }
    }

    return result;
}

static int fsskGetEstimatedTime(pfs_mount_t *mount, int *result)
{
    unsigned int i;
    u64 clock;
    iop_sys_clock_t SysClock1, SysClock2, SysClockDiff;
    u32 sec, usec;

    clock = 1000000;
    for (i = 0; i < 4; i++) {
        GetSystemTime(&SysClock1);
        mount->blockDev->transfer(mount->fd, IOBuffer, 0, 1 << mount->sector_scale, 1 << mount->sector_scale, PFS_IO_MODE_READ);
        GetSystemTime(&SysClock2);

        SysClockDiff.lo = SysClock2.lo - SysClock1.lo;
        SysClockDiff.hi = SysClock2.hi - SysClock1.hi - (SysClock2.lo < SysClock1.lo);
        SysClock2USec(&SysClockDiff, &sec, &usec);

        PFS_PRINTF("%ld system clocks = %ld.%06ld sec\n", SysClockDiff.lo, sec, usec);

        if (usec < clock)
            clock = usec;
    }

    if ((*result = ((clock + 400) * fsskRuntimeData.status.zoneUsed / 1000000)) == 0)
        *result = 1;

    return 0;
}

static int FsskIoctl2(iomanX_iop_file_t *fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int result;
    u32 FlagBits;

    (void)arglen;
    (void)buflen;

    switch (cmd) {
        case FSSK_IOCTL2_CMD_GET_ESTIMATE:
            result = fsskGetEstimatedTime(fd->privdata, buf);
            break;
        case FSSK_IOCTL2_CMD_START:
            result = StartThread(fsskThreadID, fd->privdata);
            break;
        case FSSK_IOCTL2_CMD_WAIT:
            result = WaitEventFlag(fsskEventFlagID, 1, WEF_OR | WEF_CLEAR, &FlagBits);
            break;
        case FSSK_IOCTL2_CMD_POLL:
            result = PollEventFlag(fsskEventFlagID, 1, WEF_OR | WEF_CLEAR, &FlagBits);
            if (result == KE_EVF_COND) {
                result = 1;
            }
            break;
        case FSSK_IOCTL2_CMD_GET_STATUS:
            memcpy(buf, &fsskRuntimeData.status, sizeof(struct fsskStatus));
            result = 0;
            break;
        case FSSK_IOCTL2_CMD_STOP:
            fsskRuntimeData.stopFlag = 1;
            result                   = 0;
            break;
        case FSSK_IOCTL2_CMD_SET_MINFREE:
            if (*(int *)arg >= 4)
                fsskRuntimeData.minFree = *(int *)arg;
            result = 0;
            break;
        case FSSK_IOCTL2_CMD_SIM:
            result = fsskSimGetStat(fd->privdata);
            break;
        default:
            result = 0;
    }

    return result;
}

static iomanX_iop_device_ops_t FsskDeviceOps = {
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    NULL,
    &FsskOpen,
    &FsskClose,
    NULL,
    NULL,
    NULL,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    (void *)&FsskUnsupported,
    &FsskIoctl2};

static iomanX_iop_device_t FsskDevice = {
    "fssk",
    IOP_DT_FSEXT | IOP_DT_FS,
    1,
    "FSSK",
    &FsskDeviceOps};

static int DisplayUsageHelp(void)
{
    PFS_PRINTF("error: Usage: fssk [-n <num>]\n");
    return MODULE_NO_RESIDENT_END;
}

int PFS_ENTRYPOINT(int argc, char **argv)
{
    int buffers;

    buffers = 0x7E;
    for (argc--, argv++; argc > 0 && ((*argv)[0] == '-'); argc--, argv++) {
        if (!strcmp("-n", *argv)) {
            argv++;
            if (--argc > 0) {
                if (strtol(*argv, NULL, 10) < buffers) {
                    buffers = strtol(*argv, NULL, 10);
                }
            } else {
                return DisplayUsageHelp();
            }
        } else {
            return DisplayUsageHelp();
        }
    }

    PFS_PRINTF("max depth %d, %d buffers.\n", FSSK_MAX_PATH_LEVELS - 1, buffers);

    if (pfsCacheInit(buffers, 1024) < 0) {
        PFS_PRINTF("error: cache initialization failed.\n");
        return MODULE_NO_RESIDENT_END;
    }

    if ((fsskEventFlagID = fsskCreateEventFlag()) < 0) {
        return MODULE_NO_RESIDENT_END;
    }

    if ((fsskThreadID = fsskCreateThread((void *)&FsskThread, 0x2080)) < 0) {
        return MODULE_NO_RESIDENT_END;
    }

    iomanX_DelDrv(FsskDevice.name);
    if (iomanX_AddDrv(&FsskDevice) == 0) {
        PFS_PRINTF("version %04x driver start.\n", IRX_VER(PFS_MAJOR, PFS_MINOR));
        return MODULE_RESIDENT_END;
    }

    return MODULE_NO_RESIDENT_END;
}
