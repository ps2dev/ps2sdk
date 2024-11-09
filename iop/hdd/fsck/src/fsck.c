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
#include <irx.h>
#include <iomanX.h>
#include <hdd-ioctl.h>
#include <loadcore.h>
#include <kerr.h>
#include <thbase.h>
#include <thevent.h>
#include <stdio.h>
#include <sysclib.h>

#include "pfs-opt.h"
#include "libpfs.h"
#include "bitmap_fsck.h"
#include "misc_fsck.h"

#include "fsck-ioctl.h"

#ifdef _IOP
IRX_ID("fsck", PFS_MAJOR, PFS_MINOR);
#endif

struct fsckRuntimeData
{
    struct fsckStatus status; // 0x00
    int hasError;             // 0x1C
    int stopFlag;             // 0x20
};

static int fsckWriteEnabled;
static int fsckAutoMode;
static int fsckVerbosityLevel;
extern u32 pfsMetaSize;
extern u32 pfsBlockSize;
static pfs_mount_t MainPFSMount = {0}; // FIXME: if not explicitly initialized to 0, the generated IRX would somehow have garbage in this structure.

#define IO_BUFFER_SIZE       256
#define IO_BUFFER_SIZE_BYTES (IO_BUFFER_SIZE * 512)

static struct fsckRuntimeData fsckRuntimeData;
static u8 IOBuffer[IO_BUFFER_SIZE_BYTES];

#define FSCK_NUM_SUPPORTED_DEVICES 1
#define FSCK_MAX_PATH_LEVELS       64
#define FSCK_MAX_PATH_SEG_LENGTH   256

static int fsckEventFlagID;
static int fsckThreadID;
static char fsckPathBuffer[FSCK_MAX_PATH_LEVELS][FSCK_MAX_PATH_SEG_LENGTH];

static u32 ZoneSizes[0x41]; // Contains the sizes of all zones, in units of 512-byte sectors.
static u32 ZoneMap[0x41];   // Contains the starting addresses of all zones, in units of blocks.

// 0x00000000
const char *fsckGetChar(void)
{
    static char buffer[80];
    const char *pChar;

#ifdef _IOP
    // cppcheck-suppress getsCalled
    if (gets(buffer) != NULL)
#else
    if (fgets(buffer, sizeof(buffer), stdin) != NULL)
#endif
    {
        for (pChar = buffer; *pChar != '\0'; pChar++) {
            if (isgraph(*(const unsigned char *)pChar)) {
                break;
            }
        }
    } else {
        pChar = NULL;
    }

    return pChar;
}

// 0x00000068
static int fsckPromptUserAction(const char *description, int mode)
{
    int result;

    fsckRuntimeData.status.errorCount++;

    if (fsckWriteEnabled != 0) {
        if (fsckAutoMode != 0) {
            printf("\n");
            if (mode) {
                fsckRuntimeData.status.fixedErrorCount++;
            } else {
                fsckRuntimeData.hasError = 1;
            }

            result = mode;
        } else {
            unsigned char choice;

            printf("%s (%s)? ", description, mode == 0 ? "n/y" : "y/n");
            do {
                choice = *fsckGetChar();
            } while (choice != 'n' && choice != 'y');

            if (choice == 'y') {
                fsckRuntimeData.status.fixedErrorCount++;
                result = 1;
            } else {
                fsckRuntimeData.hasError = 1;
                result                   = 0;
            }
        }
    } else {
        printf("\n");
        fsckRuntimeData.hasError = 1;
        result                   = 0;
    }

    return result;
}

#ifndef FSCK100
static int fsckPromptUserAction2(const char *description, int mode)
{
    int result;

    if (fsckWriteEnabled != 0) {
        if (fsckAutoMode != 0) {
            printf("\n");
            if (mode == 0) {
                fsckRuntimeData.hasError = 1;
            }

            result = mode;
        } else {
            unsigned char choice;

            printf("%s (%s)? ", description, mode == 0 ? "n/y" : "y/n");
            do {
                choice = *fsckGetChar();
            } while (choice != 'n' && choice != 'y');

            if (choice == 'y') {
                result = 1;
            } else {
                fsckRuntimeData.hasError = 1;
                result                   = 0;
            }
        }
    } else {
        printf("\n");
        fsckRuntimeData.hasError = 1;
        result                   = 0;
    }

    return result;
}
#endif

// 0x00002654
static int DisplayUsageHelp(void)
{
    PFS_PRINTF("error: Usage: fsck [-n <num>]\n");
    return MODULE_NO_RESIDENT_END;
}

#ifndef FSCK100
static int fsckCheckExtendedAttribute(pfs_mount_t *mount)
{
    int remaining, size, result;

    iomanX_lseek(mount->fd, 0, SEEK_SET);
    for (result = 0, remaining = 0x1FF8; remaining != 0; remaining -= size) {
        size = (remaining > IO_BUFFER_SIZE) ? IO_BUFFER_SIZE : remaining;

        if (iomanX_read(mount->fd, IOBuffer, size * 512) == -EIO) {
            PFS_PRINTF("cannot read extended attribute %d\n", -EIO);
            if (fsckPromptUserAction(" nullify extended attribute", 1) == 0) {
                break;
            }

            memset(IOBuffer, 0, IO_BUFFER_SIZE_BYTES);
            iomanX_lseek(mount->fd, 0, SEEK_SET);
            for (remaining = 0x1FF8; remaining != 0; remaining -= size) {
                size = (remaining > IO_BUFFER_SIZE) ? IO_BUFFER_SIZE : remaining;

                if ((result = iomanX_write(mount->fd, IOBuffer, size * 512)) < 0) {
                    PFS_PRINTF("error: could not nullify extended attribute.\n");
                    break;
                }
            }

            break;
        }
    }

    return result;
}
#endif

static int fsckCheckDirentryInode(pfs_cache_t *clink);

