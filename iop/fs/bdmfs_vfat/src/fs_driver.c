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
#include <ioman.h>
#include <stdio.h>
#include <sys/stat.h>

#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thsemap.h>

#include "common.h"
#include "fat.h"
#include "fat_driver.h"
#include "fat_write.h"
#include <usbhdfsd-common.h>

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define FLUSH_SECTORS fat_flushSectors

enum FS_FILE_FLAG {
    FS_FILE_FLAG_FOLDER = 0,
    FS_FILE_FLAG_FILE
};

struct fs_dirent { //Common structure for both directories and regular files.
    //This flag is always 1 for a file, and always 0 for a folder (different typedef)
    //Routines that handle both must test it, and then typecast the privdata pointer
    //to the type that is appropriate for the given case. (see also fs_dir typedef)
    short int file_flag;
    short int sizeChange; //flag, used for regular files. Otherwise padding.
    fat_dir fatdir;
};

typedef struct _fs_rec {
    struct fs_dirent dirent;
    short int sizeChange; //flag
    unsigned int filePos;
    int mode;               //file open mode
    unsigned int sfnSector; //short filename sector  - write support
    int sfnOffset;          //short filename offset  - write support
} fs_rec;

typedef struct _fs_dir {
    struct fs_dirent dirent;
    int status;
    fat_dir_list fatdlist;
    fat_dir current_fatdir;
} fs_dir;

#define MAX_FILES 16
static fs_rec fsRec[MAX_FILES]; //file info record

static void fillStat(fio_stat_t* stat, const fat_dir* fatdir)
{
    stat->mode = FIO_SO_IROTH | FIO_SO_IXOTH;
    if (fatdir->attr & FAT_ATTR_DIRECTORY) {
        stat->mode |= FIO_SO_IFDIR;
    } else {
        stat->mode |= FIO_SO_IFREG;
    }
    if (!(fatdir->attr & FAT_ATTR_READONLY)) {
        stat->mode |= FIO_SO_IWOTH;
    }

    stat->size = fatdir->size;

    //set created Date: Day, Month, Year
    stat->ctime[4] = fatdir->cdate[0];
    stat->ctime[5] = fatdir->cdate[1];
    stat->ctime[6] = fatdir->cdate[2];
    stat->ctime[7] = fatdir->cdate[3];

    //set created Time: Hours, Minutes, Seconds
    stat->ctime[3] = fatdir->ctime[0];
    stat->ctime[2] = fatdir->ctime[1];
    stat->ctime[1] = fatdir->ctime[2];

    //set accessed Date: Day, Month, Year
    stat->atime[4] = fatdir->adate[0];
    stat->atime[5] = fatdir->adate[1];
    stat->atime[6] = fatdir->adate[2];
    stat->atime[7] = fatdir->adate[3];

    //set modified Date: Day, Month, Year
    stat->mtime[4] = fatdir->mdate[0];
    stat->mtime[5] = fatdir->mdate[1];
    stat->mtime[6] = fatdir->mdate[2];
    stat->mtime[7] = fatdir->mdate[3];

    //set modified Time: Hours, Minutes, Seconds
    stat->mtime[3] = fatdir->mtime[0];
    stat->mtime[2] = fatdir->mtime[1];
    stat->mtime[1] = fatdir->mtime[2];
}

/*************************************************************************************/
/*    File IO functions                                                              */
/*************************************************************************************/

//---------------------------------------------------------------------------
static fs_rec* fs_findFreeFileSlot(void)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_FILES; i++) {
        if (fsRec[i].dirent.file_flag < 0) {
            return &fsRec[i];
            break;
        }
    }
    return NULL;
}

