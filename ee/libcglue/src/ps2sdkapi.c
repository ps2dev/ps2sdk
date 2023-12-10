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
 * fdman.c - Manager for fd.
 */

#include <ps2sdkapi.h>
#include <string.h>
#include <errno.h>
#define NEWLIB_PORT_AWARE
#include <fileio.h>
#include "iox_stat.h"

/** Inter-library helpers, default value */
#ifdef F__ps2sdk_close
int (*_ps2sdk_close)(int) = NULL;
#endif

#ifdef F__ps2sdk_open
int (*_ps2sdk_open)(const char*, int, ...) = NULL;
#endif

#ifdef F__ps2sdk_read
int (*_ps2sdk_read)(int, void*, int) = NULL;
#endif

#ifdef F__ps2sdk_lseek
int (*_ps2sdk_lseek)(int, int, int) = NULL;
#endif

#ifdef F__ps2sdk_lseek64
int64_t (*_ps2sdk_lseek64)(int, int64_t, int) = NULL;
#endif

#ifdef F__ps2sdk_write
int (*_ps2sdk_write)(int, const void*, int) = NULL;
#endif

#ifdef F__ps2sdk_ioctl
int (*_ps2sdk_ioctl)(int, int, void*) = NULL;
#endif

#ifdef F__ps2sdk_remove
int (*_ps2sdk_remove)(const char*) = NULL;
#endif

#ifdef F__ps2sdk_rename
int (*_ps2sdk_rename)(const char*, const char*) = NULL;
#endif

#ifdef F__ps2sdk_mkdir
int (*_ps2sdk_mkdir)(const char*, int) = NULL;
#endif

#ifdef F__ps2sdk_rmdir
int (*_ps2sdk_rmdir)(const char*) = NULL;
#endif

#ifdef F__ps2sdk_stat
int (*_ps2sdk_stat)(const char *path, struct stat *buf) = NULL;
#endif

#ifdef F__ps2sdk_readlink
int (*_ps2sdk_readlink)(const char *path, char *buf, size_t bufsiz) = NULL;
#endif

#ifdef F__ps2sdk_symlink
int (*_ps2sdk_symlink)(const char *target, const char *linkpath) = NULL;
#endif

#ifdef F__ps2sdk_dopen
int (*_ps2sdk_dopen)(const char *path) = NULL;
#endif

#ifdef F__ps2sdk_dread
int (*_ps2sdk_dread)(int fd, struct dirent *dir) = NULL;
#endif

#ifdef F__ps2sdk_dclose
int (*_ps2sdk_dclose)(int fd) = NULL;
#endif

/** Setting default weak fio functions */
#ifdef F__set_ps2sdk_close
__attribute__((weak))
void _set_ps2sdk_close() {
    _ps2sdk_close = fioClose;
}
#endif

#ifdef F___fioOpenHelper
int __fioOpenHelper(const char* path, int flags, ...) {
  return fioOpen(path, flags);
}
#else
int __fioOpenHelper(const char* path, int flags, ...);
#endif

#ifdef F__set_ps2sdk_open
__attribute__((weak))
void _set_ps2sdk_open() {
    _ps2sdk_open = __fioOpenHelper;
}
#endif

#ifdef F__set_ps2sdk_read
__attribute__((weak))
void _set_ps2sdk_read() {
    _ps2sdk_read = fioRead;
}
#endif

#ifdef F__set_ps2sdk_lseek
__attribute__((weak))
void _set_ps2sdk_lseek() {
    _ps2sdk_lseek = fioLseek;
}
#endif

#ifdef F___fioLseek64Helper
off64_t __fioLseek64Helper(int fd, off64_t offset, int whence)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#else
off64_t __fioLseek64Helper(int fd, off64_t offset, int whence);
#endif

#ifdef F__set_ps2sdk_lseek64
__attribute__((weak))
void _set_ps2sdk_lseek64() {
    _ps2sdk_lseek64 = __fioLseek64Helper;
}
#endif

#ifdef F__set_ps2sdk_write
__attribute__((weak))
void _set_ps2sdk_write() {
    _ps2sdk_write = fioWrite;
}
#endif

#ifdef F__set_ps2sdk_ioctl
__attribute__((weak))
void _set_ps2sdk_ioctl() {
    _ps2sdk_ioctl = fioIoctl;
}
#endif

#ifdef F__set_ps2sdk_remove
__attribute__((weak))
void _set_ps2sdk_remove() {
    _ps2sdk_remove = fioRemove;
}
#endif

