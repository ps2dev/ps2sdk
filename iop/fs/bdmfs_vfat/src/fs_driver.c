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

#include "ff.h"

#include <usbhdfsd-common.h>

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
static int fs_dummy(void)
{
    M_DEBUG("%s\n", __func__);

    return -5;
}

static int fs_started = 0;
//---------------------------------------------------------------------------
static int fs_init(iop_device_t* driver)
{
    M_DEBUG("%s\n", __func__);

    if (!fs_started){
        _fs_init_lock();
        fs_started = 1;
    }

    return 1;
}

//---------------------------------------------------------------------------
static int fs_open(iop_file_t* fd, const char* name, int flags, int mode)
{

    M_DEBUG("%s: %s flags=%X mode=%X\n", __func__, name, flags, mode);

    _fs_lock();

    

    _fs_unlock();
    return 1;
}

//---------------------------------------------------------------------------
static int fs_close(iop_file_t* fd)
{
    fat_driver* fatd;
    fs_rec* rec = (fs_rec*)fd->privdata;

    M_DEBUG("%s\n", __func__);

    if (rec == NULL)
        return -EBADF;

    _fs_lock();

    if (rec->dirent.file_flag != FS_FILE_FLAG_FILE) {
        _fs_unlock();
        return -EISDIR;
    }

    rec->dirent.file_flag = -1;
    fd->privdata          = NULL;

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    if ((rec->mode & O_WRONLY)) {
        //update direntry size and time
        if (rec->sizeChange) {
            fat_updateSfn(fatd, rec->dirent.fatdir.size, rec->sfnSector, rec->sfnOffset);
        }

        FLUSH_SECTORS(fatd);
    }

    _fs_unlock();
    return 0;
}

//---------------------------------------------------------------------------
static int fs_lseek(iop_file_t* fd, int offset, int whence)
{
    fat_driver* fatd;
    fs_rec* rec = (fs_rec*)fd->privdata;

    M_DEBUG("%s\n", __func__);

    if (rec == NULL)
        return -EBADF;

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    if (rec->dirent.file_flag != FS_FILE_FLAG_FILE) {
        _fs_unlock();
        return -EISDIR;
    }

    switch (whence) {
    case SEEK_SET:
        rec->filePos = offset;
        break;
    case SEEK_CUR:
        rec->filePos += offset;
        break;
    case SEEK_END:
        rec->filePos = rec->dirent.fatdir.size + offset;
        break;
    default:
        _fs_unlock();
        return -1;
    }
    if (rec->filePos < 0) {
        rec->filePos = 0;
    }
    if (rec->filePos > rec->dirent.fatdir.size) {
        rec->filePos = rec->dirent.fatdir.size;
    }

    _fs_unlock();
    return rec->filePos;
}

//---------------------------------------------------------------------------
static int fs_write(iop_file_t* fd, void* buffer, int size)
{
    fat_driver* fatd;
    fs_rec* rec = (fs_rec*)fd->privdata;
    int result;
    int updateClusterIndices = 0;

    M_DEBUG("%s\n", __func__);

    if (rec == NULL)
        return -EBADF;

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    if (rec->dirent.file_flag != FS_FILE_FLAG_FILE) {
        _fs_unlock();
        return -EISDIR;
    }

    if (!(rec->mode & O_WRONLY)) {
        _fs_unlock();
        return -EACCES;
    }

    if (size <= 0) {
        _fs_unlock();
        return 0;
    }

    result = fat_writeFile(fatd, &rec->dirent.fatdir, &updateClusterIndices, rec->filePos, (unsigned char*)buffer, size);
    if (result > 0) { //write succesful
        rec->filePos += result;
        if (rec->filePos > rec->dirent.fatdir.size) {
            rec->dirent.fatdir.size = rec->filePos;
            rec->sizeChange         = 1;
            //if new clusters allocated - then update file cluster indices
            if (updateClusterIndices) {
                fat_setFatDirChain(fatd, &rec->dirent.fatdir);
            }
        }
    }

    _fs_unlock();
    return result;
}

