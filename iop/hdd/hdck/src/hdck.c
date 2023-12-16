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
#include <stdio.h>
#include <sysclib.h>
#include <irx.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include "libapa.h"

#include "misc_hdck.h"

#ifdef _IOP
IRX_ID("hdck", APA_MODVER_MAJOR, APA_MODVER_MINOR);
#endif

// Function prototypes
static int HdckInit(iomanX_iop_device_t *device);
static int HdckUnsupported(void);
static int HdckDevctl(iomanX_iop_file_t *fd, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

static u8 IOBuffer[128 * 512];
static u8 IOBuffer2[128 * 512];

static iomanX_iop_device_ops_t HdckDeviceOps = {
    &HdckInit,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    (void *)&HdckUnsupported,
    &HdckDevctl,
    NULL,
    NULL,
    NULL};

static iomanX_iop_device_t HdckDevice = {
    "hdck",
    IOP_DT_FSEXT | IOP_DT_FS,
    1,
    "HDCK",
    &HdckDeviceOps};

struct HddInfo
{
    u32 sectors;
    u32 MaxPartSize;
    unsigned int format;
    unsigned int status;
};

struct HdckPrivateData
{
    struct HddInfo HddInfo[2];
    // When the HDD is scanned, it is scanned in both ways (forward and backward). Details on up to two erraneous partitions can be recorded.
    u32 ErrorPartitionLBA;
    u32 ErrorPartition2LBA;
    u32 ErrorPartitionPrevLBA;
    u32 ErrorPartition2PrevLBA;
};

static struct HdckPrivateData PrivateData = {
    {
        {0, 0, 0, 3},
        {0, 0, 0, 3},
    },
    0,
    0,
    0,
    0};

static int HdckUnsupported(void)
{
    return -1;
}

static int HdckInit(iomanX_iop_device_t *device)
{
    (void)device;

    return 0;
}

#ifdef HDCK_CHECK_CROSSLINK
static int AddPartitionToList(u32 *list, unsigned int *pCount, u32 sector)
{
    unsigned int i;
    int result;

    for (i = 0; i < *pCount; i++) {
        if (list[i] == sector) {
            break;
        }
    }

    if (i == *pCount) {
        list[i] = sector;
        (*pCount)++;
        result = 0;
    } else {
        APA_PRINTF("found cross-linked partition: 0x%lx\n", sector);
        result = -EEXIST;
    }

    return result;
}
#endif

static int CheckAPAPartitionLinks(int device, apa_cache_t *clink)
{
    int result;
#ifdef HDCK_CHECK_CROSSLINK
    unsigned int partitions;
#endif
    apa_cache_t *clink2;
    u32 CurrentLBA, ParentLBA;

    APA_PRINTF("scan partitions.\n");

    result                             = 0;
    PrivateData.ErrorPartition2LBA     = 0;
    PrivateData.ErrorPartitionLBA      = 0;
    PrivateData.ErrorPartition2PrevLBA = 0;
    PrivateData.ErrorPartitionPrevLBA  = 0;
    ParentLBA                          = 0;

#ifdef HDCK_CHECK_CROSSLINK
    partitions = 0;
#endif
    CurrentLBA = clink->header->next;
    while (CurrentLBA != 0
#ifdef HDCK_CHECK_CROSSLINK
           && ((result = AddPartitionToList((u32 *)IOBuffer, &partitions, CurrentLBA)) >= 0)
#endif
           && (clink2 = apaCacheGetHeader(device, CurrentLBA, 0, &result)) != NULL) {
        if (clink2->header->prev != ParentLBA) {
            APA_PRINTF("found invalid previous partition address, fix it.\n");
            clink2->header->prev = ParentLBA;
            clink2->flags |= APA_CACHE_FLAG_DIRTY;
            apaCacheFlushAllDirty(device);
        }

        ParentLBA  = clink2->header->start;
        CurrentLBA = clink2->header->next;
        apaCacheFree(clink2);
    }

    if (result == 0) {
        if (clink->header->prev != ParentLBA) {
            APA_PRINTF("found invalid last partition address, fix it.\n");
            clink->header->prev = ParentLBA;
            clink->flags |= APA_CACHE_FLAG_DIRTY;
            apaCacheFlushAllDirty(device);
        }

        APA_PRINTF("we do not have an error partition.\n");
    } else {
        APA_PRINTF("found error partition.\n");

        PrivateData.ErrorPartitionLBA     = CurrentLBA;
        PrivateData.ErrorPartitionPrevLBA = ParentLBA;

        // Now scan the HDD, backwards.
        CurrentLBA = clink->header->prev;
        ParentLBA  = 0;
#ifdef HDCK_CHECK_CROSSLINK
        partitions = 0;
#endif
        while (CurrentLBA != 0
#ifdef HDCK_CHECK_CROSSLINK
               && ((result = AddPartitionToList((u32 *)IOBuffer, &partitions, CurrentLBA)) >= 0)
#endif
               && (clink2 = apaCacheGetHeader(device, CurrentLBA, 0, &result)) != NULL) {
            if (clink2->header->next != ParentLBA) {
                APA_PRINTF("found invalid next partition address, fix it.\n");

                clink2->header->next = ParentLBA;
                clink2->flags |= APA_CACHE_FLAG_DIRTY;
                apaCacheFlushAllDirty(device);
            }

            ParentLBA  = clink2->header->start;
            CurrentLBA = clink2->header->prev;
            apaCacheFree(clink2);
        }

        if (result == 0) {
            if (clink->header->next != ParentLBA) {
                APA_PRINTF("found invalid first partition address, fix it.\n");
                clink->header->next = ParentLBA;
                clink->flags |= APA_CACHE_FLAG_DIRTY;
                apaCacheFlushAllDirty(device);
            }

            APA_PRINTF("found inconsistency, but already fixed.\n");
            APA_PRINTF("we do not have an error partition.\n");

            PrivateData.ErrorPartitionLBA = 0;
        } else {
            APA_PRINTF("found error partition.\n");

            PrivateData.ErrorPartition2LBA     = CurrentLBA;
            PrivateData.ErrorPartition2PrevLBA = ParentLBA;
        }
    }

    return PrivateData.ErrorPartitionLBA;
}

static void EraseSector(int unit, void *buffer, u32 lba)
{
    memset(buffer, 0, 512);
    sceAtaDmaTransfer(unit, buffer, lba, 1, ATA_DIR_WRITE);
    sceAtaFlushCache(unit);
}

static void RemoveBadPartitions(int device, apa_cache_t *clink)
{
    apa_cache_t *clink2;
    int result;

    APA_PRINTF("remove all partitions after unreadable one.\n");

    if ((clink2 = apaCacheGetHeader(device, PrivateData.ErrorPartitionPrevLBA, 0, &result)) != NULL) {
        clink2->header->next = 0;
        clink->header->prev  = PrivateData.ErrorPartitionPrevLBA;
        clink2->header->flags |= APA_CACHE_FLAG_DIRTY;
        clink->header->flags |= APA_CACHE_FLAG_DIRTY;

        apaCacheFlushAllDirty(device);
        apaCacheFree(clink2);

        PrivateData.ErrorPartition2LBA = 0;
        PrivateData.ErrorPartitionLBA  = 0;
    }
}

static void RecoverSubPartition(apa_cache_t *clink, u32 lba, u32 previous, u32 sublength, u32 substart, u16 type, int sub)
{
    int result;
    apa_cache_t *clink2;

    clink2 = apaCacheGetHeader(clink->device, lba, 1, &result);

    APA_PRINTF("found this sub partition's main, so I recover it.\n");

    memset(clink2->header, 0, sizeof(apa_header_t));
    clink2->header->magic = APA_MAGIC;
    clink2->header->start = lba;
    if (lba < clink->header->prev) {
        clink2->header->next = lba + sublength;
    }
    clink2->header->prev   = previous;
    clink2->header->length = sublength;
    clink2->header->type   = type;
    clink2->header->flags  = APA_FLAG_SUB;
    clink2->header->main   = substart;
    clink2->header->number = sub;

    apaGetTime(&clink2->header->created);

    clink2->flags |= APA_CACHE_FLAG_DIRTY;

    apaCacheFlushAllDirty(clink->device);
    apaCacheFree(clink2);
}

// Checks if the damaged partition(s) is/are sub-partition(s) of another partition. If so, attempt to recover it/them.
static void RecoverPartitionIfSub(int device, apa_cache_t *clink)
{
    int result, count, sub;
    apa_cache_t *clink2;
    u32 NextPartSector;
    apa_sub_t *pSubs;

    APA_PRINTF("check if error partition is belong to someone as sub partition.\n");

    // Start by cycling forward through the partition list.
    // If there is at least one more partition (other than __mbr) and the damaged partition is not the first partition.
    if ((NextPartSector = clink->header->next) != 0 && PrivateData.ErrorPartitionLBA != clink->header->next) {
        while ((clink2 = apaCacheGetHeader(device, NextPartSector, 0, &result)) != NULL) {
            if (clink2->header->type && !(clink2->header->flags & APA_FLAG_SUB)) {
                for (count = 1, sub = 0, pSubs = clink2->header->subs; (u32)sub < clink2->header->nsub; sub++, pSubs++, count++) {
                    if (PrivateData.ErrorPartitionLBA && pSubs->start == PrivateData.ErrorPartitionLBA) {
                        RecoverSubPartition(clink, PrivateData.ErrorPartitionLBA, PrivateData.ErrorPartitionPrevLBA, pSubs->length, clink2->header->start, clink2->header->type, count);

                        if (PrivateData.ErrorPartition2LBA == PrivateData.ErrorPartitionLBA) {
                            PrivateData.ErrorPartition2LBA = 0;
                        }
                        PrivateData.ErrorPartitionLBA = 0;
                    }

                    if (PrivateData.ErrorPartition2LBA && pSubs->start == PrivateData.ErrorPartition2LBA) {
                        RecoverSubPartition(clink, PrivateData.ErrorPartition2LBA, PrivateData.ErrorPartitionLBA, pSubs->length, clink2->header->start, clink2->header->type, count);

                        PrivateData.ErrorPartition2LBA = 0;
                    }
                }
            }

            NextPartSector = clink2->header->next;
            apaCacheFree(clink2);

            if (!PrivateData.ErrorPartitionLBA && !PrivateData.ErrorPartition2LBA) {
                return;
            }

            if (!NextPartSector || NextPartSector == PrivateData.ErrorPartitionLBA) {
                break;
            }
        }
    }

    /*    Cycle backward through the partition list.
        If there is at least one more partition (other than __mbr) and the damaged partition is not the last partition.    */
    if ((NextPartSector = clink->header->prev) != 0 && clink->header->prev != PrivateData.ErrorPartition2LBA) {
        while ((clink2 = apaCacheGetHeader(device, NextPartSector, 0, &result)) != NULL) {
            if (clink2->header->type && !(clink2->header->flags & APA_FLAG_SUB)) {
                for (count = 1, sub = 0, pSubs = clink2->header->subs; (u32)sub < clink2->header->nsub; sub++, pSubs++, count++) {
                    if (PrivateData.ErrorPartitionLBA && pSubs->start == PrivateData.ErrorPartitionLBA) {
                        RecoverSubPartition(clink, PrivateData.ErrorPartitionLBA, PrivateData.ErrorPartitionPrevLBA, pSubs->length, clink2->header->start, clink2->header->type, count);

                        if (PrivateData.ErrorPartition2LBA == PrivateData.ErrorPartitionLBA) {
                            PrivateData.ErrorPartition2LBA = 0;
                        }
                        PrivateData.ErrorPartitionLBA = 0;
                    }

                    if (PrivateData.ErrorPartition2LBA && pSubs->start == PrivateData.ErrorPartition2LBA) {
                        RecoverSubPartition(clink, PrivateData.ErrorPartition2LBA, PrivateData.ErrorPartitionLBA, pSubs->length, clink2->header->start, clink2->header->type, count);

                        PrivateData.ErrorPartition2LBA = 0;
                    }
                }
            }

            NextPartSector = clink2->header->prev;
            apaCacheFree(clink2);

            if (!PrivateData.ErrorPartitionLBA && !PrivateData.ErrorPartition2LBA) {
                return;
            }

            if (!NextPartSector || NextPartSector == PrivateData.ErrorPartitionLBA) {
                break;
            }
        }
    }
}

static apa_cache_t *apaCreateAlignedEmptyPartition(apa_cache_t *clink, u32 lba, u32 length)
{
    apa_cache_t *result;

    for (length >>= 1; 0x0003FFFF < length; length >>= 1) {
        if (lba % length == 0) {
            result              = apaRemovePartition(clink->device, lba, lba + length, clink->header->start, length);
            clink->header->next = lba;
            clink->flags |= APA_CACHE_FLAG_DIRTY;
            apaCacheFlushAllDirty(clink->device);
            apaCacheFree(clink);

            return result;
        }
    }

    return NULL;
}

static void MarkUnreadablePartionAsEmpty(int device, u32 lba, u32 parent, u32 length)
{
    apa_cache_t *clink;
    u32 new_length;
    int result;

    new_length = 0;
    clink      = apaCacheGetHeader(device, parent, 0, &result);
    APA_PRINTF("make unreadable partition as empty.\n");

    if (clink != NULL) {
        while (length != 0) {
            int i;

            for (i = 31; i >= 0; i--) {
                if ((new_length = ((u32)1) << i) & length) {
                    break;
                }
            }

            if (lba % new_length) {
                clink = apaCreateAlignedEmptyPartition(clink, lba, new_length);
            } else {
                apaRemovePartition(device, lba, lba + new_length, clink->header->start, new_length);
                clink->header->next = lba;
                clink->flags |= APA_CACHE_FLAG_DIRTY;
                apaCacheFlushAllDirty(device);
                apaCacheFree(clink);
            }

            length -= clink->header->length;
        }

        apaCacheFree(clink);
        PrivateData.ErrorPartition2LBA = 0;
        PrivateData.ErrorPartitionLBA  = 0;
    }
}

static void DeleteFreePartitions(int device, apa_cache_t *clink)
{
    apa_cache_t *clink2;
    int result;
    u32 sector;

    sector = clink->header->next;
    while (sector != 0 && (clink2 = apaCacheGetHeader(device, sector, 0, &result)) != NULL) {
        if (clink2->header->type == 0) {
            if ((clink2 = apaDeleteFixPrev(clink2, &result)) == NULL) {
                break;
            }
        }

        sector = clink2->header->next;
        apaCacheFree(clink2);
    }
}

// Generates lists of all main and sub-partitions on the disk.
static int ReadPartitionMap(int device, apa_cache_t *clink, int *pMainCount, int *pSubCount)
{
    int result;
    apa_cache_t *clink2;
    u32 sector, *MainMap, *SubMap;

    *pMainCount = 0;
    *pSubCount  = 0;
    result      = 0;
    sector      = clink->header->next;
    MainMap     = (u32 *)IOBuffer;
    SubMap      = (u32 *)IOBuffer2;
    while (sector != 0) {
        if ((clink2 = apaCacheGetHeader(device, sector, 0, &result)) != NULL) {
            if (clink2->header->type) {
                if (!(clink2->header->flags & APA_FLAG_SUB)) {
                    APA_PRINTF("main, start %08lx, nsector %08lx, nsub %ld, id %s\n", clink2->header->start, clink2->header->length, clink2->header->nsub, clink2->header->id);

                    MainMap[(*pMainCount)] = clink2->header->start;
                    (*pMainCount)++;
                }
                if (clink2->header->type && clink2->header->flags & APA_FLAG_SUB) {
                    APA_PRINTF("sub , start %08lx, nsector %08lx, num %2ld, main %08lx\n", clink2->header->start, clink2->header->length, clink2->header->number, clink2->header->main);
                    SubMap[(*pSubCount)] = clink2->header->start;
                    (*pSubCount)++;
                }
            }

            sector = clink2->header->next;
            apaCacheFree(clink2);
        } else {
            break;
        }
    }

    return result;
}

// Goes through the partitions and checks the relationships between the main and sub-partitions.
static int CheckPartitions(int device, apa_cache_t *clink)
{
    int result, MainCount, SubCount;

    if ((result = ReadPartitionMap(device, clink, &MainCount, &SubCount)) == 0) {
        int i, sub, count;
        apa_cache_t *clink2, *clink_sub;

        APA_PRINTF("check main partitions.\n");

        for (i = 0; i < MainCount && result == 0; i++) {
            if ((clink2 = apaCacheGetHeader(device, ((u32 *)IOBuffer)[i], 0, &result)) != NULL) {
                int missing_sub_found;
                apa_sub_t *pSubs;

                // Check through the list of sub-partitions, to see if a the sub-partition belongs to a main partition.
                for (missing_sub_found = 0, sub = 0, pSubs = clink2->header->subs; (u32)sub < clink2->header->nsub; sub++, pSubs++) {
                    int flag;

                    for (flag = 0, count = 0; count < SubCount; count++) {
                        if (pSubs->start == ((u32 *)IOBuffer2)[count]) {
                            flag = 1;
                            break;
                        }
                    }

                    if (!flag) {
                        // Sub-partition does not belong to anything.
                        APA_PRINTF("missing sub partition found.\n");

                        if (!missing_sub_found) {
                            clink2->header->nsub = sub;
                            clink2->flags |= APA_CACHE_FLAG_DIRTY;
                            missing_sub_found = 1; //"missing sub" found.
                            apaCacheFlushAllDirty(device);
                        }
                    } else {
                        // Sub-partition belongs to something, but follows a missing partition.
                        if (missing_sub_found) {
                            APA_PRINTF("remove sub partition follows missing partition.\n");

                            if ((clink_sub = apaCacheGetHeader(device, pSubs->start, 0, &result)) != NULL) {
                                result = apaDelete(clink_sub);
                            }
                        }
                    } // Otherwise, the sub-partition is okay.
                }

                apaCacheFree(clink2);
            }
        }

        if (result >= 0) {
            if ((result = ReadPartitionMap(device, clink, &MainCount, &SubCount)) == 0) {
                u32 *ptr;

                APA_PRINTF("check sub partitions.\n");

                for (sub = 0, ptr = (u32 *)IOBuffer2; sub < SubCount && result == 0; ptr++, sub++) {
                    // Check through the list of main partitions, to see if the sub-partition belongs to a main partition.
                    if ((clink2 = apaCacheGetHeader(device, *ptr, 0, &result)) != NULL) {
                        for (i = 0; ((u32 *)IOBuffer)[i] != clink2->header->main && i < MainCount; i++)
                            ;

                        if (MainCount == i) {
                            APA_PRINTF("this sub(start = %08lx)is not belong to anyone, remove it.\n", clink2->header->start);
                            if ((result = apaDelete(clink2)) != 0) {
                                APA_PRINTF("remove partition failed.\n");
                            }
                        } else {
                            // Check if the sub-partition belongs to the main partition's sub-partition list.
                            if ((clink_sub = apaCacheGetHeader(device, clink2->header->main, 0, &result)) != NULL) {
                                for (count = 0; (u32)count < clink2->header->nsub; count++) {
                                    if (clink_sub->header->subs[count].start == *ptr) {
                                        break;
                                    }
                                }

                                if (clink_sub->header->nsub == (u32)count) {
                                    APA_PRINTF("this subpartition is not belong to sub list.\n");
                                    apaDelete(clink2);
                                    apaCacheFree(clink_sub);
                                } else {
                                    apaCacheFree(clink_sub);
                                    apaCacheFree(clink2);
                                }
                            } else {
                                apaCacheFree(clink2);
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }

    return result;
}

static int HdckDevctl(iomanX_iop_file_t *fd, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int result, badParts;
    apa_cache_t *clink;

    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    if (PrivateData.HddInfo[fd->unit].status != 0) {
        return -ENXIO;
    }

    if ((clink = apaCacheGetHeader(fd->unit, APA_SECTOR_MBR, 0, &result)) != NULL) {
        badParts = 0;
        while (CheckAPAPartitionLinks(fd->unit, clink) != 0) {
            // Both pointers point to the same partition.
            if (PrivateData.ErrorPartitionLBA == PrivateData.ErrorPartition2LBA) {
                APA_PRINTF("only one partition has problem, try to fix it.\n");
                RecoverPartitionIfSub(fd->unit, clink);

                // If the error is still there, decide on how to delete the partition.
                if (PrivateData.ErrorPartitionLBA && PrivateData.ErrorPartition2LBA) {
                    // If the partition is not the last partition, turn it into an empty partition. Otherwise, delete it.
                    if (clink->header->prev != PrivateData.ErrorPartitionLBA) {
                        MarkUnreadablePartionAsEmpty(fd->unit, PrivateData.ErrorPartitionLBA, PrivateData.ErrorPartitionPrevLBA, PrivateData.ErrorPartition2PrevLBA - PrivateData.ErrorPartitionLBA);
                    } else {
                        RemoveBadPartitions(fd->unit, clink);
                    }
                } else {
                    continue;
                }
                // More than one partition in-between the pointers are bad.
            } else if (PrivateData.ErrorPartitionLBA < PrivateData.ErrorPartition2LBA) {
                APA_PRINTF("two or more partitions have problem, try to fix it.\n");

                RecoverPartitionIfSub(fd->unit, clink);

                // If the error is still there, decide on how to delete the partition.
                if (PrivateData.ErrorPartitionLBA && PrivateData.ErrorPartition2LBA) {
                    // If both of the partitions are not the last partition, turn them both into empty partitions. Otherwise, delete them.
                    if (clink->header->prev != PrivateData.ErrorPartitionLBA && clink->header->prev != PrivateData.ErrorPartition2LBA) {
                        MarkUnreadablePartionAsEmpty(fd->unit, PrivateData.ErrorPartitionLBA, PrivateData.ErrorPartitionPrevLBA, PrivateData.ErrorPartition2PrevLBA - PrivateData.ErrorPartitionLBA);
                    } else {
                        RemoveBadPartitions(fd->unit, clink);
                    }
                } else {
                    continue;
                }
                // The pointers overlap, making it impossible to tell where the bad partitions certainly are. The only solution is to nuke 'em all.
            } else {
                APA_PRINTF("partition table completely inconsistent.\n");
                RemoveBadPartitions(fd->unit, clink);
            }

            // If there are more than 8 bad partitions, abort.
            badParts++;
            if (badParts == 9) {
                goto check_end;
            }
        }

        EraseSector(fd->unit, IOBuffer, APA_SECTOR_SECTOR_ERROR); // Erase the error sector record.
        DeleteFreePartitions(fd->unit, clink);
        result = CheckPartitions(fd->unit, clink);

    check_end:
        apaCacheFree(clink);
        APA_PRINTF("done\n");
    } else {
        APA_PRINTF("cannot read mbr partition, I cannot continue.\n");
    }

    return result;
}

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
            PrivateData.HddInfo[i].status--;
            PrivateData.HddInfo[i].sectors = pDevInfo->total_sectors;

            PrivateData.HddInfo[i].MaxPartSize = apaGetPartitionMax(pDevInfo->total_sectors);
            if (HdckUnlockHdd(i) == 0) {
                PrivateData.HddInfo[i].status--;
            }

            APA_PRINTF("disk%d: 0x%08lx sectors, max 0x%08lx\n", i, PrivateData.HddInfo[i].sectors, PrivateData.HddInfo[i].MaxPartSize);
        }
    }

    apaCacheInit(0x100);
    for (i = 0; i < 2; i++) {
        if (PrivateData.HddInfo[i].status < 2) {
            if (apaJournalRestore(i) != 0) {
                return MODULE_NO_RESIDENT_END;
            }

            if (apaGetFormat(i, (int *)&PrivateData.HddInfo[i].format)) {
                PrivateData.HddInfo[i].status--;
            }
        }
    }

    iomanX_DelDrv(HdckDevice.name);
    if (iomanX_AddDrv(&HdckDevice) == 0) {
        APA_PRINTF("driver start.\n");
        return MODULE_RESIDENT_END;
    }

    return MODULE_NO_RESIDENT_END;
}
