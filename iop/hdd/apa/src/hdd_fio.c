/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# APA File System routines
*/

#include <stdio.h>
#ifdef _IOP
#include <irx.h>
#include <loadcore.h>
#include <sysclib.h>
#else
#include <string.h>
#endif
#ifdef APA_USE_DEV9
#include <dev9.h>
#endif
#include <errno.h>
#include <iomanX.h>
#include <thsemap.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include <libapa.h>
#include "hdd.h"
#include "hdd_fio.h"
#include "hdd_blkio.h"

hdd_file_slot_t *hddFileSlots;
int fioSema;
int apaMaxOpen = 1;

extern const char apaMBRMagic[];
extern apa_device_t hddDevices[];

#ifdef APA_FORMAT_MAKE_PARTITIONS
// TODO: For DVRP firmware 48-bit, __xdata and __xcontents partitions are created
static const char *formatPartList[] = {
    "__net", "__system", "__sysconf", "__common", NULL};
#endif

#define APA_NUMBER_OF_SIZES 9
static const char *sizeList[APA_NUMBER_OF_SIZES] = {
    "128M", "256M", "512M", "1G", "2G", "4G", "8G", "16G", "32G"};

///////////////////////////////////////////////////////////////////////////////
// Function declarations
static int fioPartitionSizeLookUp(char *str);
static int fioInputBreaker(char const **arg, char *outBuf, int maxout);
static int fioDataTransfer(iomanX_iop_file_t *f, void *buf, int size, int mode);
static int getFileSlot(apa_params_t *params, hdd_file_slot_t **fileSlot);
static int ioctl2Transfer(s32 device, hdd_file_slot_t *fileSlot, hddIoctl2Transfer_t *arg);
static void fioGetStatFiller(apa_cache_t *clink1, iox_stat_t *stat);
static int ioctl2AddSub(hdd_file_slot_t *fileSlot, char *argp);
static int ioctl2DeleteLastSub(hdd_file_slot_t *fileSlot);
static int devctlSwapTemp(s32 device, char *argp);

static int fioPartitionSizeLookUp(char *str)
{
    int i;

    for (i = 0; i < APA_NUMBER_OF_SIZES; i++) {
        if (strcmp(str, sizeList[i]) == 0)
            return (256 * 1024) << i;
    }
    APA_PRINTF(APA_DRV_NAME ": Error: Invalid partition size, %s.\n", str);
    return -EINVAL;
}

static int fioInputBreaker(char const **arg, char *outBuf, int maxout)
{
    int len;
    char *p;

    if ((p = strchr(arg[0], ','))) {
        if (maxout < (len = p - arg[0]))
            return -EINVAL;
        memcpy(outBuf, arg[0], len);
        arg[0] = p + 1;
        while (arg[0][0] == ' ')
            arg[0] += 1;
        return 0;
    } // else
    if (maxout < (len = strlen(arg[0])))
        return -EINVAL;
    memcpy(outBuf, arg[0], len);
    arg[0] += len;
    return 0;
}

struct apaFsType
{
    const char *desc;
    u16 type;
};

static int fioGetInput(const char *arg, apa_params_t *params)
{
    char argBuf[32];
    int rv = 0;
    static const struct apaFsType fsTypes[] = {
#ifdef APA_SUPPORT_MBR
        {"MBR", APA_TYPE_MBR},
#endif
        {"EXT2SWAP", APA_TYPE_EXT2SWAP},
        {"EXT2", APA_TYPE_EXT2},
        {"REISER", APA_TYPE_REISER},
        {"PFS", APA_TYPE_PFS},
        {"CFS", APA_TYPE_CFS},
#ifdef APA_SUPPORT_HDL
        {"HDL", APA_TYPE_HDL},
#endif
    };

    if (params == NULL)
        return -EINVAL;
    memset(params, 0, sizeof(apa_params_t));

    while (arg[0] == ' ')
        arg++;

    if (arg[0] == 0 || arg[0] == ',')
        return -EINVAL;
    if ((rv = fioInputBreaker(&arg, params->id, APA_IDMAX)) != 0)
        return rv;
    if (arg[0] == '\0') // Return if there are no further parameters.
        return 0;

    if ((rv = fioInputBreaker(&arg, params->fpwd, APA_PASSMAX)) != 0)
        return rv;

    if (params->fpwd[0] != '\0')
        apaEncryptPassword(params->id, params->fpwd, params->fpwd);

    if (arg[0] == '\0') // Return if there are no further parameters.
        return 0;

    if ((rv = fioInputBreaker(&arg, params->rpwd, APA_PASSMAX)) != 0)
        return rv;

    if (params->rpwd[0] != '\0')
        apaEncryptPassword(params->id, params->rpwd, params->rpwd);

    if (arg[0] == '\0') // Return if there are no further parameters.
        return 0;

    memset(argBuf, 0, sizeof(argBuf));
    if ((rv = fioInputBreaker(&arg, argBuf, sizeof(argBuf))) != 0)
        return rv;

    if ((rv = fioPartitionSizeLookUp(argBuf)) < 0)
        return rv;
    params->size = rv;

    memset(argBuf, 0, sizeof(argBuf));
    if ((rv = fioInputBreaker(&arg, argBuf, sizeof(argBuf))) != 0)
        return rv;

    // scan through fstypes if found none - error
    {
        int i;

        for (i = 0; (unsigned int)i < (sizeof(fsTypes))/(sizeof(fsTypes[0])); i++) {
            if (!strcmp(argBuf, fsTypes[i].desc)) {
                params->type = fsTypes[i].type;
                break;
            }
        }

        if ((unsigned int)i == (sizeof(fsTypes))/(sizeof(fsTypes[0]))) {
            APA_PRINTF(APA_DRV_NAME ": error: Invalid fstype, %s.\n", argBuf);
            return -EINVAL;
        }
    }

    return rv;
}