#ifdef F___fioRenameHelper
int __fioRenameHelper(const char *old, const char *new) {
	errno = ENOSYS;
	return -1; /* not supported */
}
#else
int __fioRenameHelper(const char *old, const char *new);
#endif

#ifdef F__set_ps2sdk_rename
__attribute__((weak))
void _set_ps2sdk_rename() {
    _ps2sdk_rename = __fioRenameHelper;
}
#endif

#ifdef F___fioMkdirHelper
int __fioMkdirHelper(const char *path, int mode) {
  // Old fio mkdir has no mode argument
	(void)mode;

  return fioMkdir(path);
}
#else
int __fioMkdirHelper(const char *path, int mode);
#endif

#ifdef F__set_ps2sdk_mkdir
__attribute__((weak))
void _set_ps2sdk_mkdir() {
    _ps2sdk_mkdir = __fioMkdirHelper;
}
#endif

#ifdef F__set_ps2sdk_rmdir
__attribute__((weak))
void _set_ps2sdk_rmdir() {
    _ps2sdk_rmdir = fioRmdir;
}
#endif

#ifdef F___fioGetstatHelper
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

static mode_t io_to_posix_mode(unsigned int ps2mode)
{
        mode_t posixmode = 0;
        if (ps2mode & FIO_SO_IFREG) posixmode |= S_IFREG;
        if (ps2mode & FIO_SO_IFDIR) posixmode |= S_IFDIR;
        if (ps2mode & FIO_SO_IROTH) posixmode |= S_IRUSR|S_IRGRP|S_IROTH;
        if (ps2mode & FIO_SO_IWOTH) posixmode |= S_IWUSR|S_IWGRP|S_IWOTH;
        if (ps2mode & FIO_SO_IXOTH) posixmode |= S_IXUSR|S_IXGRP|S_IXOTH;
        return posixmode;
}