// 0x0000183c
static pfs_cache_t *CheckRootDirectory(pfs_mount_t *mount)
{
    pfs_cache_t *pResultClink, *clink;
    int result;

    if ((clink = pfsInodeGetData(mount, mount->root_dir.subpart, mount->root_dir.number, &result)) != NULL) {
        if (fsckVerbosityLevel >= 2) {
            printf("/: ");
        }
        if (fsckCheckDirentryInode(clink) == 0) {
            pResultClink = clink;
        } else {
            pfsCacheFree(clink);
            pResultClink = NULL;
        }
    } else {
        pResultClink = NULL;
    }

    return pResultClink;
}

// 0x000007a8
static void pfsPrintPWD(void)
{
    int i;
    char *pName;

    for (i = 0, pName = fsckPathBuffer[0]; (u32)i < fsckRuntimeData.status.PWDLevel; i++, pName += FSCK_MAX_PATH_SEG_LENGTH) {
        printf("/%s", pName);
    }

    if (i == 0) {
        printf("/");
    }
}

// 0x000008c8
static int pfsInitDirEnt(pfs_mount_t *mount, u16 subpart, u32 inodeNumber, u32 number, int isDir)
{
    pfs_cache_t *clink;
    int result;

    result = -EIO;
    if (fsckPromptUserAction(" initialize directory entry", 1) != 0) {
        if ((clink = pfsCacheGetData(mount, subpart, number, PFS_CACHE_FLAG_NOLOAD, &result)) != NULL) {
            pfs_dentry_t *pDentry;

            memset(clink->u.dentry, 0, pfsMetaSize);
            pDentry = clink->u.dentry;

            if (isDir) { // Similar to pfsFillSelfAndParentDentries().
                // Self entry
                pDentry->inode   = inodeNumber;
                pDentry->sub     = (u8)subpart;
                pDentry->pLen    = 1;
                pDentry->aLen    = FIO_S_IFDIR | 12;
                pDentry->path[0] = '.';
                pDentry->path[1] = '\0';

                // Parent entry
                pDentry = (pfs_dentry_t *)((u8 *)pDentry + 12);

                pDentry->inode   = inodeNumber;
                pDentry->sub     = (u8)subpart;
                pDentry->pLen    = 2;
                pDentry->aLen    = FIO_S_IFDIR | 500;
                pDentry->path[0] = '.';
                pDentry->path[1] = '.';
                pDentry->path[2] = '\0';
            } else {
                pDentry->aLen = sizeof(pfs_dentry_t);
            }

            pDentry       = clink->u.dentry + 1;
            pDentry->aLen = sizeof(pfs_dentry_t);

            clink->flags |= PFS_CACHE_FLAG_DIRTY;
            pfsCacheFree(clink);
        }
    }

    return result;
}

// 0x000001c0
static int fsckCheckFileBitmap(pfs_mount_t *mount, pfs_blockinfo_t *blockinfo)
{
    pfs_bitmapInfo_t bitmapinfo;
    pfs_cache_t *clink;
    u32 chunk, bit, count;
    u32 *pBitmap;

    pfsBitmapSetupInfo(mount, &bitmapinfo, blockinfo->subpart, blockinfo->number);
    for (count = blockinfo->count; count != 0;) {
        chunk = bitmapinfo.chunk;
        bitmapinfo.chunk++;
        // bitmapinfo.index will contain the first index in the bitmap array to start from.
        if ((clink = pfsBitmapReadPartition(mount, blockinfo->subpart, chunk)) != NULL) { // bitmapinfo.bit will contain the first bit to start checking from.
            for (pBitmap = &clink->u.bitmap[bitmapinfo.index]; (pBitmap < clink->u.bitmap + 0x100) && count != 0; pBitmap++, bitmapinfo.bit = 0) {
                for (bit = bitmapinfo.bit; (bit < 32) && (count != 0); bit++, count--) {
                    if ((*pBitmap & (1 << bit)) == 0) {
                        PFS_PRINTF("not marked as used.\n");
                        if (fsckPromptUserAction(" Mark in use", 1) != 0) {
                            *pBitmap |= (1 << bit);
                            clink->flags |= PFS_CACHE_FLAG_DIRTY;
                        } else {
                            return -EINVAL;
                        }
                    }
                }
            }

            bitmapinfo.index = 0;
            pfsCacheFree(clink);
        } else {
            break;
        }
    }

    return 0;
}

// 0x00005550
static int fsckCheckZones(u32 number, u32 size)
{
    u32 index, zone, remaining, bit, startBit;
    u32 *pZone;
    pfs_bitmap_t *pBitmap;

    index    = number >> 20;
    zone     = (number >> 5) & 0x7FFF; // Contains the index of the first zone to start checking from.
    startBit = number & 31;
    for (remaining = size; remaining != 0;) {
        pBitmap = pfsBitmapRead(index);
        index++;

        if (pBitmap != NULL) { // startBit contains the first bit to start checking from.
            for (pZone = &pBitmap->bitmap[zone]; (pZone < pBitmap->bitmap + 0x8000) && remaining != 0; pZone++) {
                for (bit = startBit; bit < 32 && remaining != 0; bit++, remaining--) {
                    if ((*pZone & (1 << bit)) != 0) {
                        PFS_PRINTF("error: overlapped zone found.\n");
                        return 1;
                    }

                    *pZone |= (1 << bit);
                    pBitmap->isDirty = 1;
                }

                startBit = 0;
            }

            pfsBitmapFree(pBitmap);
            zone = 0;
        } else {
            return -EIO;
        }
    }

    return 0;
}