static int getFileSlot(apa_params_t *params, hdd_file_slot_t **fileSlot)
{
    int i;

    for (i = 0; i < apaMaxOpen; i++) {
        if (hddFileSlots[i].f)
            if (memcmp(hddFileSlots[i].id, &params->id, APA_IDMAX) == 0)
                return -EBUSY; // file is open
    }
    for (i = 0; i < apaMaxOpen; i++) {
        if (!hddFileSlots[i].f) {
            *fileSlot = &hddFileSlots[i];
            return 0;
        }
    }
    return -EMFILE; // no file slots free :(
}

static int fioDataTransfer(iomanX_iop_file_t *f, void *buf, int size, int mode)
{
    hdd_file_slot_t *fileSlot = (hdd_file_slot_t *)f->privdata;

    if ((size & 0x1FF))
        return -EINVAL;
    size >>= 9; // size/512

    if (fileSlot->post + size >= 0x1FF9) // no over reading
        size = 0x1FF8 - fileSlot->post;

    if (size != 0) {
        int rv = 0;

        WaitSema(fioSema);
        if (blkIoDmaTransfer(f->unit, buf, fileSlot->post + fileSlot->parts[0].start + 8, size, mode))
            rv = -EIO;
        SignalSema(fioSema);
        if (rv == 0) {
            fileSlot->post += size;
            return size << 9;
        }

        return rv;
    }
    return 0;
}

static int ioctl2Transfer(s32 device, hdd_file_slot_t *fileSlot, hddIoctl2Transfer_t *arg)
{
    if (fileSlot->nsub < arg->sub)
        return -ENODEV;

    // main partitions can only be read starting from the 4MB offset.
    if (arg->sub == 0 && (arg->sector < 0x2000))
        return -EINVAL;
    // sub-partitions can only be read starting from after the header.
    if (arg->sub != 0 && (arg->sector < 2))
        return -EINVAL;

    if (fileSlot->parts[arg->sub].length < arg->sector + arg->size)
        return -ENXIO;

    if (blkIoDmaTransfer(device, arg->buffer,
                             fileSlot->parts[arg->sub].start + arg->sector, arg->size, arg->mode))
        return -EIO;

    return 0;
}

int hddInit(iomanX_iop_device_t *f)
{
    iop_sema_t sema;
    (void)f;

    sema.attr = 1;
    sema.initial = 1;
    sema.max = 1;
    sema.option = 0;
    fioSema = CreateSema(&sema);

    return 0;
}

int hddDeinit(iomanX_iop_device_t *f)
{
    (void)f;

    DeleteSema(fioSema);
    return 0;
}

