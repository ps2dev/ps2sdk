/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# (c) 2023 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * fileXio_ps2sdk.c - Define set methods for _ps2sdk functions
 */

#include <ps2sdkapi.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#define NEWLIB_PORT_AWARE
#include <fileio.h>
#include <fileXio_rpc.h>
#include "iox_stat.h"

typedef struct _fxio_file_info_
{
    // cppcheck-suppress unusedStructMember
    int fd;
    // cppcheck-suppress unusedStructMember
    char filename[];
} _fxio_file_info_t;

#ifdef F___fileXioOpsInitialize
_libcglue_fdman_path_ops_t __fileXio_fdman_path_ops;
_libcglue_fdman_fd_ops_t __fileXio_fdman_ops_file;
_libcglue_fdman_fd_ops_t __fileXio_fdman_ops_dir;

extern void __fileXioOpsInitializeImpl(void);

__attribute__((constructor))
static void __fileXioOpsInitialize(void)
{
    __fileXioOpsInitializeImpl();
}
#else
extern _libcglue_fdman_path_ops_t __fileXio_fdman_path_ops;
extern _libcglue_fdman_fd_ops_t __fileXio_fdman_ops_file;
extern _libcglue_fdman_fd_ops_t __fileXio_fdman_ops_dir;
#endif

#ifdef F___fileXioOpenHelper

#define IOP_O_RDONLY       0x0001
#define IOP_O_WRONLY       0x0002
#define IOP_O_RDWR         0x0003
#define IOP_O_DIROPEN      0x0008  // Internal use for dopen
#define IOP_O_NBLOCK       0x0010
#define IOP_O_APPEND       0x0100
#define IOP_O_CREAT        0x0200
#define IOP_O_TRUNC        0x0400
#define IOP_O_EXCL         0x0800
#define IOP_O_NOWAIT       0x8000

int __fileXioOpenHelper(_libcglue_fdman_fd_info_t *info, const char *buf, int flags, mode_t mode)
{
    int iop_flags = 0;
    int is_dir = 0;
    int iop_fd;

    // newlib flags differ from iop flags
    if ((flags & 3) == O_RDONLY) iop_flags |= IOP_O_RDONLY;
    if ((flags & 3) == O_WRONLY) iop_flags |= IOP_O_WRONLY;
    if ((flags & 3) == O_RDWR  ) iop_flags |= IOP_O_RDWR;
    if (flags & O_NONBLOCK)      iop_flags |= IOP_O_NBLOCK;
    if (flags & O_APPEND)        iop_flags |= IOP_O_APPEND;
    if (flags & O_CREAT)         iop_flags |= IOP_O_CREAT;
    if (flags & O_TRUNC)         iop_flags |= IOP_O_TRUNC;
    if (flags & O_EXCL)          iop_flags |= IOP_O_EXCL;
    //if (flags & O_???)           iop_flags |= IOP_O_NOWAIT;
    if (flags & O_DIRECTORY) {
        iop_flags |= IOP_O_DIROPEN;
        is_dir = 1;
    }

    iop_fd = is_dir ? fileXioDopen(buf) : fileXioOpen(buf, iop_flags, mode);
    if (iop_fd >= 0) {
        _fxio_file_info_t *userdata;
        int buf_len;

        buf_len = strlen(buf);
        userdata = malloc(sizeof(_fxio_file_info_t) + buf_len + 1);
        if (userdata == NULL)
        {
            return -ENOMEM;
        }
        userdata->fd = iop_fd;
        memcpy(userdata, buf, buf_len);
        userdata->filename[buf_len] = '\x00';
        info->userdata = (void *)userdata;
        info->ops = is_dir ? &__fileXio_fdman_ops_dir : &__fileXio_fdman_ops_file;
        return 0;
    }
    return iop_fd;
}
#else
int __fileXioOpenHelper(_libcglue_fdman_fd_info_t *info, const char *buf, int flags, mode_t mode);
#endif