//---------------------------------------------------------------------------
static int fs_read(iop_file_t* fd, void* buffer, int size)
{
    fat_driver* fatd;
    fs_rec* rec = (fs_rec*)fd->privdata;
    int result;

    M_DEBUG("%s\n", __func__);

    if (rec == NULL)
        return -EBADF;

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    if (rec->dirent.file_flag != FS_FILE_FLAG_FILE) {
        _fs_unlock();
        return -EISDIR;
    }

    if (!(rec->mode & O_RDONLY)) {
        _fs_unlock();
        return -EACCES;
    }

    if (size <= 0) {
        _fs_unlock();
        return 0;
    }

    if ((rec->filePos + size) > rec->dirent.fatdir.size) {
        size = rec->dirent.fatdir.size - rec->filePos;
    }

    result = fat_readFile(fatd, &rec->dirent.fatdir, rec->filePos, (unsigned char*)buffer, size);
    if (result > 0) { //read succesful
        rec->filePos += result;
    }

    _fs_unlock();
    return result;
}

//---------------------------------------------------------------------------
static int fs_remove(iop_file_t* fd, const char* name)
{
    fat_driver* fatd;
    fs_rec* rec;
    int result;
    unsigned int cluster;
    fat_dir fatdir;

    M_DEBUG("%s\n", __func__);

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        result = -ENODEV;
        _fs_unlock();
        return result;
    }

    cluster = 0; //allways start from root
    M_DEBUG("Calling fat_getFileStartCluster from fs_remove\n");
    result = fat_getFileStartCluster(fatd, name, &cluster, &fatdir);
    if (result < 0) {
        _fs_unlock();
        return result;
    }

    rec = fs_findFileSlotByCluster(fatdir.startCluster);

    //file is opened - can't delete the file
    if (rec != NULL) {
        result = -EINVAL;
        _fs_unlock();
        return result;
    }

    result = fat_deleteFile(fatd, name, 0);
    FLUSH_SECTORS(fatd);

    _fs_unlock();
    return result;
}

