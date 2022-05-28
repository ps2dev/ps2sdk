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

#include <usbhdfsd-common.h>

#include "ff.h"

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

//---------------------------------------------------------------------------
static int _lock_sema_id = -1;

//---------------------------------------------------------------------------
static int _fs_init_lock(void)
{
    iop_sema_t sp;

    M_DEBUG("%s\n", __func__);

    sp.initial = 1;
    sp.max     = 1;
    sp.option  = 0;
    sp.attr    = 0;
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
static int fs_init(iop_device_t* driver)
{
    M_DEBUG("%s\n", __func__);

    if (!fs_inited) {
        fs_reset();
        fs_inited = 1;
    }

    return 1;
}

//---------------------------------------------------------------------------
static int fs_open(iop_file_t* fd, const char* name, int flags, int mode)
{
    M_DEBUG("%s: %s flags=%X mode=%X\n", __func__, name, flags, mode);

    int ret;

    _fs_lock();

    if (fd->privdata == NULL){
        fd->privdata = malloc(sizeof(FIL));
    }

    ret = f_open(fd->privdata, name, mode); // TODO: check if mode matches

    if (ret == FR_OK){
        ret = 1;
    }
    else{
        free(fd->privdata);
        fd->privdata = NULL;
        ret = -ENOENT;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_close(iop_file_t* fd)
{
    

    M_DEBUG("%s\n", __func__);

    int ret;

    _fs_lock();

    if (fd->privdata){
        ret = f_close(fd->privdata);
        free(fd->privdata);
        fd->privdata = NULL;
    }

    _fs_unlock();
    return 0;
}

//---------------------------------------------------------------------------
static int fs_lseek(iop_file_t* fd, int offset, int whence)
{

    M_DEBUG("%s\n", __func__);

    int res;

    if (fd->privdata == NULL) return -1;

    _fs_lock();
    
    FIL* file = (FIL*)(fd->privdata);

    switch (whence){
        case SEEK_CUR:
            offset += file->fptr;
            break;
        case SEEK_END:
            offset = file->obj.objsize - offset;
            break;
    }

    res = f_lseek(file, offset);

    _fs_unlock();
    return offset;
}

//---------------------------------------------------------------------------
static int fs_write(iop_file_t* fd, void* buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    UINT bw;

    if (fd->privdata == NULL) return 0;

    _fs_lock();

    ret = f_write(fd->privdata, buffer, size, &bw);

    _fs_unlock();
    return (ret == FR_OK)? bw : 0;
}

//---------------------------------------------------------------------------
static int fs_read(iop_file_t* fd, void* buffer, int size)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    UINT br;

    if (fd->privdata == NULL) return 0;

    _fs_lock();

    ret = f_read(fd->privdata, buffer, size, &br);

    _fs_unlock();
    return (ret == FR_OK)? br : 0;
}

//---------------------------------------------------------------------------
static int fs_remove(iop_file_t* fd, const char* name)
{
    M_DEBUG("%s\n", __func__);

    int ret;

    _fs_lock();

    ret = f_unlink(name);

    _fs_unlock();
    return ret; // TODO: convert return value
}

//---------------------------------------------------------------------------
static int fs_mkdir(iop_file_t* fd, const char* name, int mode)
{
    M_DEBUG("%s\n", __func__);

    int ret;

    _fs_lock();

    ret = f_mkdir(name);
   
    _fs_unlock();
    return ret; // TODO: convert return value
}

//---------------------------------------------------------------------------
static int fs_rmdir(iop_file_t* fd, const char* name)
{
    M_DEBUG("%s\n", __func__);

    int ret;

    _fs_lock();

    ret = f_rmdir(name);
   
    _fs_unlock();
    return ret; // TODO: convert return value
}

//---------------------------------------------------------------------------
static int fs_dopen(iop_file_t* fd, const char* name)
{
    M_DEBUG("%s: unit %d name %s\n", __func__, fd->unit, name);

    int ret;

    _fs_lock();

    if (fd->privdata == NULL){
        fd->privdata = malloc(sizeof(DIR));
    }

    ret = f_opendir(fd->privdata, name);

    if (ret == FR_OK){
        ret = 1;
    }
    else{
        free(fd->privdata);
        fd->privdata = NULL;
        ret = -ENOENT;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_dclose(iop_file_t* fd)
{
    M_DEBUG("%s\n", __func__);

    int ret;

    _fs_lock();

    if (fd->privdata){
        ret = f_closedir(fd->privdata);
        free(fd->privdata);
        fd->privdata = NULL;
    }
    
    _fs_unlock();
    return 0;
}

//---------------------------------------------------------------------------
static int fs_dread(iop_file_t* fd, iox_dirent_t* buffer)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    FILINFO fno;

    if (fd->privdata == NULL) return 0;

    _fs_lock();

    ret = f_readdir(fd->privdata, &fno);

    if (ret == FR_OK){
        strncpy(buffer->name, fno.fname, 255);
        // TODO: fill buffer->stat structure from fno
        ret = 1;
    } // TODO: check if return values are correct
    else{
        ret = 0;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_getstat(iop_file_t* fd, const char* name, iox_stat_t* stat)
{
    M_DEBUG("%s: unit %d name %s\n", __func__, fd->unit, name);

    int ret;
    FILINFO fno;

    if (fd->privdata == NULL) return -ENOENT;

    _fs_lock();

    ret = f_stat(name, &fno);
    // TODO: fill stat structure from fno

    _fs_unlock();
    return 0;
}

//---------------------------------------------------------------------------
int fs_ioctl(iop_file_t* fd, int cmd, void* data)
{
    M_DEBUG("%s\n", __func__);

    int ret = 0;

    _fs_lock();

    // TODO
    switch (cmd) {
    case USBMASS_IOCTL_RENAME:
        break;
    case USBMASS_IOCTL_GET_CLUSTER:
        break;
    case USBMASS_IOCTL_GET_LBA:
        break;
    case USBMASS_IOCTL_GET_DRIVERNAME:
        break;
    case USBMASS_IOCTL_CHECK_CHAIN:
        break;
    default:
        break;
    }

    _fs_unlock();
    return ret;
}

int fs_rename(iop_file_t *fd, const char *path, const char *newpath)
{
    M_DEBUG("%s\n", __func__);

    int ret;

    if (fd->privdata == NULL) return -ENOENT;

    _fs_lock();

    ret = f_rename(path, newpath);    

    _fs_unlock();
    return (ret == FR_OK)? 0 : -ENOENT;
}

static int fs_devctl(iop_file_t *fd, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int ret;

    _fs_lock();

    // TODO
    switch(cmd)
    {
        case USBMASS_DEVCTL_STOP_UNIT:
            break;
        case USBMASS_DEVCTL_STOP_ALL:
            ret = 0;
            break;
        default:
            ret = -ENXIO;
    }

    _fs_unlock();

    return ret;
}

static iop_device_ops_t fs_functarray = {
    &fs_init,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
    &fs_open,
    &fs_close,
    &fs_read,
    &fs_write,
    &fs_lseek,
    &fs_ioctl,
    &fs_remove,
    &fs_mkdir,
    &fs_rmdir,
    &fs_dopen,
    &fs_dclose,
    &fs_dread,
    &fs_getstat,
    (void*)&fs_dummy,
    &fs_rename,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
    &fs_devctl,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
    (void*)&fs_dummy,
};
static iop_device_t fs_driver = {
    "mass",
    IOP_DT_FS | IOP_DT_FSEXT,
    2,
    "FATFS driver",
    &fs_functarray
};

/* init file system driver */
int InitFS(void)
{
    M_DEBUG("%s\n", __func__);

    DelDrv("mass");
    return (AddDrv(&fs_driver) == 0 ? 0 : -1);
}

