/*
 * fat_driver.c - USB Mass storage driver for PS2
 *
 * (C) 2004, Marek Olejnik (ole00@post.cz)
 * (C) 2004  Hermes (support for sector sizes from 512 to 4096 bytes)
 * (C) 2004  raipsu (fs_dopen, fs_dclose, fs_dread, fs_getstat implementation)
 *
 * FAT filesystem layer
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */
//---------------------------------------------------------------------------
#include <errno.h>
#include <iomanX.h>
#include <stdio.h>

#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thsemap.h>

#include <bdm.h>

#include <usbhdfsd-common.h>

#include "fs_driver.h"

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

FATFS fatfs;
struct block_device *mounted_bd;

// TODO: if all drives have the same mount point, it will only allow for one mounted block device
int connect_bd(struct block_device *bd)
{
    mounted_bd = bd;
    if (f_mount(&fatfs, "", 0) == FR_OK) {
        return 0;
    } else {
        mounted_bd = NULL;
        return -1;
    }
}

void disconnect_bd(struct block_device *bd)
{
    f_unmount("");
    mounted_bd = NULL;
    return 0;
}

//---------------------------------------------------------------------------
static int _lock_sema_id = -1;

//---------------------------------------------------------------------------
static int _fs_init_lock(void)
{
    iop_sema_t sp;

    M_DEBUG("%s\n", __func__);

    sp.initial = 1;
    sp.max = 1;
    sp.option = 0;
    sp.attr = 0;
    if ((_lock_sema_id = CreateSema(&sp)) < 0) {
        return (-1);
    }

    return (0);
}

//---------------------------------------------------------------------------
static void _fs_lock(void)
{
    M_DEBUG("%s\n", __func__);

    WaitSema(_lock_sema_id);
}

//---------------------------------------------------------------------------
static void _fs_unlock(void)
{
    M_DEBUG("%s\n", __func__);

    SignalSema(_lock_sema_id);
}

//---------------------------------------------------------------------------
static void fs_reset(void)
{
    int i;

    M_DEBUG("%s\n", __func__);

    if (_lock_sema_id >= 0)
        DeleteSema(_lock_sema_id);

    _fs_init_lock();
}

//---------------------------------------------------------------------------
static int fs_inited = 0;

//---------------------------------------------------------------------------
static int fs_dummy(void)
{
    M_DEBUG("%s\n", __func__);

    return -5;
}

//---------------------------------------------------------------------------
static int fs_init(iop_device_t *driver)
{
    M_DEBUG("%s\n", __func__);

    if (!fs_inited) {
        fs_reset();
        fs_inited = 1;
    }

    return 1;
}

//---------------------------------------------------------------------------
static int fs_open(iop_file_t *fd, const char *name, int flags, int mode)
{
    M_DEBUG("%s: %s flags=%X mode=%X\n", __func__, name, flags, mode);

    int ret;
    BYTE f_mode = FA_OPEN_EXISTING;

    _fs_lock();

    if (fd->privdata == NULL) {
        fd->privdata = malloc(sizeof(FIL));
    }

    // translate mode
    if (flags & O_RDONLY)
        f_mode |= FA_READ;
    if (flags & O_WRONLY)
        f_mode |= FA_WRITE;
    if (flags & O_CREAT)
        f_mode |= FA_OPEN_ALWAYS;
    if (flags & O_TRUNC)
        f_mode |= FA_CREATE_ALWAYS;
    if (flags & O_APPEND)
        f_mode |= FA_OPEN_APPEND;

    ret = f_open(fd->privdata, name, f_mode);

    if (ret != FR_OK) {
        free(fd->privdata);
        fd->privdata = NULL;
        ret = -ret;
    } else {
        ret = 1;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_close(iop_file_t *fd)
{


    M_DEBUG("%s\n", __func__);

    int ret = FR_OK;

    _fs_lock();

    if (fd->privdata) {
        ret = f_close(fd->privdata);
        free(fd->privdata);
        fd->privdata = NULL;
    }

    _fs_unlock();
    return -ret;
}

//---------------------------------------------------------------------------

s64 fs_lseek64(iop_file_t *fd, s64 offset, int whence)
{
    M_DEBUG("%s\n", __func__);

    int res;

    if (fd->privdata == NULL)
        return -ENOENT;

    _fs_lock();

    FIL *file = (FIL *)(fd->privdata);

    FSIZE_t off = offset;

    switch (whence) {
        case SEEK_CUR:
            off += file->fptr;
            break;
        case SEEK_END:
            off = file->obj.objsize - offset;
            break;
    }

    res = f_lseek(file, off);

    _fs_unlock();
    return (res == FR_OK) ? file->fptr : -res;
}

static int fs_lseek(iop_file_t *fd, int offset, int whence)
{
    return fs_lseek64(fd, (s64)offset, whence);
}

#if FF_FS_READONLY == 0
//---------------------------------------------------------------------------
static int fs_write(iop_file_t *fd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    UINT bw;

    if (fd->privdata == NULL)
        return -ENOENT;

    _fs_lock();

    ret = f_write(fd->privdata, buffer, size, &bw);

    _fs_unlock();
    return (ret == FR_OK) ? bw : 0;
}
#endif

//---------------------------------------------------------------------------
static int fs_read(iop_file_t *fd, void *buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    UINT br;

    if (fd->privdata == NULL)
        return -ENOENT;

    _fs_lock();

    ret = f_read(fd->privdata, buffer, size, &br);

    _fs_unlock();
    return (ret == FR_OK) ? br : 0;
}

static iop_device_ops_t fs_functarray = {
    &fs_init,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    &fs_open,
    &fs_close,
    &fs_read,
    #if FF_FS_READONLY == 0
    &fs_write,
    #else
    (void *)&fs_dummy,
    #endif
    &fs_lseek,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    &fs_lseek64,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
};
static iop_device_t fs_driver = {
    "mass",
    IOP_DT_FS | IOP_DT_FSEXT,
    2,
    "FATFS driver",
    &fs_functarray};

/* init file system driver */
int InitFS(void)
{
    M_DEBUG("%s\n", __func__);

    DelDrv("mass");
    return (AddDrv(&fs_driver) == 0 ? 0 : -1);
}