int hddFormat(iomanX_iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen)
{
    int rv = 0;
    apa_cache_t *clink;
    u32 i;
#ifdef APA_FORMAT_MAKE_PARTITIONS
    apa_params_t params;
    u32 emptyBlocks[32];
#endif

	(void)dev;
	(void)blockdev;
	(void)arg;
	(void)arglen;

#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        return -ENODEV;
#endif

    if (f->unit >= BLKIO_MAX_VOLUMES)
        return -ENXIO;

    // clear all errors on hdd
    clink = apaCacheAlloc();
    memset(clink->header, 0, sizeof(apa_header_t));
    if (blkIoDmaTransfer(f->unit, clink->header, APA_SECTOR_SECTOR_ERROR, 1, BLKIO_DIR_WRITE)) {
        apaCacheFree(clink);
        return -EIO;
    }
    if (blkIoDmaTransfer(f->unit, clink->header, APA_SECTOR_PART_ERROR, 1, BLKIO_DIR_WRITE)) {
        apaCacheFree(clink);
        return -EIO;
    }
    // clear apa headers
    // TODO: Why does DVRP firmware start clearing at 1024 * 4104 when not 48-bit?
    // TODO: DVRP firmware 48-bit clears offset_24+0x2000, offset_24+0x42000, offset_24+0x242000
    for (i = 1024 * 8; i < hddDevices[f->unit].totalLBA; i += (1024 * 256)) {
        blkIoDmaTransfer(f->unit, clink->header, i, sizeof(apa_header_t) / 512,
                             BLKIO_DIR_WRITE);
    }
    apaCacheFree(clink);
    if ((rv = apaJournalReset(f->unit)) != 0)
        return rv;

    // set up mbr :)
    if ((clink = apaCacheGetHeader(f->unit, APA_SECTOR_MBR, APA_IO_MODE_WRITE, &rv))) {
        apa_header_t *header = clink->header;
        memset(header, 0, sizeof(apa_header_t));
        header->magic = APA_MAGIC;
        // TODO: DVRP firmware sets this to 0x400 bytes
        header->length = (1024 * 256); // 128MB
        header->type = APA_TYPE_MBR;
        // TODO: DVRP firmware 48-bit sets this to __extend
        strcpy(header->id, "__mbr");
#ifdef APA_FORMAT_LOCK_MBR
        apaEncryptPassword(header->id, header->fpwd, "sce_mbr");
        apaEncryptPassword(header->id, header->rpwd, "sce_mbr");
#endif
        memcpy(header->mbr.magic, apaMBRMagic, sizeof(header->mbr.magic));

        header->mbr.version = APA_MBR_VERSION;
        header->mbr.nsector = 0;
        apaGetTime(&header->created);
        apaGetTime(&header->mbr.created);

#ifdef APA_SUPPORT_GPT
        header->mbr.protective_mbr.UniqueMbrSignature = 0;
        header->mbr.protective_mbr.Unknown = 0;
        header->mbr.protective_mbr.partition_record1.BootIndicator = 0;
        header->mbr.protective_mbr.partition_record1.StartHead = 0;
        header->mbr.protective_mbr.partition_record1.StartSector = 2;
        header->mbr.protective_mbr.partition_record1.StartTrack = 0;
        header->mbr.protective_mbr.partition_record1.OSIndicator = 0xEE;
        header->mbr.protective_mbr.partition_record1.EndHead = 0xFF;
        header->mbr.protective_mbr.partition_record1.EndSector = 0xFF;
        header->mbr.protective_mbr.partition_record1.EndTrack = 0xFF;
        header->mbr.protective_mbr.partition_record1.StartingLBA = 1;
        header->mbr.protective_mbr.partition_record1.SizeInLBA = hddDevices[f->unit].totalLBA - header->mbr.protective_mbr.partition_record1.StartingLBA;
        header->mbr.protective_mbr.Signature = 0xAA55;
#endif

        header->checksum = apaCheckSum(header, 1);
        clink->flags |= APA_CACHE_FLAG_DIRTY;
        apaCacheFlushDirty(clink);
        blkIoFlushCache(f->unit);
        apaCacheFree(clink);
        hddDevices[f->unit].status = 0;
        hddDevices[f->unit].format = APA_MBR_VERSION;
    }
    // TODO: DVRP firmware 48-bit creates __extend partition at offset_24_bit, with same params as __mbr except content is empty
#ifdef APA_FORMAT_MAKE_PARTITIONS
    memset(&emptyBlocks, 0, sizeof(emptyBlocks));
    memset(&params, 0, sizeof(apa_params_t));
    params.size = (1024 * 256);
    params.type = APA_TYPE_PFS;

    // TODO: For DVRP firmware 48-bit, __xdata is created with size 1024 * 2048
    // TODO: For DVRP firmware 48-bit, __xcontents is created with size (offset_48_bit - offset_24_bit) - 0x240000 & 0xfffff7ff
    // add __net, __system....
    for (i = 0; formatPartList[i]; i++) {
        memset(params.id, 0, APA_IDMAX);
        strcpy(params.id, formatPartList[i]);
        if (!(clink = hddAddPartitionHere(f->unit, &params, emptyBlocks, i ? clink->sector : 0, &rv)))
            return rv;
        apaCacheFree(clink);

        params.size <<= 1;
        if (hddDevices[f->unit].partitionMaxSize < params.size)
            params.size = hddDevices[f->unit].partitionMaxSize;
    }
#endif
    return rv;
}

