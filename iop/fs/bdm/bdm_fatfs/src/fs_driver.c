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

#define U64_2XU32(val)  ((u32*)val)[1], ((u32*)val)[0]

fatfs_fs_driver_mount_info fs_driver_mount_info[FF_VOLUMES];

#define FATFS_FS_DRIVER_MOUNT_INFO_MAX ((int)(sizeof(fs_driver_mount_info) / sizeof(fs_driver_mount_info[0])))

// Macros for defining the modified path on stack.
#define FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(varname) \
    const char *modified_##varname;

#define FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(varname, fd) \
    { \
        if ((fd)->unit != 0) \
        { \
            int strlen_##varname; \
            char *modified_scope_##varname; \
            \
            strlen_##varname = strlen(varname); \
            modified_scope_##varname = __builtin_alloca(3 + strlen_##varname + 1); \
            modified_scope_##varname[0] = '0' + (fd)->unit; \
            modified_scope_##varname[1] = ':'; \
            modified_scope_##varname[2] = '/'; \
            memcpy((modified_scope_##varname) + 3, varname, strlen_##varname); \
            modified_scope_##varname[3 + strlen_##varname] = '\x00'; \
            modified_##varname = modified_scope_##varname; \
        } \
        else \
        { \
            modified_##varname = varname; \
        } \
    }



//---------------------------------------------------------------------------
static int _fs_lock_sema_id = -1;

//---------------------------------------------------------------------------
static int _fs_init_lock(void)
{
    iop_sema_t sp;

    M_DEBUG("%s\n", __func__);

    sp.initial = 1;
    sp.max     = 1;
    sp.option  = 0;
    sp.attr    = 0;
    if ((_fs_lock_sema_id = CreateSema(&sp)) < 0) {
        return (-1);
    }

    return (0);
}

//---------------------------------------------------------------------------
static void _fs_lock(void)
{
    //M_DEBUG("%s\n", __func__);

    WaitSema(_fs_lock_sema_id);
}

//---------------------------------------------------------------------------
static void _fs_unlock(void)
{
    //M_DEBUG("%s\n", __func__);

    SignalSema(_fs_lock_sema_id);
}

//---------------------------------------------------------------------------
static void fs_reset(void)
{
    M_DEBUG("%s\n", __func__);

    if (_fs_lock_sema_id >= 0)
        DeleteSema(_fs_lock_sema_id);

    _fs_init_lock();
}

static void fatfs_fs_driver_initialize_all_mount_info(void)
{
    int i;
    for (i = 0; i < FATFS_FS_DRIVER_MOUNT_INFO_MAX; i += 1) {
        memset(&(fs_driver_mount_info[i].fatfs), 0, sizeof(fs_driver_mount_info[i].fatfs));
        fs_driver_mount_info[i].mounted_bd = NULL;
    }
}

static int fatfs_fs_driver_find_mount_info_index_from_block_device(const struct block_device *bd)
{
    int i;
    for (i = 0; i < FATFS_FS_DRIVER_MOUNT_INFO_MAX; i += 1) {
        if (fs_driver_mount_info[i].mounted_bd == bd) {
            return i;
        }
    }
    return -1;
}

static int fatfs_fs_driver_find_mount_info_index_free(void)
{
    return fatfs_fs_driver_find_mount_info_index_from_block_device(NULL);
}

struct block_device *fatfs_fs_driver_get_mounted_bd_from_index(int mount_info_index)
{
    struct block_device *mounted_bd;
    if (mount_info_index > FATFS_FS_DRIVER_MOUNT_INFO_MAX) {
        return NULL;
    }
    mounted_bd = fs_driver_mount_info[mount_info_index].mounted_bd;
    return mounted_bd;
}

static FRESULT fatfs_fs_driver_mount_bd(int mount_info_index, struct block_device *bd)
{
    int ret;
    char mount_point[3];

    M_DEBUG("%s\n", __func__);

    mount_point[0] = '0' + mount_info_index;
    mount_point[1] = ':';
    mount_point[2] = '\x00';

    fs_driver_mount_info[mount_info_index].mounted_bd = bd;
    ret = f_mount(&(fs_driver_mount_info[mount_info_index].fatfs), mount_point, 1);
    if (ret != FR_OK) {
        fs_driver_mount_info[mount_info_index].mounted_bd = NULL;
    }
    return ret;
}

static void fatfs_fs_driver_unmount_bd(int mount_info_index)
{
    char mount_point[3];
    mount_point[0] = '0' + mount_info_index;
    mount_point[1] = ':';
    mount_point[2] = '\x00';

    f_unmount(mount_point);
    fs_driver_mount_info[mount_info_index].mounted_bd = NULL;
}

static void fatfs_fs_driver_stop_single_bd(int mount_info_index)
{
    struct block_device *mounted_bd;

    mounted_bd = fatfs_fs_driver_get_mounted_bd_from_index(mount_info_index);
    if (mounted_bd != NULL) {
        fatfs_fs_driver_unmount_bd(mount_info_index);
        mounted_bd->stop(mounted_bd);
    }
}

static void fatfs_fs_driver_stop_all_bd(void)
{
    int i;
    for (i = 0; i < FATFS_FS_DRIVER_MOUNT_INFO_MAX; i += 1) {
        fatfs_fs_driver_stop_single_bd(i);
    }
}

int connect_bd(struct block_device *bd)
{
    int mount_info_index;

    M_DEBUG("%s\n", __func__);

    _fs_lock();
    mount_info_index = fatfs_fs_driver_find_mount_info_index_free();
    if (mount_info_index != -1) {
        M_DEBUG("connect_bd: trying to mount to index %d\n", mount_info_index);
        if (fatfs_fs_driver_mount_bd(mount_info_index, bd) == FR_OK) {
            _fs_unlock();
            return 0;
        }
    }
    _fs_unlock();
    M_DEBUG("connect_bd: failed to mount device\n");
    return -1;
}

void disconnect_bd(struct block_device *bd)
{
    int mount_info_index;

    _fs_lock();
    mount_info_index = fatfs_fs_driver_find_mount_info_index_from_block_device(bd);
    if (mount_info_index != -1) {
        fatfs_fs_driver_unmount_bd(mount_info_index);
    }
    _fs_unlock();
}

//---------------------------------------------------------------------------

#define MAX_FILES 128
static FIL fil_structures[MAX_FILES];

#define MAX_DIRS 16
static DIR dir_structures[MAX_DIRS];

static FIL *fs_find_free_fil_structure(void)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_FILES; i++) {
        if (fil_structures[i].obj.fs == NULL) {
            return &fil_structures[i];
        }
    }
    return NULL;
}