#ifdef F___fileXioGetFdHelper
int __fileXioGetFdHelper(void *userdata)
{
    _fxio_file_info_t *finfo;

    if (userdata == NULL)
    {
        return -EINVAL;
    }

    finfo = (_fxio_file_info_t *)userdata;
    return finfo->fd;
}
#else
int __fileXioGetFdHelper(void *userdata);
#endif

#ifdef F___fileXioGetFilenameHelper
char *__fileXioGetFilenameHelper(void *userdata)
{
    _fxio_file_info_t *finfo;

    if (userdata == NULL)
    {
        return NULL;
    }

    finfo = (_fxio_file_info_t *)userdata;
    return finfo->filename;
}
#else
char *__fileXioGetFilenameHelper(void *userdata);
#endif

#ifdef F___fileXioGetstatHelper
static time_t io_to_posix_time(const unsigned char *ps2time)
{
        struct tm tim;
        tim.tm_sec  = ps2time[1];
        tim.tm_min  = ps2time[2];
        tim.tm_hour = ps2time[3];
        tim.tm_mday = ps2time[4];
        tim.tm_mon  = ps2time[5] - 1;
        tim.tm_year = ((u16)ps2time[6] | ((u16)ps2time[7] << 8)) - 1900;
        return mktime(&tim);
}

static mode_t iox_to_posix_mode(unsigned int ps2mode)
{
        mode_t posixmode = 0;
        if (ps2mode & FIO_S_IFREG) posixmode |= S_IFREG;
        if (ps2mode & FIO_S_IFDIR) posixmode |= S_IFDIR;
        if (ps2mode & FIO_S_IRUSR) posixmode |= S_IRUSR;
        if (ps2mode & FIO_S_IWUSR) posixmode |= S_IWUSR;
        if (ps2mode & FIO_S_IXUSR) posixmode |= S_IXUSR;
        if (ps2mode & FIO_S_IRGRP) posixmode |= S_IRGRP;
        if (ps2mode & FIO_S_IWGRP) posixmode |= S_IWGRP;
        if (ps2mode & FIO_S_IXGRP) posixmode |= S_IXGRP;
        if (ps2mode & FIO_S_IROTH) posixmode |= S_IROTH;
        if (ps2mode & FIO_S_IWOTH) posixmode |= S_IWOTH;
        if (ps2mode & FIO_S_IXOTH) posixmode |= S_IXOTH;
        return posixmode;
}

static void fill_stat(struct stat *stat, const iox_stat_t *fiostat)
{
        stat->st_dev = 0;
        stat->st_ino = 0;
        stat->st_mode = iox_to_posix_mode(fiostat->mode);
        stat->st_nlink = 0;
        stat->st_uid = 0;
        stat->st_gid = 0;
        stat->st_rdev = 0;
        stat->st_size = ((off_t)fiostat->hisize << 32) | (off_t)fiostat->size;
        stat->st_atime = io_to_posix_time(fiostat->atime);
        stat->st_mtime = io_to_posix_time(fiostat->mtime);
        stat->st_ctime = io_to_posix_time(fiostat->ctime);
        stat->st_blksize = 16*1024;
        stat->st_blocks = stat->st_size / 512;
}

int __fileXioGetstatHelper(const char *path, struct stat *buf) {
    iox_stat_t fiostat;

    if (strncmp(path, "tty", 3) == 0 && path[3] >= '0' && path[3] <= '9' && path[4] == ':')
    {
        memset(buf, 0, sizeof(struct stat));
        buf->st_mode = S_IFCHR;
        return 0;
    }

    if (fileXioGetStat(path, &fiostat) < 0) {
        errno = ENOENT;
        return -1;
    }

    fill_stat(buf, &fiostat);

    return 0;
}
#else
int __fileXioGetstatHelper(const char *path, struct stat *buf);
#endif

#ifdef F___fileXioCloseHelper
int __fileXioCloseHelper(void *userdata)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioClose(fd);
    free(userdata);
    return rv;
}
#else
int __fileXioCloseHelper(void *userdata);
#endif