static int apaOpen(s32 device, hdd_file_slot_t *fileSlot, apa_params_t *params, int mode)
{
    int rv = 0;
    u32 emptyBlocks[32];
    apa_cache_t *clink;
    apa_cache_t *clink2;
    u32 sector = 0;

#ifdef APA_SUPPORT_BHDD
    // TODO: start sector usually is at either 0x4A817C8 (40GB offset) or 0x400000 (2GiB offset)
    if (strcmp(params->id, "__xcontents") == 0 || strcmp(params->id, "__extend") == 0 || strcmp(params->id, "__xdata") == 0)
        sector = hddDevices[device].totalLBA;
#endif

    // walk all looking for any empty blocks & look for partition
    clink = apaCacheGetHeader(device, sector, APA_IO_MODE_READ, &rv);
    memset(&emptyBlocks, 0, sizeof(emptyBlocks));
    while (clink) {
        sector = clink->sector;
        if (!(clink->header->flags & APA_FLAG_SUB)) {
            if (memcmp(clink->header->id, params->id, APA_IDMAX) == 0)
                break; // found :)
        }
        apaAddEmptyBlock(clink->header, emptyBlocks);
        clink = apaGetNextHeader(clink, &rv);
    }

    if (rv != 0)
        return rv;
    rv = -ENOENT;

    if (clink == NULL && (mode & FIO_O_CREAT)) {
        if ((rv = hddCheckPartitionMax(device, params->size)) >= 0) {
            if ((clink = hddAddPartitionHere(device, params, emptyBlocks, sector, &rv)) != NULL) {
                sector = clink->header->start;
                clink2 = apaCacheAlloc();
                memset(clink2->header, 0, sizeof(apa_header_t));
                blkIoDmaTransfer(device, clink2->header, sector + 8, 2, BLKIO_DIR_WRITE);
                blkIoDmaTransfer(device, clink2->header, sector + 0x2000, 2, BLKIO_DIR_WRITE);
                apaCacheFree(clink2);
            }
        }
    }
    if (clink == NULL)
        return rv;
    fileSlot->parts[0].start = clink->header->start;
    fileSlot->parts[0].length = clink->header->length;
    memcpy(&fileSlot->parts[1], &clink->header->subs, APA_MAXSUB * sizeof(apa_sub_t));
    fileSlot->type = clink->header->type;
    fileSlot->nsub = clink->header->nsub;
    memcpy(&fileSlot->id, &clink->header->id, APA_IDMAX);
    apaCacheFree(clink);
    if (apaPassCmp(clink->header->fpwd, params->fpwd) != 0) {
        rv = (!(mode & FIO_O_WRONLY)) ? apaPassCmp(clink->header->rpwd, params->rpwd) : -EACCES;
    } else
        rv = 0;

    return rv;
}

static int apaRemove(s32 device, const char *id, const char *fpwd)
{
    u32 nsub;
    apa_cache_t *clink;
    int rv, i;

    for (i = 0; i < apaMaxOpen; i++) // look to see if open
    {
        if (hddFileSlots[i].f != 0) {
            if (memcmp(hddFileSlots[i].id, id, APA_IDMAX) == 0)
                return -EBUSY;
        }
    }
#ifndef APA_ALLOW_REMOVE_PARTITION_WITH_LEADING_UNDERSCORE
    if (id[0] == '_' && id[1] == '_')
        return -EACCES;
#endif
    if ((clink = apaFindPartition(device, id, &rv)) == NULL)
        return rv;
    if (apaPassCmp(clink->header->fpwd, fpwd)) {
        apaCacheFree(clink);
        return -EACCES;
    }
    // remove all subs first...
    nsub = clink->header->nsub;
    clink->header->nsub = 0;
    clink->flags |= APA_CACHE_FLAG_DIRTY;
    apaCacheFlushAllDirty(device);
    for (i = nsub - 1; i != -1; i--) {
        apa_cache_t *clink2;

        if ((clink2 = apaCacheGetHeader(device, clink->header->subs[i].start, APA_IO_MODE_READ, &rv))) {
            if ((rv = apaDelete(clink2))) {
                apaCacheFree(clink);
                return rv;
            }
        }
    }
    if (rv == 0)
        return apaDelete(clink);

    apaCacheFree(clink);
    return rv;
}

// Unofficial helper for renaming APA partitions.
static int apaRename(s32 device, const apa_params_t *oldParams, const apa_params_t *newParams)
{
    apa_cache_t *clink;
    int i, rv;

    // look to see if can make(newname) or not...
    if ((clink = apaFindPartition(device, newParams->id, &rv)) != NULL) {
        apaCacheFree(clink);
        SignalSema(fioSema);
        return -EEXIST; // File exists
    }

    // look to see if open(oldname)
    for (i = 0; i < apaMaxOpen; i++) {
        if (hddFileSlots[i].f != NULL) {
            if (memcmp(hddFileSlots[i].id, oldParams->id, APA_IDMAX) == 0) {
                SignalSema(fioSema);
                return -EBUSY;
            }
        }
    }

    // Do not allow system partitions (__*) to be renamed.
    if (oldParams->id[0] == '_' && oldParams->id[1] == '_')
        return -EACCES;

    // find :)
    if ((clink = apaFindPartition(device, oldParams->id, &rv)) == NULL) {
        SignalSema(fioSema);
        return rv;
    }

    // Check for access rights.
    if (apaPassCmp(clink->header->fpwd, oldParams->fpwd) != 0) {
        apaCacheFree(clink);
        return -EACCES;
    }

    // do the renaming :) note: subs have no names!!
    memcpy(clink->header->id, newParams->id, APA_IDMAX);

    // Update passwords
    memcpy(clink->header->rpwd, newParams->rpwd, APA_PASSMAX);
    memcpy(clink->header->fpwd, newParams->fpwd, APA_PASSMAX);

    clink->flags |= APA_CACHE_FLAG_DIRTY;

    apaCacheFlushAllDirty(device);
    apaCacheFree(clink);

    return 0;
}

int hddRemove(iomanX_iop_file_t *f, const char *name)
{
    int rv;
    apa_params_t params;

    if ((rv = fioGetInput(name, &params)) < 0)
        return rv;
#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        return -EACCES;
#endif
    WaitSema(fioSema);
    rv = apaRemove(f->unit, params.id, params.fpwd);
    SignalSema(fioSema);

    return rv;
}