static DIR *fs_find_free_dir_structure(void)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_DIRS; i++) {
        if (dir_structures[i].obj.fs == NULL) {
            return &dir_structures[i];
        }
    }
    return NULL;
}

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

    (void)driver;

    return 1;
}

//---------------------------------------------------------------------------
static int fs_open(iop_file_t *fd, const char *name, int flags, int mode)
{
    M_DEBUG("%s: %s flags=%X mode=%X\n", __func__, name, flags, mode);

    int ret;
    BYTE f_mode = FA_OPEN_EXISTING;
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(name);

    (void)mode;

    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(name, fd);

    _fs_lock();

    // check if the slot is free
    fd->privdata = fs_find_free_fil_structure();
    if (fd->privdata == NULL) {
        _fs_unlock();
        return -EMFILE;
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

    ret = f_open(fd->privdata, modified_name, f_mode);

    if (ret != FR_OK) {
        fd->privdata = NULL;
        ret          = -ret;
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
            off = file->obj.objsize + offset;
            break;
    }

    res = f_lseek(file, off);

    _fs_unlock();
    return (res == FR_OK) ? (s64)(file->fptr) : -res;
}

static int fs_lseek(iop_file_t *fd, int offset, int whence)
{
    return fs_lseek64(fd, (s64)offset, whence);
}

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

//---------------------------------------------------------------------------
static int fs_remove(iop_file_t *fd, const char *name)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(name);

    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(name, fd);

    _fs_lock();

    ret = f_unlink(modified_name);

    _fs_unlock();
    return -ret;
}

