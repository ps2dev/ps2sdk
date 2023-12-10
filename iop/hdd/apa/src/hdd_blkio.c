/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <iomanX.h>

#include <libapa.h>
#include "hdd.h"
#include "hdd_blkio.h"

#ifdef APA_USE_BDM

#ifdef _IOP
#include <sysclib.h>
#else
#include <string.h>
#endif
#include <thbase.h>
#include <thsemap.h>

#include <bdm.h>

typedef struct hdd_blkio_fs_driver_mount_info_
{
    struct block_device *mounted_bd;
} hdd_blkio_fs_driver_mount_info;

extern apa_device_t hddDevices[]; // defined in hdd.c
static hdd_blkio_fs_driver_mount_info fs_driver_mount_info[BLKIO_MAX_VOLUMES];

#define HDD_BLKIO_FS_DRIVER_MOUNT_INFO_MAX ((int)(sizeof(fs_driver_mount_info) / sizeof(fs_driver_mount_info[0])))

static int _fs_lock_sema_id = -1;

static int _fs_init_lock(void)
{
    iop_sema_t sp;

    sp.initial = 1;
    sp.max     = 1;
    sp.option  = 0;
    sp.attr    = 0;
    if ((_fs_lock_sema_id = CreateSema(&sp)) < 0) {
        return (-1);
    }

    return (0);
}

static void _fs_lock(void)
{
    WaitSema(_fs_lock_sema_id);
}

static void _fs_unlock(void)
{
    SignalSema(_fs_lock_sema_id);
}

static void fs_reset(void)
{
    if (_fs_lock_sema_id >= 0)
        DeleteSema(_fs_lock_sema_id);

    _fs_init_lock();
}

static void hdd_blkio_fs_driver_initialize_all_mount_info(void)
{
    int i;
    for (i = 0; i < HDD_BLKIO_FS_DRIVER_MOUNT_INFO_MAX; i += 1) {
        fs_driver_mount_info[i].mounted_bd = NULL;
    }
}

static int hdd_blkio_fs_driver_find_mount_info_index_from_block_device(const struct block_device *bd)
{
    int i;
    for (i = 0; i < HDD_BLKIO_FS_DRIVER_MOUNT_INFO_MAX; i += 1) {
        if (fs_driver_mount_info[i].mounted_bd == bd) {
            return i;
        }
    }
    return -1;
}

static int hdd_blkio_fs_driver_find_mount_info_index_free(void)
{
    return hdd_blkio_fs_driver_find_mount_info_index_from_block_device(NULL);
}

static struct block_device *hdd_blkio_fs_driver_get_mounted_bd_from_index(int mount_info_index)
{
    struct block_device *mounted_bd;
    if (mount_info_index >= HDD_BLKIO_FS_DRIVER_MOUNT_INFO_MAX) {
        return NULL;
    }
    mounted_bd = fs_driver_mount_info[mount_info_index].mounted_bd;
    return mounted_bd;
}

static int hdd_blkio_fs_driver_mount_bd(int mount_info_index, struct block_device *bd)
{
    fs_driver_mount_info[mount_info_index].mounted_bd = bd;
    hddDevices[mount_info_index].status = 3;
    // 512 sector size only supported
    if (bd->sectorSize != 512)
    {
        fs_driver_mount_info[mount_info_index].mounted_bd = NULL;
        return -1;
    }
    hddDevices[mount_info_index].status -= 1; // assume drive exists
    hddDevices[mount_info_index].totalLBA = bd->sectorCount;
    hddDevices[mount_info_index].partitionMaxSize = apaGetPartitionMax(hddDevices[mount_info_index].totalLBA);
    hddDevices[mount_info_index].status -= 1; // assume drive unlocked
    if (apaJournalRestore(mount_info_index) != 0)
    {
        fs_driver_mount_info[mount_info_index].mounted_bd = NULL;
        return -1;
    }
    // Check if drive is formatted in APA
    if (apaGetFormat(mount_info_index, &hddDevices[mount_info_index].format) != 0)
    {
        hddDevices[mount_info_index].status -= 1;
    }
    if (hddDevices[mount_info_index].status != 0)
    {
        fs_driver_mount_info[mount_info_index].mounted_bd = NULL;
        return -1;
    }
    return 0;
}