int hddOpen(iomanX_iop_file_t *f, const char *name, int flags, int mode)
{
    int rv;
    apa_params_t params;
    hdd_file_slot_t *fileSlot;

    (void)mode;

    if (f->unit >= BLKIO_MAX_VOLUMES || hddDevices[f->unit].status != 0)
        return -ENODEV;

#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        if ((flags & FIO_O_CREAT) != 0)
            return -EACCES;
#endif

    if (!(f->mode & FIO_O_DIROPEN))
        if ((rv = fioGetInput(name, &params)) < 0)
            return rv;

    WaitSema(fioSema);
    if ((rv = getFileSlot(&params, &fileSlot)) == 0) {
        if (!(f->mode & FIO_O_DIROPEN)) {
            if ((rv = apaOpen(f->unit, fileSlot, &params, flags)) == 0) {
                fileSlot->f = f;
                f->privdata = fileSlot;
            }
        } else {
#ifdef APA_SUPPORT_BHDD
            if (strcmp(f->device->name, "bhdd") == 0) {
                fileSlot->parts[0].start = hddDevices[f->unit].totalLBA;
            } else {
                fileSlot->parts[0].start = 0;
            }
#endif
            fileSlot->f = f;
            f->privdata = fileSlot;
        }
    }
    SignalSema(fioSema);
    return rv;
}

int hddClose(iomanX_iop_file_t *f)
{
    WaitSema(fioSema);
    memset(f->privdata, 0, sizeof(hdd_file_slot_t));
    SignalSema(fioSema);
    return 0;
}

int hddRead(iomanX_iop_file_t *f, void *buf, int size)
{
    return fioDataTransfer(f, buf, size, BLKIO_DIR_READ);
}

int hddWrite(iomanX_iop_file_t *f, void *buf, int size)
{
    if (!(f->mode & FIO_O_WRONLY))
        return -EACCES;
#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        return -EACCES;
#endif
    return fioDataTransfer(f, buf, size, BLKIO_DIR_WRITE);
}

int hddLseek(iomanX_iop_file_t *f, int post, int whence)
{
    int rv = 0;
    hdd_file_slot_t *fileSlot;

    // test input( no seeking to end point less :P )
    if (whence == FIO_SEEK_END)
        return -EINVAL;
    if ((post & 0x1FF))
        return -EINVAL;

    post >>= 9; // post/512

    WaitSema(fioSema);
    fileSlot = f->privdata;
    if (whence == FIO_SEEK_CUR) {
        if (((int)fileSlot->post + post) < 0 || (fileSlot->post + post) >= 0x1FF9)
            rv = -EINVAL;
        else {
            fileSlot->post += post;
            rv = fileSlot->post << 9;
        }
    } else if (whence == FIO_SEEK_SET) {
        if (post < 0 || post >= 0x1FF9)
            rv = -EINVAL;
        else {
            fileSlot->post = post;
            rv = fileSlot->post << 9;
        }
    }
    SignalSema(fioSema);
    return rv;
}

static void fioGetStatFiller(apa_cache_t *clink, iox_stat_t *stat)
{
    stat->mode = clink->header->type;
    stat->attr = clink->header->flags;
    stat->hisize = 0;
    stat->size = clink->header->length;
    memcpy(&stat->ctime, &clink->header->created, sizeof(apa_ps2time_t));
    memcpy(&stat->atime, &clink->header->created, sizeof(apa_ps2time_t));
    memcpy(&stat->mtime, &clink->header->created, sizeof(apa_ps2time_t));
    if (clink->header->flags & APA_FLAG_SUB)
        stat->private_0 = clink->header->number;
    else
        stat->private_0 = clink->header->nsub;
    stat->private_1 = 0;
    stat->private_2 = 0;
    stat->private_3 = 0;
    stat->private_4 = 0;
#ifndef APA_STAT_RETURN_PART_LBA
    stat->private_5 = 0; // game ver
#else
    stat->private_5 = clink->header->start; // SONY ver (return start LBA of the partition)
#endif
}

int hddGetStat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat)
{
    apa_cache_t *clink;
    apa_params_t params;
    int rv;

    if ((rv = fioGetInput(name, &params)) < 0)
        return rv;

    WaitSema(fioSema);
    if ((clink = apaFindPartition(f->unit, params.id, &rv))) {
        if ((rv = apaPassCmp(clink->header->fpwd, params.fpwd)) == 0 || (rv = apaPassCmp(clink->header->rpwd, params.rpwd)) == 0)
            fioGetStatFiller(clink, stat);
        apaCacheFree(clink);
    }
    SignalSema(fioSema);
    return rv;
}

int hddDopen(iomanX_iop_file_t *f, const char *name)
{
    return hddOpen(f, name, 0, 0);
}