// 0x000009dc    - BUGBUG - if there's no next segment (blockClink == NULL), this function will end up dereferencing a NULL pointer.
static void fsckFillInode(pfs_cache_t *inodeClink, pfs_cache_t *blockClink, u32 blocks, u32 entries, u32 segdesg)
{
    // non-SCE: if NULL, don't do anything
    if (blockClink == NULL) {
        return;
    }
    memset(&blockClink->u.inode->next_segment, 0, sizeof(blockClink->u.inode->next_segment));

    inodeClink->u.inode->number_segdesg = (blocks - segdesg) * inodeClink->pfsMount->zsize;
    inodeClink->u.inode->subpart        = 0;
    if (!FIO_S_ISDIR(inodeClink->u.inode->mode)) {
        inodeClink->u.inode->attr &= ~PFS_FIO_ATTR_CLOSED;
    }

    inodeClink->u.inode->number_blocks        = blocks;
    inodeClink->u.inode->number_data          = entries;
    inodeClink->u.inode->number_segdesg       = segdesg;
    inodeClink->u.inode->last_segment.subpart = blockClink->sub;
    inodeClink->u.inode->last_segment.number  = blockClink->block >> blockClink->pfsMount->inode_scale;
    blockClink->flags |= PFS_CACHE_FLAG_DIRTY;
    inodeClink->flags |= PFS_CACHE_FLAG_DIRTY;
}

// 0x00000b04    - I hate this function and it hates me.
static int fsckCheckDirentryInode(pfs_cache_t *direntInodeClink)
{
    int i, result;
    pfs_cache_t *blockClink, *childDirentBlockClink;
    pfs_blockinfo_t *pInodeInfo;
    u32 index, new_index, blocks, segdesg, inodeStart, sector;
    u32 inodeOffset; // inodeOffset doesn't seem to be 64-bit, even though the inode size field is 64-bits wide.

    inodeOffset = 0;
    blocks      = 0;
    segdesg     = 1;
    result      = 0;
    blockClink  = pfsCacheUsedAdd(direntInodeClink);

    // Iterate through the whole directory entry and check that every part of it can be read.
    for (index = 0; (index < direntInodeClink->u.inode->number_data) && (result == 0); index++) // While within bounds and no error occurs.
    {
        new_index = pfsFixIndex(index);

        if (index != 0 && new_index == 0) { // Read the next inode
            pInodeInfo = &blockClink->u.inode->next_segment;
            pfsCacheFree(blockClink);
            if ((blockClink = pfsCacheGetData(direntInodeClink->pfsMount, pInodeInfo->subpart, pInodeInfo->number << direntInodeClink->pfsMount->inode_scale, PFS_CACHE_FLAG_SEGI, &result)) == NULL) {
                if ((result == -EIO) && (fsckPromptUserAction(" Remove rest of file", 1) != 0)) {
                    fsckFillInode(direntInodeClink, NULL, blocks, index, segdesg); // bug?! This will cause a NULL-pointer to be dereferenced!
                }
                break;
            }

            segdesg++;
        }

        // 0x00000c18
        // Check that the block is valid.
        pInodeInfo = &blockClink->u.inode->data[new_index];
        if ((direntInodeClink->pfsMount->num_subs < pInodeInfo->subpart) || (pInodeInfo->count == 0) || (pInodeInfo->number < 2) || (ZoneSizes[pInodeInfo->subpart] < ((pInodeInfo->number + pInodeInfo->count) << direntInodeClink->pfsMount->sector_scale))) {
            putchar('\n');
            pfsPrintPWD();
            printf(" contains a bad zone.");
            if (fsckPromptUserAction(" Remove rest of file", 1) != 0) {
                fsckFillInode(direntInodeClink, blockClink, blocks, index, segdesg);
                break;
            }

            result = -EINVAL;
            break;
        }

        // 0x00000ccc
        if (new_index != 0) {
            if (FIO_S_ISDIR(direntInodeClink->u.inode->mode)) { // If the inode is a directory, then all its blocks contain directory entries. Ensure that all of them can be read.
                // 0x00000cfc
                for (i = 0; i < pInodeInfo->count; i++) {
                    inodeStart = (pInodeInfo->number + i) << direntInodeClink->pfsMount->inode_scale;

                    // 0x00000dbc
                    for (sector = 0; (sector < (u32)(1 << direntInodeClink->pfsMount->inode_scale)) && (inodeOffset < direntInodeClink->u.inode->size); sector++) {
                        inodeOffset += pfsMetaSize;
                        if ((childDirentBlockClink = pfsCacheGetData(direntInodeClink->pfsMount, pInodeInfo->subpart, inodeStart + sector, PFS_CACHE_FLAG_NOTHING, &result)) != NULL) {
                            pfsCacheFree(childDirentBlockClink);
                        } else {
                            if (result == -ENOMEM) {
                                goto end;
                            }

                            PFS_PRINTF("could not read directory block.\n");
                            if ((result = pfsInitDirEnt(direntInodeClink->pfsMount, pInodeInfo->subpart, inodeStart + sector, direntInodeClink->u.inode->inode_block.number, (index == 1 && i == 0) ? sector < 1 : 0)) < 0) {
                                goto end2;
                            }
                        }
                    }

                    // 0x00000e0c
                    if (fsckVerbosityLevel >= 10) {
                        putchar('.');
                    }
                }
            } else {
                // The Inode contains a file. Ensure that the whole file can be read.
                // 0x00000e50
                for (i = 0; i < pInodeInfo->count; i++) {
                    if (direntInodeClink->pfsMount->blockDev->transfer(direntInodeClink->pfsMount->fd, IOBuffer, pInodeInfo->subpart, (pInodeInfo->number + i) << direntInodeClink->pfsMount->sector_scale, 1 << direntInodeClink->pfsMount->sector_scale, PFS_IO_MODE_READ) == 0) {
                        if (fsckVerbosityLevel >= 10) {
                            putchar('.');
                        }

                        if (fsckRuntimeData.stopFlag != 0) {
                            goto end;
                        }
                    } else {
                        PFS_PRINTF("could not read zone.\n");
                        if (fsckPromptUserAction(" Remove rest of file", 1) != 0) {
                            fsckFillInode(direntInodeClink, blockClink, blocks, index, segdesg);
                        }

                        goto end;
                    }
                }
            }
        }

        // 0x00000ef8
        // Check the free space bitmap.
        if ((result = fsckCheckFileBitmap(direntInodeClink->pfsMount, pInodeInfo)) >= 0) {
            result = fsckCheckZones(pInodeInfo->number + ZoneMap[pInodeInfo->subpart], pInodeInfo->count);
            if (result > 0) { // 0x00000f44
                if (fsckPromptUserAction(" Remove rest of file", 1) != 0) {
                    fsckFillInode(direntInodeClink, blockClink, blocks, index, segdesg);
                }
                break;
            } else if (result < 0) // result < 0
            {
                goto end2;
            }
        } else
            goto end2;

        // 0x00000f7c - Final part of the loop.
        fsckRuntimeData.status.inodeBlockCount += pInodeInfo->count;
        blocks += pInodeInfo->count;
        if (fsckRuntimeData.stopFlag != 0)
            break;
    }

end:
    if (result < 0) {
    end2:
        fsckRuntimeData.hasError = 1;
    }

    // 0x00000ff0
    if (fsckVerbosityLevel >= 2)
        printf("\n");

    pfsCacheFree(blockClink);

    return result;
}