//---------------------------------------------------------------------------
static int fs_mkdir(iop_file_t* fd, const char* name, int mode)
{
    fat_driver* fatd;
    int ret;
    int sfnOffset;
    unsigned int sfnSector;
    unsigned int cluster;

    M_DEBUG("%s\n", __func__);

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    M_DEBUG("fs_mkdir: name=%s \n", name);
    ret = fat_createFile(fatd, name, 1, 0, &cluster, &sfnSector, &sfnOffset);

    //directory of the same name already exist
    if (ret == 2) {
        ret = -EEXIST;
    }
    FLUSH_SECTORS(fatd);

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_rmdir(iop_file_t* fd, const char* name)
{
    fat_driver* fatd;
    int ret;

    M_DEBUG("%s\n", __func__);

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    ret = fat_deleteFile(fatd, name, 1);
    FLUSH_SECTORS(fatd);
    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_dopen(iop_file_t* fd, const char* name)
{
    fat_driver* fatd;
    int is_root = 0;
    fs_dir* rec;

    M_DEBUG("%s: unit %d name %s\n", __func__, fd->unit, name);

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    if (((name[0] == '/') && (name[1] == '\0'))
        || ((name[0] == '/') && (name[1] == '.') && (name[2] == '\0'))) {
        name    = "/";
        is_root = 1;
    }

    fd->privdata = malloc(sizeof(fs_dir));
    memset(fd->privdata, 0, sizeof(fs_dir)); //NB: also implies "file_flag = FS_FILE_FLAG_FOLDER;"
    rec = (fs_dir*)fd->privdata;

    rec->status = fat_getFirstDirentry(fatd, name, &rec->fatdlist, &rec->dirent.fatdir, &rec->current_fatdir);

    // root directory may have no entries, nothing else may.
    if (rec->status == 0 && !is_root)
        rec->status = -EFAULT;

    if (rec->status < 0)
        free(fd->privdata);

    _fs_unlock();
    return rec->status;
}

//---------------------------------------------------------------------------
static int fs_dclose(iop_file_t* fd)
{
    fs_dir* rec = (fs_dir*)fd->privdata;

    M_DEBUG("%s\n", __func__);

    if (fd->privdata == NULL)
        return -EBADF;

    _fs_lock();
    M_DEBUG("fs_dclose called: unit %d\n", fd->unit);
    if (rec->dirent.file_flag != FS_FILE_FLAG_FOLDER) {
        _fs_unlock();
        return -ENOTDIR;
    }

    free(fd->privdata);
    fd->privdata = NULL;
    _fs_unlock();
    return 0;
}

//---------------------------------------------------------------------------
static int fs_dread(iop_file_t* fd, iox_dirent_t* buffer)
{
    fat_driver* fatd;
    int ret;
    fs_dir* rec = (fs_dir*)fd->privdata;

    M_DEBUG("%s\n", __func__);

    if (rec == NULL)
        return -EBADF;

    _fs_lock();

    M_DEBUG("fs_dread called: unit %d\n", fd->unit);

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    if (rec->dirent.file_flag != FS_FILE_FLAG_FOLDER) {
        _fs_unlock();
        return -ENOTDIR;
    }

    while (rec->status > 0
        && (rec->current_fatdir.attr & FAT_ATTR_VOLUME_LABEL
               || ((rec->current_fatdir.attr & (FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM)) == (FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM))))
        rec->status = fat_getNextDirentry(fatd, &rec->fatdlist, &rec->current_fatdir);

    ret = rec->status;
    if (rec->status >= 0) {
        memset(buffer, 0, sizeof(iox_dirent_t));
        fillStat(&buffer->stat, &rec->current_fatdir);
        strcpy(buffer->name, (const char*)rec->current_fatdir.name);
    }

    if (rec->status > 0)
        rec->status = fat_getNextDirentry(fatd, &rec->fatdlist, &rec->current_fatdir);

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_getstat(iop_file_t* fd, const char* name, iox_stat_t* stat)
{
    fat_driver* fatd;
    int ret;
    unsigned int cluster = 0;
    fat_dir fatdir;

    M_DEBUG("%s: unit %d name %s\n", __func__, fd->unit, name);

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    M_DEBUG("Calling fat_getFileStartCluster from fs_getstat\n");
    ret = fat_getFileStartCluster(fatd, name, &cluster, &fatdir);
    if (ret < 0) {
        _fs_unlock();
        return ret;
    }

    memset(stat, 0, sizeof(iox_stat_t));
    fillStat(stat, &fatdir);

    _fs_unlock();
    return 0;
}

//---------------------------------------------------------------------------
int fs_ioctl(iop_file_t* fd, int cmd, void* data)
{
    fat_driver* fatd;
    struct fs_dirent* dirent = (struct fs_dirent*)fd->privdata; //Remember to re-cast this to the right structure (either fs_rec or fs_dir)!
    int ret;

    M_DEBUG("%s\n", __func__);

    if (dirent == NULL)
        return -EBADF;

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    switch (cmd) {
    case USBMASS_IOCTL_RENAME:
        ret = fat_renameFile(fatd, &dirent->fatdir, data); //No need to re-cast since this inner structure is a common one.
        FLUSH_SECTORS(fatd);
        break;
    case USBMASS_IOCTL_GET_CLUSTER:
        ret = ((fs_rec*)fd->privdata)->dirent.fatdir.startCluster;
        break;
    case USBMASS_IOCTL_GET_LBA:
        ret = fat_cluster2sector(&fatd->partBpb, ((fs_rec*)fd->privdata)->dirent.fatdir.startCluster);
        break;
    case USBMASS_IOCTL_GET_DRIVERNAME:
        ret = *(int*)fatd->bd->name;
        break;
    case USBMASS_IOCTL_CHECK_CHAIN:
        ret = fat_CheckChain(fatd, ((fs_rec*)fd->privdata)->dirent.fatdir.startCluster);
        break;
    default:
        ret = fs_dummy();
    }

    _fs_unlock();
    return ret;
}

int fs_rename(iop_file_t *fd, const char *path, const char *newpath)
{
    fat_dir fatdir;
    fat_driver *fatd;
    fs_rec *rec = NULL;
    unsigned int cluster;
    struct fs_dirent *dirent = (struct fs_dirent *)fd->privdata;
    int ret;

    if (dirent == NULL)
        return -EBADF;

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    //find the file
    cluster = 0; //allways start from root
    M_DEBUG("Calling fat_getFileStartCluster from fs_rename\n");
    ret = fat_getFileStartCluster(fatd, path, &cluster, &fatdir);
    if (ret < 0 && ret != -ENOENT) {
        _fs_unlock();
        return ret;
    } else {
        //File exists. Check if the file is already open
        rec = fs_findFileSlotByCluster(fatdir.startCluster);
        if (rec != NULL) {
            _fs_unlock();
            return -EACCES;
        }
    }

    ret = fat_renameFile(fatd, &fatdir, newpath);
    FLUSH_SECTORS(fatd);

    _fs_unlock();
    return ret;
}

static int fs_devctl(iop_file_t *fd, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int ret;

    _fs_lock();

    switch(cmd)
    {
        case USBMASS_DEVCTL_STOP_UNIT:
            ret = fat_stopUnit(fd->unit);
            break;
        case USBMASS_DEVCTL_STOP_ALL:
            fat_stopAll();
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