int hddDread(iomanX_iop_file_t *f, iox_dirent_t *dirent)
{
    int rv;
    hdd_file_slot_t *fileSlot = f->privdata;
    apa_cache_t *clink;

    if (!(f->mode & FIO_O_DIROPEN))
        return -ENOTDIR;

    if (fileSlot->parts[0].start == (u32)(-1))
        return 0; // end :)

    WaitSema(fioSema);
    if ((clink = apaCacheGetHeader(f->unit, fileSlot->parts[0].start, APA_IO_MODE_READ, &rv)) &&
        clink->header->length) {
        if (clink->header->flags & APA_FLAG_SUB) {
            // if sub get id from main header...
            apa_cache_t *cmain = apaCacheGetHeader(f->unit, clink->header->main, APA_IO_MODE_READ, &rv);
            if (cmain != NULL) {
                /*  This was the SONY original, which didn't do bounds-checking:
                    rv=strlen(cmain->header->id);
                    strcpy(dirent->name, cmain->header->id); */
                strncpy(dirent->name, cmain->header->id, APA_IDMAX);
                dirent->name[APA_IDMAX] = '\0';
                rv = strlen(dirent->name);

                apaCacheFree(cmain);
            }
        } else {
            /*  This was the SONY original, which didn't do bounds-checking:
                rv=strlen(clink->header->id);
                strcpy(dirent->name, clink->header->id); */
            strncpy(dirent->name, clink->header->id, APA_IDMAX);
            dirent->name[APA_IDMAX] = '\0';
            rv = strlen(dirent->name);
        }
        fioGetStatFiller(clink, &dirent->stat);
        if (clink->header->next == 0)
            fileSlot->parts[0].start = -1; // mark end
        else
            fileSlot->parts[0].start = clink->header->next; // set next
        apaCacheFree(clink);
    }
    SignalSema(fioSema);
    return rv;
}

/*  Originally, SONY provided no function for renaming partitions.
    Syntax: rename <Old ID>,<fpwd> <New ID>,<fpwd>

    The full-access password (fpwd) is required.
    System partitions (__*) cannot be renamed. */
int hddReName(iomanX_iop_file_t *f, const char *oldname, const char *newname)
{
    apa_params_t oldParams;
    apa_params_t newParams;
    int rv;

    if ((rv = fioGetInput(oldname, &oldParams)) < 0)
        return rv;
    if ((rv = fioGetInput(newname, &newParams)) < 0)
        return rv;

    WaitSema(fioSema);
    rv = apaRename(f->unit, &oldParams, &newParams);
    SignalSema(fioSema);

    return rv;
}

static int ioctl2AddSub(hdd_file_slot_t *fileSlot, char *argp)
{
    int rv;
    u32 device = fileSlot->f->unit;
    apa_params_t params;
    u32 emptyBlocks[32];
    apa_cache_t *clink;
    u32 sector = 0;
    u32 length;

    if (!(fileSlot->f->mode & FIO_O_WRONLY))
        return -EACCES;

    if (!(fileSlot->nsub < APA_MAXSUB))
        return -EFBIG;

    memset(&params, 0, sizeof(apa_params_t));

    if ((rv = fioPartitionSizeLookUp(argp)) < 0)
        return rv;

    params.size = rv;
    params.flags = APA_FLAG_SUB;
    params.type = fileSlot->type;
    params.main = fileSlot->parts[0].start;
    params.number = fileSlot->nsub + 1;
    if ((rv = hddCheckPartitionMax(device, params.size)) < 0)
        return rv;

    // walk all looking for any empty blocks
    memset(&emptyBlocks, 0, sizeof(emptyBlocks));
    clink = apaCacheGetHeader(device, APA_SECTOR_MBR, APA_IO_MODE_READ, &rv);
    while (clink) {
        sector = clink->sector;
        apaAddEmptyBlock(clink->header, emptyBlocks);
        clink = apaGetNextHeader(clink, &rv);
    }
    if (rv != 0)
        return rv;

    if (!(clink = hddAddPartitionHere(device, &params, emptyBlocks, sector, &rv)))
        return rv;

    sector = clink->header->start;
    length = clink->header->length;
    apaCacheFree(clink);
    if (!(clink = apaCacheGetHeader(device, fileSlot->parts[0].start, APA_IO_MODE_READ, &rv)))
        return rv;

    clink->header->subs[clink->header->nsub].start = sector;
    clink->header->subs[clink->header->nsub].length = length;
    clink->header->nsub++;
    fileSlot->nsub++;
    fileSlot->parts[fileSlot->nsub].start = sector;
    fileSlot->parts[fileSlot->nsub].length = length;
    clink->flags |= APA_CACHE_FLAG_DIRTY;
    apaCacheFlushAllDirty(device);
    apaCacheFree(clink);
    return rv;
}

static int ioctl2DeleteLastSub(hdd_file_slot_t *fileSlot)
{
    int rv;
    u32 device = fileSlot->f->unit;
    apa_cache_t *mainPart;
    apa_cache_t *subPart;

    if (!(fileSlot->f->mode & FIO_O_WRONLY))
        return -EACCES;

    if (fileSlot->nsub == 0)
        return -ENOENT;

    if (!(mainPart = apaCacheGetHeader(device, fileSlot->parts[0].start, APA_IO_MODE_READ, &rv)))
        return rv;

    if ((subPart = apaCacheGetHeader(device,
                                     mainPart->header->subs[mainPart->header->nsub - 1].start, APA_IO_MODE_READ, &rv))) {
        fileSlot->nsub--;
        mainPart->header->nsub--;
        mainPart->flags |= APA_CACHE_FLAG_DIRTY;
        apaCacheFlushAllDirty(device);
        rv = apaDelete(subPart);
    }
    apaCacheFree(mainPart);
    return rv;
}