// 0x00000828
static void fsckFixDEntry(pfs_cache_t *clink, pfs_dentry_t *dentry)
{
    u32 dEntrySize, offset;
    u16 aLen;
    pfs_dentry_t *pDEntryNew, *pDEntry;

    dEntrySize = (u32)((u8 *)dentry - (u8 *)clink->u.dentry);
    // if((s32)dEntrySize < 0)
    if (clink->u.dentry > dentry)
        dEntrySize += 0x1FF;

    dEntrySize = dEntrySize >> 9 << 9; // Round off
    pDEntryNew = (pfs_dentry_t *)((u8 *)clink->u.dentry + dEntrySize);
    for (pDEntry = NULL, offset = 0; offset < sizeof(pfs_dentry_t); offset += aLen, pDEntry = pDEntryNew, pDEntryNew = (pfs_dentry_t *)((u8 *)pDEntryNew + aLen)) {
        aLen = pDEntryNew->aLen & 0x0FFF;

        if (pDEntryNew == dentry) {
            if (pDEntry != NULL) {
                pDEntry->aLen = (pDEntry->aLen & FIO_S_IFMT) | ((pDEntry->aLen & 0x0FFF) + aLen);
            } else {
                pDEntryNew->inode = 0;
                pDEntryNew->pLen  = 0;
            }

            clink->flags |= PFS_CACHE_FLAG_DIRTY;
            break;
        }
    }
}

// 0x000012b4
static void fsckCheckSelfEntry(pfs_cache_t *SelfInodeClink, pfs_cache_t *SelfDEntryClink, pfs_dentry_t *dentry)
{
    if ((SelfInodeClink->sub != dentry->sub) || (SelfInodeClink->u.inode->inode_block.number != dentry->inode)) {
        PFS_PRINTF("'.' point not itself.\n");
        if (fsckPromptUserAction(" Fix", 1) != 0) {
            dentry->sub   = SelfInodeClink->u.inode->inode_block.subpart;
            dentry->inode = SelfInodeClink->u.inode->inode_block.number;
            SelfDEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
        }
    }
}

// 0x00001374
static void fsckCheckParentEntry(pfs_cache_t *ParentInodeClink, pfs_cache_t *SelfDEntryClink, pfs_dentry_t *dentry)
{
    if ((ParentInodeClink->sub != dentry->sub) || (ParentInodeClink->u.inode->inode_block.number != dentry->inode)) {
        PFS_PRINTF("'..' point not parent.\n");
        if (fsckPromptUserAction(" Fix", 1) != 0) {
            dentry->sub   = ParentInodeClink->u.inode->inode_block.subpart;
            dentry->inode = ParentInodeClink->u.inode->inode_block.number;
            SelfDEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
        }
    }
}

static void fsckCheckFiles(pfs_cache_t *ParentInodeClink, pfs_cache_t *InodeClink);

// 0x00001054
static void fsckCheckFile(pfs_cache_t *FileInodeClink, pfs_cache_t *FileInodeDataClink, pfs_dentry_t *dentry)
{
    if (fsckRuntimeData.status.PWDLevel < FSCK_MAX_PATH_LEVELS - 1) {
        memset(fsckPathBuffer[fsckRuntimeData.status.PWDLevel], 0, FSCK_MAX_PATH_SEG_LENGTH);
        strncpy(fsckPathBuffer[fsckRuntimeData.status.PWDLevel], dentry->path, dentry->pLen);
        fsckRuntimeData.status.PWDLevel++;

        if (fsckVerbosityLevel >= 2) {
            pfsPrintPWD();
            if (FIO_S_ISDIR(dentry->aLen)) {
                printf(": ");
            }
        }

        if (FIO_S_ISREG(dentry->aLen)) {
            if (FileInodeDataClink->pfsMount->blockDev->transfer(FileInodeDataClink->pfsMount->fd, IOBuffer, FileInodeDataClink->sub, (FileInodeDataClink->block + 1) << pfsBlockSize, 1 << pfsBlockSize, PFS_IO_MODE_READ) != 0) {
                PFS_PRINTF("could not read extended attribute.\n");
                if (fsckPromptUserAction(" initialize attribute", 1) != 0) {
                    memset(IOBuffer, 0, 1024);
                    ((pfs_aentry_t *)IOBuffer)->aLen = 1024;
                    if (FileInodeDataClink->pfsMount->blockDev->transfer(FileInodeDataClink->pfsMount->fd, IOBuffer, FileInodeDataClink->sub, (FileInodeDataClink->block + 1) << pfsBlockSize, 1 << pfsBlockSize, PFS_IO_MODE_WRITE) != 0) {
                        fsckRuntimeData.hasError = 1;
                    }
                } else // This is not actually needed, but it's done in the original.
                {
                    fsckRuntimeData.hasError = 1;
                }
            }
        }

        // 0x00001220
        fsckCheckDirentryInode(FileInodeDataClink);
        if (FIO_S_ISDIR(dentry->aLen)) {
            fsckRuntimeData.status.directories++;
            fsckCheckFiles(FileInodeClink, FileInodeDataClink);
        } else {
            fsckRuntimeData.status.files++;
        }

        fsckRuntimeData.status.PWDLevel--;
    } else {
        PFS_PRINTF("error: exceed max directory depth.\n");
        fsckRuntimeData.hasError = 1;
    }
}