//---------------------------------------------------------------------------
static int fs_mkdir(iop_file_t *fd, const char *name, int mode)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(name);

    (void)mode;

    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(name, fd);

    _fs_lock();

    ret = f_mkdir(modified_name);

    _fs_unlock();
    return -ret;
}

//---------------------------------------------------------------------------
static int fs_dopen(iop_file_t *fd, const char *name)
{
    M_DEBUG("%s: unit %d name %s\n", __func__, fd->unit, name);

    int ret;
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(name);

    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(name, fd);

    _fs_lock();

    // check if the slot is free
    fd->privdata = fs_find_free_dir_structure();
    if (fd->privdata == NULL) {
        _fs_unlock();
        return -EMFILE;
    }

    ret = f_opendir(fd->privdata, modified_name);

    if (ret != FR_OK) {
        fd->privdata = NULL;
        ret          = -ret;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_dclose(iop_file_t *fd)
{
    M_DEBUG("%s\n", __func__);

    int ret = ENOENT;

    _fs_lock();

    if (fd->privdata) {
        ret = f_closedir(fd->privdata);
        fd->privdata = NULL;
    }

    _fs_unlock();
    return -ret;
}

//--------------------------------------------------------------------------

static void fileInfoToStat(FILINFO *fno, iox_stat_t *stat)
{
    WORD fdate = fno->fdate;
    WORD ftime = fno->ftime;
    unsigned char stime[8];
    u16 year;

    stat->attr           = 0777;
    stat->size           = (unsigned int)(fno->fsize);
    stat->hisize         = (unsigned int)(fno->fsize>>32);

    stat->mode = FIO_S_IROTH | FIO_S_IXOTH;
    if (fno->fattrib & AM_DIR) {
        stat->mode |= FIO_S_IFDIR;
    } else {
        stat->mode |= FIO_S_IFREG;
    }
    if (!(fno->fattrib & AM_RDO)) {
        stat->mode |= FIO_S_IWOTH;
    }

    // Since the VFAT file system does not support timezones, the timezone offset will not be applied.
    // exFAT does support timezones, but the feature is not used/exposed in the FatFs library.
    // Thus, conversion to/from JST may be incorrect.
    // For simplicity's sake, the timezone is not read from the system configuration and timezone conversion is not done.

    stime[0] = 0; // Padding

    stime[4] = (fdate & 31); // Day
    stime[5] = (fdate >> 5) & 15; // Month

    year = (fdate >> 9) + 1980;
    stime[6] = year & 0xff; // Year (low bits)
    stime[7] = (year >> 8) & 0xff; // Year (high bits)

    stime[3] = (ftime >> 11); // Hours
    stime[2] = (ftime >> 5) & 63; // Minutes
    stime[1] = (ftime << 1) & 31; // Seconds (multiplied by 2)

    memcpy(stat->ctime, stime, sizeof(stime));
    memcpy(stat->atime, stime, sizeof(stime));
    memcpy(stat->mtime, stime, sizeof(stime));
}

//---------------------------------------------------------------------------
static int fs_dread(iop_file_t *fd, iox_dirent_t *buffer)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    FILINFO fno;

    if (fd->privdata == NULL)
        return -ENOENT;

    _fs_lock();

    ret = f_readdir(fd->privdata, &fno);

    if (ret == FR_OK && fno.fname[0]) {
        strncpy(buffer->name, fno.fname, 255);
        fileInfoToStat(&fno, &(buffer->stat));
        ret = 1;
    } else {
        ret = 0;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_getstat(iop_file_t *fd, const char *name, iox_stat_t *stat)
{
    M_DEBUG("%s: unit %d name %s\n", __func__, fd->unit, name);

    int ret;
    FILINFO fno;
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(name);

    // FatFs f_stat doesn't handle the root directory, so we'll handle this case ourselves.
    {
        const char *name_no_leading_slash = name;
        while (*name_no_leading_slash == '/') {
            name_no_leading_slash += 1;
        }
        if ((strcmp(name_no_leading_slash, "") == 0) || (strcmp(name_no_leading_slash, ".") == 0)) {
            if (fatfs_fs_driver_get_mounted_bd_from_index(fd->unit) == NULL) {
                return -ENXIO;
            }
            // Return data indicating that it is a directory.
            memset(stat, 0, sizeof(*stat));
            stat->mode = FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH | FIO_S_IFDIR;
            return 0;
        }
    }

    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(name, fd);

    _fs_lock();

    ret = f_stat(modified_name, &fno);

    if (ret == FR_OK) {
        fileInfoToStat(&fno, stat);
    } else {
        ret = -ret;
    }

    _fs_unlock();
    return ret;
}

static int get_frag_list(FIL *file, void *rdata, unsigned int rdatalen)
{
    bd_fragment_t *f = (bd_fragment_t*)rdata;
    int iMaxFragments = rdatalen / sizeof(bd_fragment_t);
    int iFragCount = 0;

    // Get the block device backing the file so we can get the starting LBA of the file system.
    struct block_device* bd = fatfs_fs_driver_get_mounted_bd_from_index(file->obj.fs->pdrv);

    DWORD iClusterStart = file->obj.sclust;
    DWORD iClusterCurrent = iClusterStart;

    do {
        DWORD iClusterNext = get_fat(&file->obj, iClusterCurrent);
        if (iClusterNext != (iClusterCurrent + 1)) {
            // Fragment or file end
            M_DEBUG("fragment: %uc - %uc + 1\n", iClusterStart, iClusterCurrent + 1);
            if (iFragCount < iMaxFragments) {
                f[iFragCount].sector = clst2sect(file->obj.fs, iClusterStart) + bd->sectorOffset;
                f[iFragCount].count  = clst2sect(file->obj.fs, iClusterCurrent) - clst2sect(file->obj.fs, iClusterStart) + file->obj.fs->csize;
                M_DEBUG(" - sectors: 0x%08x%08x count %u\n", U64_2XU32(&f[iFragCount].sector), f[iFragCount].count);
            }
            iFragCount++;
            iClusterStart = iClusterNext;
        }
        iClusterCurrent = iClusterNext;
    } while(iClusterCurrent < file->obj.fs->n_fatent);

    return iFragCount;
}

//---------------------------------------------------------------------------
int fs_ioctl2(iop_file_t *fd, int cmd, void *data, unsigned int datalen, void *rdata, unsigned int rdatalen)
{
    M_DEBUG("%s cmd=%d\n", __func__, cmd);

    int ret   = 0;
    FIL *file = ((FIL *)(fd->privdata));

    (void)data;
    (void)datalen;

    if (file == NULL)
        return -ENXIO;

    _fs_lock();

    switch (cmd) {
        case USBMASS_IOCTL_RENAME:
            // TODO
            // f_rename() requires two string parameters (newpath, oldpath)
            // figure out how to get the path from the current entry object
            ret = -ENOENT;
            break;
        case USBMASS_IOCTL_GET_CLUSTER:
            ret = file->obj.sclust;
            break;
        case USBMASS_IOCTL_GET_LBA:
            // Check for a return buffer and copy the 64bit LBA. If no buffer is provided return an error.
            if (rdata == NULL || rdatalen < sizeof(u64))
            {
                ret = -EINVAL;
                break;
            }

            // Get the block device backing the file so we can get the starting LBA of the file system.
            struct block_device* bd = fatfs_fs_driver_get_mounted_bd_from_index(file->obj.fs->pdrv);
            
            *(u64*)rdata = clst2sect(file->obj.fs, file->obj.sclust) + bd->sectorOffset;
            ret = 0;
            break;
        case USBMASS_IOCTL_GET_DRIVERNAME: {
            struct block_device *mounted_bd = fatfs_fs_driver_get_mounted_bd_from_index(fd->unit);
            ret = (mounted_bd == NULL) ? -ENXIO : *(int *)(mounted_bd->name);

            // Check for a return buffer and copy the whole name.
            if (rdata != NULL)
                strncpy(rdata, mounted_bd->name, rdatalen);
            break;
        }
        case USBMASS_IOCTL_CHECK_CHAIN:
            ret = get_frag_list(file, NULL, 0) == 1 ? 1 : 0;
            break;
        case USBMASS_IOCTL_GET_FRAGLIST:
            ret = get_frag_list(file, rdata, rdatalen);
            break;
        case USBMASS_IOCTL_GET_DEVICE_NUMBER:
        {
            // Check for a return buffer and copy the device number. If no buffer is provided return an error.
            if (rdata == NULL || rdatalen < sizeof(u32))
            {
                ret = -EINVAL;
                break;
            }

            struct block_device* bd = fatfs_fs_driver_get_mounted_bd_from_index(file->obj.fs->pdrv);
            if (bd == NULL)
                ret = -ENXIO;
            else
            {
                *(u32*)rdata = bd->devNr;
                ret = 0;
            }
            break;
        }
        default:
            break;
    }

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
int fs_ioctl(iop_file_t *fd, int cmd, void *data)
{
    return fs_ioctl2(fd, cmd, data, 1024, NULL, 0);
}

int fs_rename(iop_file_t *fd, const char *path, const char *newpath)
{
    M_DEBUG("%s\n", __func__);

    int ret;
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(path);
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_DEFINITIONS(newpath);

    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(path, fd);
    FATFS_FS_DRIVER_NAME_ALLOC_ON_STACK_IMPLEMENTATION(newpath, fd);

    // If old and new path are the same, no need to do anything
    if (strcmp(modified_path, modified_newpath) == 0) {
        return 0;
    }

    _fs_lock();

    ret = f_rename(modified_path, modified_newpath);

    _fs_unlock();
    return -ret;
}

static int fs_devctl(iop_file_t *fd, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int ret;

    (void)name;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    _fs_lock();


    switch (cmd) {
        case USBMASS_DEVCTL_STOP_UNIT: {
            fatfs_fs_driver_stop_all_bd();
            ret        = FR_OK;
            break;
        }
        case USBMASS_DEVCTL_STOP_ALL: {
            fatfs_fs_driver_stop_single_bd(fd->unit);
            ret        = FR_OK;
            break;
        }
        default: {
            ret = -ENXIO;
            break;
        }
    }

    _fs_unlock();

    return ret;
}

static iop_device_ops_t fs_functarray = {
    &fs_init,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    &fs_open,
    &fs_close,
    &fs_read,
    &fs_write,
    &fs_lseek,
    &fs_ioctl,
    &fs_remove,
    &fs_mkdir,
    &fs_remove,
    &fs_dopen,
    &fs_dclose,
    &fs_dread,
    &fs_getstat,
    (void *)&fs_dummy,
    &fs_rename,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    &fs_lseek64,
    &fs_devctl,
    (void *)&fs_dummy,
    (void *)&fs_dummy,
    &fs_ioctl2,
};
static iop_device_t fs_driver = {
    "mass",
    IOP_DT_FS | IOP_DT_FSEXT,
    2,
    "FATFS driver",
    &fs_functarray,
};

/* init file system driver */
int InitFS(void)
{
    M_DEBUG("%s\n", __func__);

    fs_reset();
    fatfs_fs_driver_initialize_all_mount_info();

    DelDrv(fs_driver.name);
    return (AddDrv(&fs_driver) == 0 ? 0 : -1);
}