int hddIoctl2(iomanX_iop_file_t *f, int req, void *argp, unsigned int arglen,
              void *bufp, unsigned int buflen)
{
    u32 rv = 0, err_lba;
    hdd_file_slot_t *fileSlot = f->privdata;

	(void)arglen;
	(void)bufp;
	(void)buflen;

#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0) {
        switch (req) {
            case HIOCADDSUB:
            case HIOCDELSUB:
            case HIOCFLUSH:
            case HIOCSETPARTERROR:
            case HIOCGETPARTERROR:
                return -EACCES;
            default:
                break;
        }
    }
#endif
    WaitSema(fioSema);
    switch (req) {
        // cmd set 1
        case HIOCADDSUB:
            rv = ioctl2AddSub(fileSlot, (char *)argp);
            break;

        case HIOCDELSUB:
            rv = ioctl2DeleteLastSub(fileSlot);
            break;

        case HIOCNSUB:
            rv = fileSlot->nsub;
            break;

        case HIOCFLUSH:
            blkIoFlushCache(f->unit);
            break;

        // cmd set 2
        case HIOCTRANSFER:
            rv = ioctl2Transfer(f->unit, fileSlot, argp);
            break;

        case HIOCGETSIZE:
            rv = fileSlot->parts[*(u32 *)argp].length;
            break;

        case HIOCSETPARTERROR:
            apaSetPartErrorSector(f->unit, fileSlot->parts[0].start);
            rv = 0;
            break;

        case HIOCGETPARTERROR:
            if ((rv = apaGetPartErrorSector(f->unit, APA_SECTOR_PART_ERROR, &err_lba)) > 0) {
                if (err_lba == fileSlot->parts[0].start) {
                    rv = 0;
                    apaSetPartErrorSector(f->unit, 0); // clear last error :)
                }
            }
            break;

#ifdef APA_SUPPORT_IOCTL_GETPARTSTART
        // Special HDD.IRX IOCTL2 command for supporting HDLFS
        case HIOCGETPARTSTART:
            rv = fileSlot->parts[*(u32 *)argp].start;
            break;
#endif

        default:
            rv = -EINVAL;
            break;
    }
    SignalSema(fioSema);
    return rv;
}

static int devctlSwapTemp(s32 device, char *argp)
{
    int rv;
    apa_params_t params;
    char szBuf[APA_IDMAX];
    apa_cache_t *partTemp;
    apa_cache_t *partNew;


    if ((rv = fioGetInput(argp, &params)) < 0)
        return rv;

    if (params.id[0] == '_' && params.id[1] == '_') // test for '__' system partition
        return -EINVAL;

    memset(szBuf, 0, APA_IDMAX);
    strcpy(szBuf, "_tmp");
    if (!(partTemp = apaFindPartition(device, szBuf, &rv)))
        return rv;

    if ((partNew = apaFindPartition(device, params.id, &rv))) {
        if ((rv = apaPassCmp(partNew->header->fpwd, params.fpwd)) == 0) {
            memcpy(partTemp->header->id, partNew->header->id, APA_IDMAX);
            memcpy(partTemp->header->rpwd, partNew->header->rpwd, APA_PASSMAX);
            memcpy(partTemp->header->fpwd, partNew->header->fpwd, APA_PASSMAX);
            // memset(partNew->header->id, 0, 8);// BUG! can make it so can not open!!
            memset(partNew->header->id, 0, APA_IDMAX);
            strcpy(partNew->header->id, "_tmp");
            memset(partNew->header->rpwd, 0, APA_PASSMAX);
            memset(partNew->header->fpwd, 0, APA_PASSMAX);
            partTemp->flags |= APA_CACHE_FLAG_DIRTY;
            partNew->flags |= APA_CACHE_FLAG_DIRTY;
            apaCacheFlushAllDirty(device);
        }
        apaCacheFree(partNew);
    }
    apaCacheFree(partTemp);
    return rv;
}

static int devctlSetOsdMBR(s32 device, hddSetOsdMBR_t *mbrInfo)
{
    int rv;
    apa_cache_t *clink;

    if (!(clink = apaCacheGetHeader(device, APA_SECTOR_MBR, APA_IO_MODE_READ, &rv)))
        return rv;

    APA_PRINTF(APA_DRV_NAME ": mbr start: %ld\n" APA_DRV_NAME ": mbr size : %ld\n", mbrInfo->start, mbrInfo->size);
#ifdef APA_SUPPORT_GPT
    // osdStart should not overwrite APA journal
    if (mbrInfo->start < APA_SECTOR_MIN_OSDSTART)
        return -EINVAL;
#endif
    clink->header->mbr.osdStart = mbrInfo->start;
    clink->header->mbr.osdSize = mbrInfo->size;
    clink->flags |= APA_CACHE_FLAG_DIRTY;
    apaCacheFlushAllDirty(device);
    apaCacheFree(clink);
    return rv;
}