// 0x00001434
static void fsckCheckFiles(pfs_cache_t *ParentInodeClink, pfs_cache_t *InodeClink)
{
    pfs_blockpos_t BlockPosition;
    pfs_dentry_t *pDEntry, *pDEntryEnd;
    int result;
    u32 inodeOffset, dEntrySize; // inodeOffset doesn't seem to be 64-bit, even though the inode size field is 64-bits wide.
    pfs_cache_t *DEntryClink, *FileInodeDataClink;

    inodeOffset                 = 0;
    BlockPosition.inode         = pfsCacheUsedAdd(InodeClink);
    BlockPosition.block_segment = 1;
    BlockPosition.block_offset  = 0;
    BlockPosition.byte_offset   = 0;
    if ((DEntryClink = pfsGetDentriesChunk(&BlockPosition, &result)) == NULL) {
        pfsCacheFree(BlockPosition.inode);
        fsckRuntimeData.hasError = 1;
        return;
    }
    pDEntry = DEntryClink->u.dentry;

    // 0x000017c0
    while (inodeOffset < InodeClink->u.inode->size) {
        // 0x000014cc
        if (pDEntry >= (pfs_dentry_t *)(DEntryClink->u.data + 1024)) {
            // Read next inode
            // 0x000014e4
            pfsCacheFree(DEntryClink);
            if (pfsInodeSync(&BlockPosition, 1024, InodeClink->u.inode->number_data) != 0 || (DEntryClink = pfsGetDentriesChunk(&BlockPosition, &result)) == NULL) {
                fsckRuntimeData.hasError = 1;
                goto end;
            }

            pDEntry = DEntryClink->u.dentry;
        }

        // 0x0000153c
        for (pDEntryEnd = pDEntry + 1; pDEntry < pDEntryEnd; pDEntry = (pfs_dentry_t *)((u8 *)pDEntry + dEntrySize), inodeOffset += dEntrySize) {
            if (fsckRuntimeData.stopFlag != 0) {
                goto end;
            }

            dEntrySize = pDEntry->aLen & 0x0FFF;

            if (dEntrySize & 3) {
                dEntrySize = (u32)((u8 *)pDEntryEnd - (u8 *)pDEntry);
                PFS_PRINTF("directory entry is not aligned.\n");

                if (fsckPromptUserAction(" Fix", 1) != 0) {
                    pDEntry->aLen = (pDEntry->aLen & 0xF000) | dEntrySize;
                    DEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
                }
            }

            if (dEntrySize < (u32)((pDEntry->pLen + 11) & ~3)) {
                dEntrySize = (u32)((u8 *)pDEntryEnd - (u8 *)pDEntry);
                PFS_PRINTF("directory entry is too small.\n");

                if (fsckPromptUserAction(" Fix", 1) != 0) {
                    if ((u32)((pDEntry->pLen + 11) & ~3) < dEntrySize) {
                        pDEntry->aLen = (pDEntry->aLen & 0xF000) | dEntrySize;
                        DEntryClink->flags |= PFS_CACHE_FLAG_DIRTY;
                    } else {
                        fsckFixDEntry(DEntryClink, pDEntry);
                        pDEntry->inode = 0;
                    }
                } else {
                    pDEntry->inode = 0;
                }
            }

            // 0x00001654
            if (pDEntryEnd < (pfs_dentry_t *)((u8 *)pDEntry + dEntrySize)) {
                dEntrySize = (u32)((u8 *)pDEntryEnd - (u8 *)pDEntry);
                PFS_PRINTF("directory entry is too long.\n");
                if (fsckPromptUserAction(" Fix", 1) != 0) {
                    fsckFixDEntry(DEntryClink, pDEntry);
                }

                pDEntry->inode = 0;
            }

            // 0x00001694
            if (pDEntry->inode != 0) {
                if ((pDEntry->pLen == 1) && (pDEntry->path[0] == '.')) {
                    fsckCheckSelfEntry(InodeClink, DEntryClink, pDEntry);
                } else if ((pDEntry->pLen == 2) && (pDEntry->path[0] == '.')) {
                    fsckCheckParentEntry(ParentInodeClink, DEntryClink, pDEntry);
                } else {
                    if ((FileInodeDataClink = pfsInodeGetData(InodeClink->pfsMount, pDEntry->sub, pDEntry->inode, &result)) != NULL) {
                        fsckCheckFile(InodeClink, FileInodeDataClink, pDEntry);
                        pfsCacheFree(FileInodeDataClink);
                    } else {
                        if (result == -EIO) {
                            printf(" contains an unreadable file '%.*s'.\n", pDEntry->pLen, pDEntry->path);
                            if (fsckPromptUserAction(" Remove", 1) != 0)
                                fsckFixDEntry(DEntryClink, pDEntry);
                        } else
                            fsckRuntimeData.hasError = 1;
                    }
                }
            }
        }

        // 0x000017ac
        if (fsckRuntimeData.stopFlag != 0)
            break;
    }

end:
    pfsCacheFree(DEntryClink);
    pfsCacheFree(BlockPosition.inode);
}