#ifdef F___fileXioDcloseHelper
int __fileXioDcloseHelper(void *userdata)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioDclose(fd);
    free(userdata);
    return rv;
}
#else
int __fileXioDcloseHelper(void *userdata);
#endif

#ifdef F___fileXioReadHelper
int __fileXioReadHelper(void *userdata, void *buf, int nbytes)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioRead(fd, buf, nbytes);
    return rv;
}
#else
int __fileXioReadHelper(void *userdata, void *buf, int nbytes);
#endif

#ifdef F___fileXioLseekHelper
int __fileXioLseekHelper(void *userdata, int offset, int whence)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioLseek(fd, offset, whence);
    return rv;
}
#else
int __fileXioLseekHelper(void *userdata, int offset, int whence);
#endif

#ifdef F___fileXioLseek64Helper
int64_t __fileXioLseek64Helper(void *userdata, int64_t offset, int whence)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioLseek64(fd, offset, whence);
    return rv;
}
#else
int64_t __fileXioLseek64Helper(void *userdata, int64_t offset, int whence);
#endif

#ifdef F___fileXioWriteHelper
int __fileXioWriteHelper(void *userdata, const void *buf, int nbytes)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioWrite(fd, buf, nbytes);
    return rv;
}
#else
int __fileXioWriteHelper(void *userdata, const void *buf, int nbytes);
#endif

#ifdef F___fileXioIoctlHelper
int __fileXioIoctlHelper(void *userdata, int request, void *data)
{
    int rv;
    int fd;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioIoctl(fd, request, data);
    return rv;
}
#else
int __fileXioIoctlHelper(void *userdata, int request, void *data);
#endif

#ifdef F___fileXioDreadHelper
int __fileXioDreadHelper(void *userdata, struct dirent *dir)
{
    int rv;
    int fd;
    iox_dirent_t ioxdir;

    // Took from iox_dirent_t
    #define __MAXNAMLEN 256

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    rv = fileXioDread(fd, &ioxdir);
    if (rv < 0) {
        return -ENOENT;
    }

    dir->d_fileno = rv; // TODO: This number should be in theory a unique number per file
    strncpy(dir->d_name, ioxdir.name, __MAXNAMLEN);
    dir->d_name[__MAXNAMLEN - 1] = 0;
    dir->d_reclen = 0;
    switch (ioxdir.stat.mode & FIO_S_IFMT) {
        case FIO_S_IFLNK: dir->d_type = DT_LNK;     break;
        case FIO_S_IFDIR: dir->d_type = DT_DIR;     break;
        case FIO_S_IFREG: dir->d_type = DT_REG;     break;
        default:          dir->d_type = DT_UNKNOWN; break;
    }

    return rv;
}
#else
int __fileXioDreadHelper(void *userdata, struct dirent *dir);
#endif

#ifdef F___fileXioLseekDirHelper
int __fileXioLseekDirHelper(void *userdata, int offset, int whence)
{
    int i;
    int fd;
    char *filename;
    _fxio_file_info_t *finfo;
    int uid;
    struct dirent dir;

    fd = __fileXioGetFdHelper(userdata);
    if (fd < 0)
    {
        return fd;
    }

    filename = __fileXioGetFilenameHelper(userdata);

    if (filename == NULL)
    {
        return -EINVAL;
    }

    if (userdata == NULL)
    {
        return -EINVAL;
    }

    finfo = (_fxio_file_info_t *)userdata;

    if (whence != SEEK_SET)
    {
        return -EINVAL;
    }

    fileXioDclose(fd);
    uid = fileXioDopen(filename);
    finfo->fd = uid;
    for (i = 0; i < offset; i++) {
        __fileXioDreadHelper(userdata, &dir);
    }

    return offset;
}
#else
int __fileXioLseekDirHelper(void *userdata, int offset, int whence);
#endif