//---------------------------------------------------------------------------
static fs_rec* fs_findFileSlotByCluster(unsigned int startCluster)
{
    int i;

    M_DEBUG("%s\n", __func__);

    for (i = 0; i < MAX_FILES; i++) {
        if (fsRec[i].dirent.file_flag >= 0 && fsRec[i].dirent.fatdir.startCluster == startCluster) {
            return &fsRec[i];
        }
    }
    return NULL;
}

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

    for (i = 0; i < MAX_FILES; i++)
        fsRec[i].dirent.file_flag = -1;

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
static int fs_open(iop_file_t* fd, const char* name, int mode)
{
    fat_driver* fatd;
    fs_rec *rec, *rec2;
    int ret;
    unsigned int cluster;
    char escapeNotExist;

    M_DEBUG("%s: %s mode=%X\n", __func__, name, mode);

    _fs_lock();

    fatd = fat_getData(fd->unit);
    if (fatd == NULL) {
        _fs_unlock();
        return -ENODEV;
    }

    //check if the slot is free
    rec = fs_findFreeFileSlot();
    if (rec == NULL) {
        _fs_unlock();
        return -EMFILE;
    }

    //find the file
    cluster = 0; //allways start from root
    M_DEBUG("Calling fat_getFileStartCluster from fs_open\n");
    ret = fat_getFileStartCluster(fatd, name, &cluster, &rec->dirent.fatdir);
    if (ret < 0 && ret != -ENOENT) {
        _fs_unlock();
        return ret;
    } else {
        //File exists. Check if the file is already open
        rec2 = fs_findFileSlotByCluster(rec->dirent.fatdir.startCluster);
        if (rec2 != NULL) {
            if ((mode & O_WRONLY) ||       //current file is opened for write
                (rec2->mode & O_WRONLY)) { //other file is opened for write
                _fs_unlock();
                return -EACCES;
            }
        }
    }

    if (mode & O_WRONLY) { //dlanor: corrected bad test condition
        cluster = 0;       //start from root

        escapeNotExist = 1;
        if (mode & O_CREAT) {
            M_DEBUG("FAT I: O_CREAT detected!\n");
            escapeNotExist = 0;
        }

        rec->sfnSector = 0;
        rec->sfnOffset = 0;
        ret            = fat_createFile(fatd, name, 0, escapeNotExist, &cluster, &rec->sfnSector, &rec->sfnOffset);
        if (ret < 0) {
            _fs_unlock();
            return ret;
        }
        //the file already exist but mode is set to truncate
        if (ret == 2 && (mode & O_TRUNC)) {
            M_DEBUG("FAT I: O_TRUNC detected!\n");
            fat_truncateFile(fatd, cluster, rec->sfnSector, rec->sfnOffset);
        }

        //find the file
        cluster = 0; //allways start from root
        M_DEBUG("Calling fat_getFileStartCluster from fs_open after file creation\n");
        ret = fat_getFileStartCluster(fatd, name, &cluster, &rec->dirent.fatdir);
    }

    if (ret < 0) { //At this point, the file should be locatable without any errors.
        _fs_unlock();
        return ret;
    }

    if ((rec->dirent.fatdir.attr & FAT_ATTR_DIRECTORY) == FAT_ATTR_DIRECTORY) {
        // Can't open a directory with fioOpen
        _fs_unlock();
        return -EISDIR;
    }

    rec->dirent.file_flag = FS_FILE_FLAG_FILE;
    rec->mode             = mode;
    rec->filePos          = 0;
    rec->sizeChange       = 0;

    if ((mode & O_APPEND) && (mode & O_WRONLY)) {
        M_DEBUG("FAT I: O_APPEND detected!\n");
        rec->filePos = rec->dirent.fatdir.size;
    }

    //store the slot to user parameters
    fd->privdata = rec;

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
static int fs_mkdir(iop_file_t* fd, const char* name)
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
static int fs_dread(iop_file_t* fd, fio_dirent_t* buffer)
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
        memset(buffer, 0, sizeof(fio_dirent_t));
        fillStat(&buffer->stat, &rec->current_fatdir);
        strcpy(buffer->name, (const char*)rec->current_fatdir.name);
    }

    if (rec->status > 0)
        rec->status = fat_getNextDirentry(fatd, &rec->fatdlist, &rec->current_fatdir);

    _fs_unlock();
    return ret;
}

//---------------------------------------------------------------------------
static int fs_getstat(iop_file_t* fd, const char* name, fio_stat_t* stat)
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

    memset(stat, 0, sizeof(fio_stat_t));
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
    default:
        ret = fs_dummy();
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
    (void*)&fs_dummy
};
static iop_device_t fs_driver = {
    "mass",
    IOP_DT_FS,
    2,
    "VFAT driver",
    &fs_functarray
};

/* init file system driver */
int InitFS(void)
{
    M_DEBUG("%s\n", __func__);

    DelDrv("mass");
    return (AddDrv(&fs_driver) == 0 ? 0 : -1);
}
iop_device_ops_t dev_ops = {
    .init    = &fs_init,
    .deinit  = (void*)&fs_dummy,
    .format  = (void*)&fs_dummy,
    .open    = &fs_open,
    .close   = &fs_close,
    .read    = &fs_read,
    .write   = &fs_write,
    .lseek   = &fs_lseek,
    .ioctl   = &fs_ioctl,
    .remove  = &fs_remove,
    .mkdir   = &fs_mkdir,
    .rmdir   = &fs_rmdir,
    .dopen   = &fs_dopen,
    .dclose  = &fs_dclose,
    .dread   = &fs_dread,
    .getstat = &fs_getstat,
    .chstat  = (void*)&fs_dummy,
};