// 0x0000054c
static void fsckCompareBitmap(pfs_mount_t *mount, void *buffer)
{
    int hasUpdate;
    u32 i, NumZones, zone;
    u32 *pBitmap, ZoneStartOffset, *pRawBitmap, RawSize, unaligned, length, offset;

    (void)buffer;

    for (i = 0, offset = 0; i < mount->num_subs + 1; i++, offset++) {
        RawSize   = ZoneSizes[i] >> mount->sector_scale;
        NumZones  = RawSize / (mount->zsize << 3);
        unaligned = RawSize % (mount->zsize << 3);

        for (zone = 0; (unaligned == 0 && zone < NumZones) || (unaligned != 0 && zone < NumZones + 1); zone++) {
            length    = (zone == NumZones) ? unaligned : mount->zsize << 3;
            hasUpdate = 0;

            pBitmap         = pfsGetBitmapEntry(offset + (zone * (mount->zsize << 3)));
            ZoneStartOffset = (i == 0) ? (0x2000 >> mount->sector_scale) + 1 : 1;
            if (mount->blockDev->transfer(mount->fd, IOBuffer, i, (ZoneStartOffset + zone) << mount->sector_scale, 1 << mount->sector_scale, PFS_IO_MODE_READ) < 0) {
                break;
            }

            for (pRawBitmap = (u32 *)IOBuffer; pRawBitmap < (u32 *)(IOBuffer + (length >> 3)); pRawBitmap++, pBitmap++) {
                // 0x00000698
                if (*pRawBitmap != *pBitmap) {
                    PFS_PRINTF("bitmap unmatch %08lx, %08lx\n", *pRawBitmap, *pBitmap);
#ifdef FSCK100
                    if (fsckPromptUserAction(" Replace", 1) != 0)
#else
                    if (fsckPromptUserAction2(" Replace", 1) != 0)
#endif
                    {
                        *pRawBitmap = *pBitmap;
                        hasUpdate   = 1;
                    }
                }
            }

            if (hasUpdate != 0) {
                mount->blockDev->transfer(mount->fd, IOBuffer, i, (ZoneStartOffset + zone) << mount->sector_scale, 1 << mount->sector_scale, PFS_IO_MODE_WRITE);
            }
        }
    }
}

// 0x00001f34
static void FsckThread(void *arg)
{
    pfs_cache_t *clink;
    pfs_mount_t *mount       = (pfs_mount_t *)arg;
    pfs_super_block_t *super = (pfs_super_block_t *)IOBuffer;

#ifdef FSCK100
    if (fsckVerbosityLevel > 0) {
        PFS_PRINTF("Check Root Directory...\n");
    }
#else
    if (fsckVerbosityLevel > 0) {
        PFS_PRINTF("Check Extended attribute...\n");
    }

    if (fsckCheckExtendedAttribute(mount) < 0) {
        PFS_PRINTF("error: I cannot continue, giving up.\n");
        goto fsck_thread_end;
    }

    if (fsckVerbosityLevel > 0) {
        PFS_PRINTF("done.\n");

        PFS_PRINTF("Check Root Directory...\n");
    }
#endif

    if ((clink = CheckRootDirectory(mount)) == NULL) {
        PFS_PRINTF("error: I cannot continue, giving up.\n");
        goto fsck_thread_end;
    }

    fsckRuntimeData.status.directories++;

    if (fsckVerbosityLevel > 0)
        PFS_PRINTF("done.\n");

    if (fsckVerbosityLevel > 0)
        PFS_PRINTF("Check all files...\n");

    fsckCheckFiles(clink, clink);

    if (fsckVerbosityLevel > 0)
        PFS_PRINTF("done.\n");

    pfsCacheFlushAllDirty(mount);
    pfsCacheFree(clink);

    // 0x00002030
    if (fsckRuntimeData.hasError == 0) {
        if (fsckRuntimeData.stopFlag == 0) {
            if (fsckVerbosityLevel > 0) {
                PFS_PRINTF("Compare bitmap...\n");
            }

            fsckCompareBitmap(mount, IOBuffer);

            if (fsckVerbosityLevel > 0) {
                PFS_PRINTF("done.\n");
            }
        }

        // 0x000020ac
        if (fsckRuntimeData.hasError == 0) {
            if (fsckRuntimeData.stopFlag == 0) { // Clear the write error state, if it was set.
                if (mount->blockDev->transfer(mount->fd, super, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_READ) == 0) {
                    if (super->pfsFsckStat & PFS_FSCK_STAT_WRITE_ERROR) {
                        super->pfsFsckStat &= ~PFS_FSCK_STAT_WRITE_ERROR;
                        mount->blockDev->transfer(mount->fd, super, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_WRITE);
                    }
                }

                iomanX_ioctl2(mount->fd, HIOCGETPARTERROR, NULL, 0, NULL, 0);
            }
        }
    }

    // 0x00002164
    if (fsckRuntimeData.status.fixedErrorCount != 0) { // Indicate that errors were fixed.
        if (mount->blockDev->transfer(mount->fd, super, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_READ) == 0) {
            super->pfsFsckStat |= PFS_FSCK_STAT_ERRORS_FIXED;
            mount->blockDev->transfer(mount->fd, super, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_WRITE);
        }
    }

    mount->blockDev->flushCache(mount->fd);

fsck_thread_end:
    SetEventFlag(fsckEventFlagID, 1);
}

// 0x0000264c
static int FsckUnsupported(void)
{
    return 0;
}

