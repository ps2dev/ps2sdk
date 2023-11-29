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
int (*_ps2sdk_close)(int) = fioClose;
#endif

#ifdef F__ps2sdk_open
int (*_ps2sdk_open)(const char*, int, ...) = (void *)fioOpen;
#endif

#ifdef F__ps2sdk_read
int (*_ps2sdk_read)(int, void*, int) = fioRead;
#endif

#ifdef F__ps2sdk_lseek
int (*_ps2sdk_lseek)(int, int, int) = fioLseek;
#endif

#ifdef F__ps2sdk_lseek64
static off64_t _default_lseek64(int fd, off64_t offset, int whence)
{
	errno = ENOSYS;
	return -1; /* not supported */
}

int64_t (*_ps2sdk_lseek64)(int, int64_t, int) = _default_lseek64;
#endif

#ifdef F__ps2sdk_write
int (*_ps2sdk_write)(int, const void*, int) = fioWrite;
#endif

#ifdef F__ps2sdk_ioctl
int (*_ps2sdk_ioctl)(int, int, void*) = fioIoctl;
#endif

#ifdef F__ps2sdk_remove
int (*_ps2sdk_remove)(const char*) = fioRemove;
#endif

#ifdef F__ps2sdk_rename
static int fioRename(const char *old, const char *new) {
	errno = ENOSYS;
	return -1; /* not supported */
}

int (*_ps2sdk_rename)(const char*, const char*) = fioRename;
#endif

#ifdef F__ps2sdk_mkdir
static int fioMkdirHelper(const char *path, int mode) {
  // Old fio mkdir has no mode argument
	(void)mode;

  return fioMkdir(path);
}

int (*_ps2sdk_mkdir)(const char*, int) = fioMkdirHelper;
#endif

#ifdef F__ps2sdk_rmdir
int (*_ps2sdk_rmdir)(const char*) = fioRmdir;
#endif

#ifdef F__ps2sdk_stat
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

static int fioGetstatHelper(const char *path, struct stat *buf) {
        io_stat_t fiostat;

        if (fioGetstat(path, &fiostat) < 0) {
			errno = ENOENT;
			return -1;
        }

        __fill_stat(buf, &fiostat);

        return 0;
}

int (*_ps2sdk_stat)(const char *path, struct stat *buf) = fioGetstatHelper;
#endif

#ifdef F__ps2sdk_readlink
static ssize_t _default_readlink(const char *path, char *buf, size_t bufsiz)
{
	errno = ENOSYS;
	return -1; /* not supported */
}

int (*_ps2sdk_readlink)(const char *path, char *buf, size_t bufsiz) = _default_readlink;
#endif

#ifdef F__ps2sdk_symlink
static int _default_symlink(const char *target, const char *linkpath)
{
	errno = ENOSYS;
	return -1; /* not supported */
}

int (*_ps2sdk_symlink)(const char *target, const char *linkpath) = _default_symlink;
#endif

#ifdef F__ps2sdk_dopen
int (*_ps2sdk_dopen)(const char *path) = fioDopen;
#endif

#ifdef F__ps2sdk_dread
static int fioDreadHelper(int fd, struct dirent *dir) {
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

int (*_ps2sdk_dread)(int fd, struct dirent *dir) = fioDreadHelper;
#endif

#ifdef F__ps2sdk_dclose
int (*_ps2sdk_dclose)(int fd) = fioDclose;
#endif