#ifdef F___fileXioOpsInitializeImpl
int __attribute__((weak)) _open(const char *buf, int flags, ...);
int __attribute__((weak)) _unlink(const char *path);
int __attribute__((weak)) _rename(const char *old, const char *new);
int __attribute__((weak)) mkdir(const char *path, mode_t mode);
int __attribute__((weak)) rmdir(const char *path);
int __attribute__((weak)) _stat(const char *path, struct stat *buf);
ssize_t __attribute__((weak)) readlink(const char *path, char *buf, size_t bufsiz);
int __attribute__((weak)) symlink(const char *target, const char *linkpath);
int __attribute__((weak)) _close(int fd);
int __attribute__((weak)) _read(int fd, void *buf, size_t nbytes);
off_t __attribute__((weak)) _lseek(int fd, off_t offset, int whence);
off64_t __attribute__((weak)) lseek64(int fd, off64_t offset, int whence);
int __attribute__((weak)) _write(int fd, const void *buf, size_t nbytes);
int __attribute__((weak)) _ioctl(int fd, int request, void *data);
int __attribute__((weak)) getdents(int fd, void *dd_buf, int count);

extern void __fileXioOpsInitializeImpl(void)
{
    memset(&__fileXio_fdman_path_ops, 0, sizeof(__fileXio_fdman_path_ops));
    // cppcheck-suppress knownConditionTrueFalse
    if (&_open) __fileXio_fdman_path_ops.open = __fileXioOpenHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_unlink) __fileXio_fdman_path_ops.remove = fileXioRemove;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_rename) __fileXio_fdman_path_ops.rename = fileXioRename;
    // cppcheck-suppress knownConditionTrueFalse
    if (&mkdir) __fileXio_fdman_path_ops.mkdir = fileXioMkdir;
    // cppcheck-suppress knownConditionTrueFalse
    if (&rmdir) __fileXio_fdman_path_ops.rmdir = fileXioRmdir;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_stat) __fileXio_fdman_path_ops.stat = __fileXioGetstatHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&readlink) __fileXio_fdman_path_ops.readlink = fileXioReadlink;
    // cppcheck-suppress knownConditionTrueFalse
    if (&symlink) __fileXio_fdman_path_ops.symlink = fileXioSymlink;

    memset(&__fileXio_fdman_ops_file, 0, sizeof(__fileXio_fdman_ops_file));
    __fileXio_fdman_ops_file.getfd = __fileXioGetFdHelper;
    __fileXio_fdman_ops_file.getfilename = __fileXioGetFilenameHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_close) __fileXio_fdman_ops_file.close = __fileXioCloseHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_read) __fileXio_fdman_ops_file.read = __fileXioReadHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_lseek) __fileXio_fdman_ops_file.lseek = __fileXioLseekHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&lseek64) __fileXio_fdman_ops_file.lseek64 = __fileXioLseek64Helper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_write) __fileXio_fdman_ops_file.write = __fileXioWriteHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_ioctl) __fileXio_fdman_ops_file.ioctl = __fileXioIoctlHelper;

    memset(&__fileXio_fdman_ops_dir, 0, sizeof(__fileXio_fdman_ops_dir));
    __fileXio_fdman_ops_dir.getfd = __fileXioGetFdHelper;
    __fileXio_fdman_ops_dir.getfilename = __fileXioGetFilenameHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_close) __fileXio_fdman_ops_dir.close = __fileXioDcloseHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&_lseek) __fileXio_fdman_ops_dir.lseek = __fileXioLseekDirHelper;
    // cppcheck-suppress knownConditionTrueFalse
    if (&getdents) __fileXio_fdman_ops_dir.dread = __fileXioDreadHelper;
}
#endif

#ifdef F__ps2sdk_fileXio_init_deinit
/* Backup pointer functions to restore after exit fileXio */
static _libcglue_fdman_path_ops_t *_backup_libcglue_fdman_path_ops;

void _ps2sdk_fileXio_init()
{
    _backup_libcglue_fdman_path_ops = _libcglue_fdman_path_ops;
    _libcglue_fdman_path_ops = &__fileXio_fdman_path_ops;
}

void _ps2sdk_fileXio_deinit()
{
    _libcglue_fdman_path_ops = _backup_libcglue_fdman_path_ops;
}
#endif