// 0x00000340
static int fsckCheckBitmap(pfs_mount_t *mount, void *buffer)
{
    u32 i, count, block, BitmapStart, sector;
    int result;

    result = 0;
    for (i = 0; i < mount->num_subs + 1; i++) {
        block = 0;
        for (block = 0, count = pfsGetBitmapSizeBlocks(mount->sector_scale, ZoneSizes[i]); block < count; block++) {
            BitmapStart = block + 1;
            if (i == 0) {
                BitmapStart += 0x2000 >> mount->sector_scale;
            }

            if ((result = mount->blockDev->transfer(mount->fd, buffer, i, BitmapStart << mount->sector_scale, 1 << mount->sector_scale, PFS_IO_MODE_READ)) < 0) {
                PFS_PRINTF("cannot read bitmap\n");
                if (fsckPromptUserAction(" Overwrite", 1) == 0) {
                    return result;
                }

                for (sector = 0, result = 0; sector < (u32)(1 << mount->sector_scale); sector++) {
                    // 0x0000044c
                    if (mount->blockDev->transfer(mount->fd, buffer, i, (BitmapStart << mount->sector_scale) + sector, 1, PFS_IO_MODE_READ) < 0) {
                        memset(buffer, -1, 512);
                        if ((result = mount->blockDev->transfer(mount->fd, buffer, i, (BitmapStart << mount->sector_scale) + sector, 1, PFS_IO_MODE_WRITE)) < 0) {
                            PFS_PRINTF("error: overwrite bitmap failed.\n");
                            return result;
                        }
                    }
                }
            }
        }
    }

    return result;
}

// 0x000018b8
static int CheckSuperBlock(pfs_mount_t *pMainPFSMount)
{
    int result, i;
    pfs_super_block_t *super = (pfs_super_block_t *)IOBuffer;

    pMainPFSMount->num_subs = pMainPFSMount->blockDev->getSubNumber(pMainPFSMount->fd);
    if (pMainPFSMount->blockDev->transfer(pMainPFSMount->fd, super, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_READ) < 0 || (super->magic != PFS_SUPER_MAGIC)) {
        // 0x00001930
        PFS_PRINTF("Read super block failed, try another.\n");

        if (pMainPFSMount->blockDev->transfer(pMainPFSMount->fd, IOBuffer, 0, PFS_SUPER_BACKUP_SECTOR, 1, PFS_IO_MODE_READ) != 0) {
            PFS_PRINTF("error: could not read any super block.\n");
            return -EIO;
        }

        result = (super->magic == PFS_SUPER_MAGIC) ? 0 : -EIO;

        if (result != 0) {
            PFS_PRINTF("error: could not read any super block.\n");
            return -EIO;
        }

        if (fsckPromptUserAction(" Overwrite super block", 1) == 0) {
            return -EIO;
        }

        if ((result = pMainPFSMount->blockDev->transfer(pMainPFSMount->fd, IOBuffer, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_WRITE)) < 0) {
            PFS_PRINTF("error: overwrite failed.\n");
            return result;
        }
    }

    // 0x00001a1c
    if (super->version > PFS_FORMAT_VERSION) {
        PFS_PRINTF("error: unknown version.\n");
        return -EINVAL;
    }

    if (((super->zone_size & (super->zone_size - 1)) != 0) ||
        (super->zone_size < 0x800) ||
        (0x20000 < super->zone_size)) {
        PFS_PRINTF("error: invalid zone size.\n");
        return -EINVAL;
    }

    if (pMainPFSMount->num_subs < super->num_subs) {
        PFS_PRINTF("filesystem larger than partition size.\n");

        if (fsckPromptUserAction(" Fix size", 1) == 0) {
            return -EINVAL;
        }

        super->num_subs = pMainPFSMount->num_subs;
        if ((result = pMainPFSMount->blockDev->transfer(pMainPFSMount->fd, IOBuffer, 0, PFS_SUPER_SECTOR, 1, PFS_IO_MODE_WRITE)) < 0) {
            PFS_PRINTF("error: could not fix the filesystem size.\n");
            return result;
        }
    }

    pMainPFSMount->zsize        = super->zone_size;
    pMainPFSMount->sector_scale = pfsGetScale(super->zone_size, 512);
    pMainPFSMount->inode_scale  = pfsGetScale(super->zone_size, 1024);

    memcpy(&pMainPFSMount->root_dir, &super->root, sizeof(pMainPFSMount->root_dir));
    memcpy(&pMainPFSMount->log, &super->log, sizeof(pMainPFSMount->log));
    memcpy(&pMainPFSMount->current_dir, &super->root, sizeof(pMainPFSMount->current_dir));
    pMainPFSMount->total_zones = 0;

    if (fsckVerbosityLevel) {
        PFS_PRINTF("\tlog check...\n");
    }

    if ((result = pfsJournalRestore(pMainPFSMount)) < 0)
        return result;

    memset(ZoneSizes, 0, 0x10);
    memset(ZoneMap, 0, 0x10);

    for (i = 0; (u32)i < pMainPFSMount->num_subs + 1; i++) {
        ZoneSizes[i] = pMainPFSMount->blockDev->getSize(pMainPFSMount->fd, i);
        if (i != 0) {
            ZoneMap[i] = (ZoneSizes[i - 1] >> pMainPFSMount->sector_scale) + ZoneMap[i - 1];
        }
    }

    if (fsckVerbosityLevel > 0) {
        PFS_PRINTF("\tCheck Bitmaps...\n");
    }

    // 0x00001c80
    if ((result = fsckCheckBitmap(pMainPFSMount, IOBuffer)) >= 0) {
        u32 *pFreeZones;

        if (fsckVerbosityLevel > 0) {
            PFS_PRINTF("\tdone.\n");
        }

        for (i = 0, pFreeZones = pMainPFSMount->free_zone; (u32)i < pMainPFSMount->num_subs + 1; i++, pFreeZones++) {
            pMainPFSMount->total_zones += ZoneSizes[i] >> pMainPFSMount->sector_scale;
            pMainPFSMount->zfree += (*pFreeZones = pfsBitmapCalcFreeZones(pMainPFSMount, i));
        }

        if (fsckVerbosityLevel > 0) {
            PFS_PRINTF("zonesz %ld, %ld zones, %ld free.\n", pMainPFSMount->zsize, pMainPFSMount->total_zones, pMainPFSMount->zfree);
        }
    }

    return result;
}