static void __fill_stat(struct stat *stat, const io_stat_t *fiostat)
{
        stat->st_dev = 0;
        stat->st_ino = 0;
        stat->st_mode = io_to_posix_mode(fiostat->mode);
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

int __fioGetstatHelper(const char *path, struct stat *buf) {
        io_stat_t fiostat;

        if (fioGetstat(path, &fiostat) < 0) {
			errno = ENOENT;
			return -1;
        }

        __fill_stat(buf, &fiostat);

        return 0;
}
#else
int __fioGetstatHelper(const char *path, struct stat *buf);
#endif

#ifdef F__set_ps2sdk_stat
__attribute__((weak))
void _set_ps2sdk_stat() {
    _ps2sdk_stat = __fioGetstatHelper;
}
#endif

#ifdef F___fioReadlinkHelper
ssize_t __fioReadlinkHelper(const char *path, char *buf, size_t bufsiz)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#else
ssize_t __fioReadlinkHelper(const char *path, char *buf, size_t bufsiz);
#endif

#ifdef F__set_ps2sdk_readlink
__attribute__((weak))
void _set_ps2sdk_readlink() {
    _ps2sdk_readlink = __fioReadlinkHelper;
}
#endif

#ifdef F___fioSymlinkHelper
int __fioSymlinkHelper(const char *target, const char *linkpath)
{
	errno = ENOSYS;
	return -1; /* not supported */
}
#else
int __fioSymlinkHelper(const char *target, const char *linkpath);
#endif

#ifdef F__set_ps2sdk_symlink
__attribute__((weak))
void _set_ps2sdk_symlink() {
    _ps2sdk_symlink = __fioSymlinkHelper;
}
#endif

#ifdef F__set_ps2sdk_dopen
__attribute__((weak))
void _set_ps2sdk_dopen() {
    _ps2sdk_dopen = fioDopen;
}
#endif

#ifdef F___fioDreadHelper
int __fioDreadHelper(int fd, struct dirent *dir) {
	int rv;
	io_dirent_t iodir;

	// Took from io_dirent_t
	#define __MAXNAMLEN 256

	rv = fioDread(fd, &iodir);
	if (rv < 0) {
		errno = ENOENT;
		return -1;
	}

	dir->d_fileno = rv; // TODO: This number should be in theory a unique number per file
	strncpy(dir->d_name, iodir.name, __MAXNAMLEN);
	dir->d_name[__MAXNAMLEN - 1] = 0;
	dir->d_reclen = 0;
	switch (iodir.stat.mode & FIO_SO_IFMT) {
		case FIO_SO_IFLNK: dir->d_type = DT_LNK;     break;
		case FIO_SO_IFDIR: dir->d_type = DT_DIR;     break;
		case FIO_SO_IFREG: dir->d_type = DT_REG;     break;
		default:          dir->d_type = DT_UNKNOWN; break;
	}


	return rv;
}
#else
int __fioDreadHelper(int fd, struct dirent *dir);
#endif

#ifdef F__set_ps2sdk_dread
__attribute__((weak))
void _set_ps2sdk_dread() {
    _ps2sdk_dread = __fioDreadHelper;
}
#endif

#ifdef F__set_ps2sdk_dclose
__attribute__((weak))
void _set_ps2sdk_dclose() {
    _ps2sdk_dclose = fioDclose;
}
#endif


#ifdef F__predefined_ps2sdk_close
__attribute__((weak))
void _predefined_ps2sdk_close() {}
#endif

#ifdef F__glue_ps2sdk_close
void _predefined_ps2sdk_close();
void _set_ps2sdk_close();

void _glue_ps2sdk_close() {
    // Check if predefined values are set
    _predefined_ps2sdk_close();

    if (_ps2sdk_close == NULL) _set_ps2sdk_close();
}
#endif

#ifdef F__predefined_ps2sdk_open
__attribute__((weak))
void _predefined_ps2sdk_open() {}
#endif

#ifdef F__glue_ps2sdk_open
void _predefined_ps2sdk_open();
void _set_ps2sdk_open();

void _glue_ps2sdk_open() {
    // Check if predefined values are set
    _predefined_ps2sdk_open();

    if (_ps2sdk_open == NULL) _set_ps2sdk_open();
}
#endif

#ifdef F__predefined_ps2sdk_read
__attribute__((weak))
void _predefined_ps2sdk_read() {}
#endif

#ifdef F__glue_ps2sdk_read
void _predefined_ps2sdk_read();
void _set_ps2sdk_read();

void _glue_ps2sdk_read() {
    // Check if predefined values are set
    _predefined_ps2sdk_read();

    if (_ps2sdk_read == NULL) _set_ps2sdk_read();
}
#endif

#ifdef F__predefined_ps2sdk_lseek
__attribute__((weak))
void _predefined_ps2sdk_lseek() {}
#endif

#ifdef F__glue_ps2sdk_lseek
void _predefined_ps2sdk_lseek();
void _set_ps2sdk_lseek();

void _glue_ps2sdk_lseek() {
    // Check if predefined values are set
    _predefined_ps2sdk_lseek();

    if (_ps2sdk_lseek == NULL) _set_ps2sdk_lseek();
}
#endif

#ifdef F__predefined_ps2sdk_lseek64
__attribute__((weak))
void _predefined_ps2sdk_lseek64() {}
#endif

#ifdef F__glue_ps2sdk_lseek64
void _predefined_ps2sdk_lseek64();
void _set_ps2sdk_lseek64();

void _glue_ps2sdk_lseek64() {
    // Check if predefined values are set
    _predefined_ps2sdk_lseek64();

    if (_ps2sdk_lseek64 == NULL) _set_ps2sdk_lseek64();
}
#endif

#ifdef F__predefined_ps2sdk_write
__attribute__((weak))
void _predefined_ps2sdk_write() {}
#endif

#ifdef F__glue_ps2sdk_write
void _predefined_ps2sdk_write();
void _set_ps2sdk_write();

void _glue_ps2sdk_write() {
    // Check if predefined values are set
    _predefined_ps2sdk_write();

    if (_ps2sdk_write == NULL) _set_ps2sdk_write();
}
#endif

#ifdef F__predefined_ps2sdk_ioctl
__attribute__((weak))
void _predefined_ps2sdk_ioctl() {}
#endif

#ifdef F__glue_ps2sdk_ioctl
void _predefined_ps2sdk_ioctl();
void _set_ps2sdk_ioctl();

void _glue_ps2sdk_ioctl() {
    // Check if predefined values are set
    _predefined_ps2sdk_ioctl();

    if (_ps2sdk_ioctl == NULL) _set_ps2sdk_ioctl();
}
#endif

#ifdef F__predefined_ps2sdk_remove
__attribute__((weak))
void _predefined_ps2sdk_remove() {}
#endif

#ifdef F__glue_ps2sdk_remove
void _predefined_ps2sdk_remove();
void _set_ps2sdk_remove();

void _glue_ps2sdk_remove() {
    // Check if predefined values are set
    _predefined_ps2sdk_remove();

    if (_ps2sdk_remove == NULL) _set_ps2sdk_remove();
}
#endif

#ifdef F__predefined_ps2sdk_rename
__attribute__((weak))
void _predefined_ps2sdk_rename() {}
#endif

#ifdef F__glue_ps2sdk_rename
void _predefined_ps2sdk_rename();
void _set_ps2sdk_rename();

void _glue_ps2sdk_rename() {
    // Check if predefined values are set
    _predefined_ps2sdk_rename();

    if (_ps2sdk_rename == NULL) _set_ps2sdk_rename();
}
#endif

#ifdef F__predefined_ps2sdk_mkdir
__attribute__((weak))
void _predefined_ps2sdk_mkdir() {}
#endif

#ifdef F__glue_ps2sdk_mkdir
void _predefined_ps2sdk_mkdir();
void _set_ps2sdk_mkdir();

void _glue_ps2sdk_mkdir() {
    // Check if predefined values are set
    _predefined_ps2sdk_mkdir();

    if (_ps2sdk_mkdir == NULL) _set_ps2sdk_mkdir();
}
#endif

#ifdef F__predefined_ps2sdk_rmdir
__attribute__((weak))
void _predefined_ps2sdk_rmdir() {}
#endif

#ifdef F__glue_ps2sdk_rmdir
void _predefined_ps2sdk_rmdir();
void _set_ps2sdk_rmdir();

void _glue_ps2sdk_rmdir() {
    // Check if predefined values are set
    _predefined_ps2sdk_rmdir();

    if (_ps2sdk_rmdir == NULL) _set_ps2sdk_rmdir();
}
#endif

#ifdef F__predefined_ps2sdk_stat
__attribute__((weak))
void _predefined_ps2sdk_stat() {}
#endif

#ifdef F__glue_ps2sdk_stat
void _predefined_ps2sdk_stat();
void _set_ps2sdk_stat();

void _glue_ps2sdk_stat() {
    // Check if predefined values are set
    _predefined_ps2sdk_stat();

    if (_ps2sdk_stat == NULL) _set_ps2sdk_stat();
}
#endif

#ifdef F__predefined_ps2sdk_readlink
__attribute__((weak))
void _predefined_ps2sdk_readlink() {}
#endif

#ifdef F__glue_ps2sdk_readlink
void _predefined_ps2sdk_readlink();
void _set_ps2sdk_readlink();

void _glue_ps2sdk_readlink() {
    // Check if predefined values are set
    _predefined_ps2sdk_readlink();

    if (_ps2sdk_readlink == NULL) _set_ps2sdk_readlink();
}
#endif

#ifdef F__predefined_ps2sdk_symlink
__attribute__((weak))
void _predefined_ps2sdk_symlink() {}
#endif

#ifdef F__glue_ps2sdk_symlink
void _predefined_ps2sdk_symlink();
void _set_ps2sdk_symlink();

void _glue_ps2sdk_symlink() {
    // Check if predefined values are set
    _predefined_ps2sdk_symlink();

    if (_ps2sdk_symlink == NULL) _set_ps2sdk_symlink();
}
#endif

#ifdef F__predefined_ps2sdk_dopen
__attribute__((weak))
void _predefined_ps2sdk_dopen() {}
#endif

#ifdef F__glue_ps2sdk_dopen
void _predefined_ps2sdk_dopen();
void _set_ps2sdk_dopen();

void _glue_ps2sdk_dopen() {
    // Check if predefined values are set
    _predefined_ps2sdk_dopen();

    if (_ps2sdk_dopen == NULL) _set_ps2sdk_dopen();
}
#endif

#ifdef F__predefined_ps2sdk_dread
__attribute__((weak))
void _predefined_ps2sdk_dread() {}
#endif

#ifdef F__glue_ps2sdk_dread
void _predefined_ps2sdk_dread();
void _set_ps2sdk_dread();

void _glue_ps2sdk_dread() {
    // Check if predefined values are set
    _predefined_ps2sdk_dread();

    if (_ps2sdk_dread == NULL) _set_ps2sdk_dread();
}
#endif

#ifdef F__predefined_ps2sdk_dclose
__attribute__((weak))
void _predefined_ps2sdk_dclose() {}
#endif

#ifdef F__glue_ps2sdk_dclose
void _predefined_ps2sdk_dclose();
void _set_ps2sdk_dclose();

void _glue_ps2sdk_dclose() {
    // Check if predefined values are set
    _predefined_ps2sdk_dclose();

    if (_ps2sdk_dclose == NULL) _set_ps2sdk_dclose();
}
#endif