static void hdd_blkio_fs_driver_unmount_bd(int mount_info_index)
{
    fs_driver_mount_info[mount_info_index].mounted_bd = NULL;
    hddDevices[mount_info_index].status = 3;
}

static int hdd_blkio_connect_bd(struct block_device *bd)
{
    int mount_info_index;

    _fs_lock();
    mount_info_index = hdd_blkio_fs_driver_find_mount_info_index_free();
    if (mount_info_index != -1) {
        if (hdd_blkio_fs_driver_mount_bd(mount_info_index, bd) == 0) {
            _fs_unlock();
            return 0;
        }
    }
    _fs_unlock();
    return -1;
}

static void hdd_blkio_disconnect_bd(struct block_device *bd)
{
    int mount_info_index;

    _fs_lock();
    mount_info_index = hdd_blkio_fs_driver_find_mount_info_index_from_block_device(bd);
    if (mount_info_index != -1) {
        hdd_blkio_fs_driver_unmount_bd(mount_info_index);
    }
    _fs_unlock();
}

static struct file_system g_fs = {
    .priv = NULL,
    .name = "ps2hdd",
    .connect_bd = hdd_blkio_connect_bd,
    .disconnect_bd = hdd_blkio_disconnect_bd,
};
#endif

#ifdef APA_USE_IOMANX

#ifdef _IOP
#include <sysclib.h>
#else
#include <string.h>
#endif
#include <errno.h>

typedef struct vhdd_diskinfo_
{
    int fd;
    int sectorcount;
} vhdd_diskinfo_t;

extern apa_device_t hddDevices[]; // defined in hdd.c
static vhdd_diskinfo_t hdd_blkio_vhdd_diskinfo[BLKIO_MAX_VOLUMES];

static void hdd_blkio_vhdd_clear_slot(int slot)
{
    vhdd_diskinfo_t *diskinfo;
    diskinfo = &hdd_blkio_vhdd_diskinfo[slot];
    if (diskinfo->fd >= 0) {
        iomanX_close(diskinfo->fd);
    }
    memset(diskinfo, 0, sizeof(vhdd_diskinfo_t));
    diskinfo->fd = -1;
}

static int hdd_blkio_vhdd_validate(int slot, int check_mounted)
{
    vhdd_diskinfo_t *diskinfo;

    if (slot > BLKIO_MAX_VOLUMES) {
        return -EOVERFLOW;
    }

    diskinfo = &hdd_blkio_vhdd_diskinfo[slot];

    if (check_mounted) {
        if (diskinfo->fd < 0) {
            return -ENXIO;
        }
    }
    return 0;
}

int hdd_blkio_vhdd_mount(int slot, const char *filename)
{
    int fd;
    s64 disksize;
    int r;
    vhdd_diskinfo_t *diskinfo;

    r = hdd_blkio_vhdd_validate(slot, 0);
    if (r < 0) {
        goto cleanup;
    }

    diskinfo = &hdd_blkio_vhdd_diskinfo[slot];

    fd = iomanX_open(filename, FIO_O_RDWR, 0);
    if (fd < 0) {
        // Propogate open error
        r = fd;
        goto cleanup;
    }
    diskinfo->fd = fd;

    disksize = iomanX_lseek64(fd, 0, FIO_SEEK_END);
    diskinfo->sectorcount = disksize >> 9; // divide by 512
    iomanX_lseek64(fd, 0, FIO_SEEK_SET);

    // Check if image is formatted
    {
        hddDevices[slot].status = 3;
        hddDevices[slot].status -= 1; // assume drive exists
        hddDevices[slot].totalLBA = diskinfo->sectorcount;
        hddDevices[slot].partitionMaxSize = apaGetPartitionMax(hddDevices[slot].totalLBA);
        hddDevices[slot].status -= 1; // assume drive unlocked
        if (apaJournalRestore(slot) != 0)
        {
            r = -EIO;
            goto cleanup;
        }
        // Check if drive is formatted in APA
        if (apaGetFormat(slot, &hddDevices[slot].format) != 0)
        {
            hddDevices[slot].status -= 1;
        }
        if (hddDevices[slot].status != 0)
        {
            r = -EIO;
            goto cleanup;
        }
        r = 0; // Success
    }
cleanup:
    if (r < 0) {
        hdd_blkio_vhdd_clear_slot(slot);
    }
    return r;
}