// 0x00002224
static int FsckOpen(iomanX_iop_file_t *fd, const char *name, int flags, int mode)
{
    int blockfd, result;
    u32 count;
    iox_stat_t StatData;
    pfs_block_device_t *pblockDevData;

    (void)flags;

    fsckWriteEnabled   = mode & FSCK_MODE_WRITE;
    fsckAutoMode       = mode & FSCK_MODE_AUTO;
    fsckVerbosityLevel = (mode & 0xF0) >> 4;

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

    if ((pblockDevData = pfsGetBlockDeviceTable(name)) == NULL) {
        return -ENXIO;
    }

    if ((blockfd = iomanX_open(name, O_RDWR)) < 0) {
        PFS_PRINTF("error: cannot open.\n");
        return blockfd;
    }

    memset(&MainPFSMount, 0, sizeof(MainPFSMount));
    MainPFSMount.fd       = blockfd;
    MainPFSMount.blockDev = pblockDevData;

    if (fsckVerbosityLevel > 0) {
        PFS_PRINTF("Check Super Block...\n");
    }

    if ((result = CheckSuperBlock(&MainPFSMount)) < 0) {
        MainPFSMount.fd = 0;
        PFS_PRINTF("error: cannot continue.\n");
        return result;
    }

    if (fsckVerbosityLevel > 0) {
        PFS_PRINTF("done.\n");
    }

    if ((result = pfsBitmapPartInit(MainPFSMount.total_zones)) >= 0) {
        int i;

        // 0x000023bc
        memset(&fsckRuntimeData, 0, sizeof(fsckRuntimeData));

        fsckRuntimeData.status.zoneUsed = MainPFSMount.total_zones - MainPFSMount.zfree;
        for (i = 0; (u32)i < MainPFSMount.num_subs + 1; fsckRuntimeData.status.inodeBlockCount += count, i++) {
            count = pfsGetBitmapSizeBlocks(MainPFSMount.sector_scale, ZoneSizes[i]) + 1;
            if (i == 0) {
                count += (0x2000 >> MainPFSMount.sector_scale) + MainPFSMount.log.count;
            }

            if (fsckCheckZones(ZoneMap[i], count) < 0) {
                break;
            }
        }

        // 0x0000246c
        fd->privdata = &MainPFSMount;
        result       = 0;
    } else {
        MainPFSMount.fd = 0;
    }

    return result;
}

// 0x000024a4
static int FsckClose(iomanX_iop_file_t *fd)
{
    iomanX_close(((pfs_mount_t *)fd->privdata)->fd);
    pfsCacheClose((pfs_mount_t *)fd->privdata);
    memset(fd->privdata, 0, sizeof(pfs_mount_t));

    return 0;
}

// 0x00001d7c
static int fsckGetEstimatedTime(pfs_mount_t *mount, int *result)
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

        if (usec < clock) {
            clock = usec;
        }
    }

    // 0x00001e8c
    if ((*result = ((clock + 400) * fsckRuntimeData.status.zoneUsed / 1000000)) == 0) {
        *result = 1;
    }

    return 0;
}

// 0x000024f0
static int FsckIoctl2(iomanX_iop_file_t *fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int result;
    u32 FlagBits;

    (void)arg;
    (void)arglen;
    (void)buflen;

    switch (cmd) {
        case FSCK_IOCTL2_CMD_GET_ESTIMATE: // 0x00002528
            result = fsckGetEstimatedTime(fd->privdata, buf);
            break;
        case FSCK_IOCTL2_CMD_START: // 0x0000253c
            result = StartThread(fsckThreadID, fd->privdata);
            break;
        case FSCK_IOCTL2_CMD_WAIT: // 0x00002554
            result = WaitEventFlag(fsckEventFlagID, 1, WEF_OR | WEF_CLEAR, &FlagBits);
            break;
        case FSCK_IOCTL2_CMD_POLL: // 0x00002574
            result = PollEventFlag(fsckEventFlagID, 1, WEF_OR | WEF_CLEAR, &FlagBits);
            if (result == KE_EVF_COND) {
                result = 1;
            }
            break;
        case FSCK_IOCTL2_CMD_GET_STATUS: // 0x000025a8
            memcpy(buf, &fsckRuntimeData.status, sizeof(struct fsckStatus));
            result = 0;
            break;
        case FSCK_IOCTL2_CMD_STOP: // 0x0000262c
            fsckRuntimeData.stopFlag = 1;
            result                   = 0;
            break;
        default:
            result = 0;
    }

    return result;
}

static iomanX_iop_device_ops_t FsckDeviceOps = {
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    NULL,
    &FsckOpen,
    &FsckClose,
    NULL,
    NULL,
    NULL,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    (void *)&FsckUnsupported,
    &FsckIoctl2};

static iomanX_iop_device_t FsckDevice = {
    "fsck",
    IOP_DT_FSEXT | IOP_DT_FS,
    1,
    "FSCK",
    &FsckDeviceOps};

// 0x0000267c
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

    PFS_PRINTF("max depth %d, %d buffers.\n", FSCK_MAX_PATH_LEVELS - 1, buffers);

    if (pfsCacheInit(buffers, 1024) < 0) {
        PFS_PRINTF("error: cache initialization failed.\n");
        return MODULE_NO_RESIDENT_END;
    }

    if (pfsBitmapInit() < 0) {
        PFS_PRINTF("error: bitmap initialization failed.\n");
        return MODULE_NO_RESIDENT_END;
    }

    if ((fsckEventFlagID = fsckCreateEventFlag()) < 0) {
        return MODULE_NO_RESIDENT_END;
    }

    if ((fsckThreadID = fsckCreateThread(&FsckThread, 0x2080)) < 0) {
        return MODULE_NO_RESIDENT_END;
    }

    iomanX_DelDrv(FsckDevice.name);
    if (iomanX_AddDrv(&FsckDevice) == 0) {
        PFS_PRINTF("version %04x driver start.\n", IRX_VER(PFS_MAJOR, PFS_MINOR));
        return MODULE_RESIDENT_END;
    }

    return MODULE_NO_RESIDENT_END;
}