int hddDevctl(iomanX_iop_file_t *f, const char *devname, int cmd, void *arg,
              unsigned int arglen, void *bufp, unsigned int buflen)
{
    int rv = 0;

	(void)devname;
	(void)arglen;
	(void)buflen;

#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        return -ENODEV;
#endif

    WaitSema(fioSema);
    switch (cmd) {
        // Command set 1 ('H')
        case HDIOC_DEV9OFF:
            // Early versions called sceAtaSmartSaveAttr() here, when their old dev9 versions did not support the pre-shutdown callback.
#ifdef APA_USE_DEV9
            dev9Shutdown();
#else
            blkIoSmartSaveAttr(f->unit);
#endif
            break;

        case HDIOC_IDLE:
            rv = blkIoIdle(f->unit, *(char *)arg);
            break;

        case HDIOC_MAXSECTOR:
            rv = hddDevices[f->unit].partitionMaxSize;
            break;

        case HDIOC_TOTALSECTOR:
            rv = hddDevices[f->unit].totalLBA;
            break;

        case HDIOC_FLUSH:
            if (blkIoFlushCache(f->unit))
                rv = -EIO;
            break;

        case HDIOC_SWAPTMP:
            rv = devctlSwapTemp(f->unit, (char *)arg);
            break;

        case HDIOC_SMARTSTAT:
            rv = blkIoSmartReturnStatus(f->unit);
            break;

        case HDIOC_STATUS:
            rv = hddDevices[f->unit].status;
            break;

        case HDIOC_FORMATVER:
            rv = hddDevices[f->unit].format;
            break;

        case HDIOC_FREESECTOR:
            rv = apaGetFreeSectors(f->unit, bufp, hddDevices);
            break;

        case HDIOC_IDLEIMM:
            rv = blkIoIdleImmediate(f->unit);
            break;

        // Command set 2 ('h')
        case HDIOC_GETTIME:
            rv = apaGetTime((apa_ps2time_t *)bufp);
            break;

        case HDIOC_SETOSDMBR:
            rv = devctlSetOsdMBR(f->unit, (hddSetOsdMBR_t *)arg);
            break;

        case HDIOC_GETSECTORERROR:
            rv = apaGetPartErrorSector(f->unit, APA_SECTOR_SECTOR_ERROR, 0);
            break;

        case HDIOC_GETERRORPARTNAME:
            rv = apaGetPartErrorName(f->unit, (char *)bufp);
            break;

        case HDIOC_READSECTOR:
            rv = blkIoDmaTransfer(f->unit, (void *)bufp, ((hddAtaTransfer_t *)arg)->lba,
                                      ((hddAtaTransfer_t *)arg)->size, BLKIO_DIR_READ);
            break;

        case HDIOC_WRITESECTOR:
            rv = blkIoDmaTransfer(f->unit, ((hddAtaTransfer_t *)arg)->data,
                                      ((hddAtaTransfer_t *)arg)->lba, ((hddAtaTransfer_t *)arg)->size,
                                      BLKIO_DIR_WRITE);
            break;

        case HDIOC_SCEIDENTIFY:
            rv = blkIoGetSceId(f->unit, (u16 *)bufp);
            break;

        // HDIOC_INSTSEC is not implemented in DVRP firmware
        // HDIOC_SETMAXLBA28 is implemented in DVRP firmware
        // HDIOC_GETMAXLBA48 is implemented in DVRP firmware
        // HDIOC_ISLBA48 is implemented in DVRP firmware
        // HDIOC_PRESETMAXLBA28 is not implemented in DVRP firmware
        // HDIOC_POSTSETMAXLBA28 is not implemented in DVRP firmware
        // HDIOC_ENABLEWRITECACHE is implemented in DVRP firmware -> ATA 0xEF subcommand 0x02
        // HDIOC_DISABLEWRITECACHE is implemented in DVRP firmware -> ATA 0xEF subcommand 0x82

        default:
            rv = -EINVAL;
            break;
    }
    SignalSema(fioSema);

    return rv;
}

#ifdef APA_USE_IOMANX
int hddMount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
    int rv = 0;

    (void)flag;
    (void)arg;
    (void)arglen;

#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        return -ENODEV;
#endif

    WaitSema(fioSema);
    rv = hdd_blkio_vhdd_mount(f->unit, devname);
    SignalSema(fioSema);

    return rv;
}

int hddUmount(iomanX_iop_file_t *f, const char *fsname)
{
    int rv = 0;

    (void)fsname;

#ifdef APA_SUPPORT_BHDD
    if (strcmp(f->device->name, "bhdd") == 0)
        return -ENODEV;
#endif

    WaitSema(fioSema);
    rv = hdd_blkio_vhdd_umount(f->unit);
    SignalSema(fioSema);

    return rv;
}
#endif

int hddUnsupported(iomanX_iop_file_t *f)
{
	(void)f;

	return -1;
}