int hdd_blkio_vhdd_umount(int slot)
{
    int r;

    r = hdd_blkio_vhdd_validate(slot, 1);
    if (r < 0) {
        goto cleanup;
    }

    hdd_blkio_vhdd_clear_slot(slot);

cleanup:
    return r;
}
#endif

#ifndef APA_USE_ATAD
int blkIoInit(void)
{
#ifdef APA_USE_BDM
    fs_reset();
    hdd_blkio_fs_driver_initialize_all_mount_info();
    bdm_connect_fs(&g_fs);
#endif
    return 0;
}

int blkIoDmaTransfer(int device, void *buf, u32 lba, u32 nsectors, int dir)
{
#ifdef APA_USE_BDM
    struct block_device *mounted_bd;

    mounted_bd = hdd_blkio_fs_driver_get_mounted_bd_from_index(device);

    if (mounted_bd == NULL)
    {
        return -1;
    }

    if (dir == BLKIO_DIR_READ)
    {
        return (mounted_bd->read(mounted_bd, lba, buf, nsectors) == nsectors) ? 0 : -1;
    }
    else if (dir == BLKIO_DIR_WRITE)
    {
        return (mounted_bd->write(mounted_bd, lba, buf, nsectors) == nsectors) ? 0 : -1;
    }
#elif defined(APA_USE_IOMANX)
    {
        int r;
        vhdd_diskinfo_t *diskinfo;

        r = hdd_blkio_vhdd_validate(device, 1);
        if (r < 0) {
            return -1;
        }
        diskinfo = &hdd_blkio_vhdd_diskinfo[device];

        if (buf != NULL) {
            int sector_to_byte_count;
            sector_to_byte_count = nsectors * 512;
            iomanX_lseek64(diskinfo->fd, (s64)lba * (s64)512, FIO_SEEK_SET);
            if (dir == BLKIO_DIR_READ)
            {
                return (iomanX_read(diskinfo->fd, buf, sector_to_byte_count) == sector_to_byte_count) ? 0 : -1;
            }
            else if (dir == BLKIO_DIR_WRITE)
            {
                return (iomanX_write(diskinfo->fd, buf, sector_to_byte_count) == sector_to_byte_count) ? 0 : -1;
            }
        }
    }
#else
    (void)device;
    (void)buf;
    (void)lba;
    (void)nsectors;
    (void)dir;
#endif
    return -1;
}

int blkIoIdle(int device, int period)
{
    (void)device;
    (void)period;
    return 0;
}

int blkIoGetSceId(int device, void *data)
{
    (void)device;
    (void)data;
    return 0;
}

int blkIoSmartReturnStatus(int device)
{
    (void)device;
    return 0;
}

int blkIoSmartSaveAttr(int device)
{
#ifdef APA_USE_BDM
    struct block_device *mounted_bd;

    mounted_bd = hdd_blkio_fs_driver_get_mounted_bd_from_index(device);

    if (mounted_bd == NULL)
    {
        return 0;
    }

    mounted_bd->stop(mounted_bd);
#else
    (void)device;
#endif
    return 0;
}

int blkIoFlushCache(int device)
{
#ifdef APA_USE_BDM
    struct block_device *mounted_bd;

    mounted_bd = hdd_blkio_fs_driver_get_mounted_bd_from_index(device);

    if (mounted_bd == NULL)
    {
        return 0;
    }

    mounted_bd->flush(mounted_bd);
#else
    (void)device;
#endif
    return 0;
}

int blkIoIdleImmediate(int device)
{
    (void)device;
    return 0;
}
#endif
