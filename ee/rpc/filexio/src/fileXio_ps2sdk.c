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
#include <errno.h>
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include "iox_stat.h"

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

#ifdef F___fileXioDreadHelper
int __fileXioDreadHelper(int fd, struct dirent *dir) {
	int rv;
	iox_dirent_t ioxdir;

	// Took from iox_dirent_t
	#define __MAXNAMLEN 256

	rv = fileXioDread(fd, &ioxdir);
	if (rv < 0) {
		errno = ENOENT;
		return -1;
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
int __fileXioDreadHelper(int fd, struct dirent *dir);
#endif

#ifdef F__ps2sdk_fileXio_init_deinit
/* Backup pointer functions to restore after exit fileXio */
static int (*_backup_ps2sdk_close)(int);
static int (*_backup_ps2sdk_open)(const char*, int, ...);
static int (*_backup_ps2sdk_read)(int, void*, int);
static int (*_backup_ps2sdk_lseek)(int, int, int);
static int64_t (*_backup_ps2sdk_lseek64)(int, int64_t, int);
static int (*_backup_ps2sdk_write)(int, const void*, int);
static int (*_backup_ps2sdk_ioctl)(int, int, void*);
static int (*_backup_ps2sdk_remove)(const char*);
static int (*_backup_ps2sdk_rename)(const char*, const char*);
static int (*_backup_ps2sdk_mkdir)(const char*, int);
static int (*_backup_ps2sdk_rmdir)(const char*);

static int (*_backup_ps2sdk_stat)(const char *path, struct stat *buf);
static int (*_backup_ps2sdk_readlink)(const char *path, char *buf, size_t bufsiz);
static int (*_backup_ps2sdk_symlink)(const char *target, const char *linkpath);

static int (*_backup_ps2sdk_dopen)(const char *path);
static int (*_backup_ps2sdk_dread)(int fd, struct dirent *dir);
static int (*_backup_ps2sdk_dclose)(int fd);

void _ps2sdk_fileXio_init() {
    _backup_ps2sdk_close = _ps2sdk_close;
    _ps2sdk_close = fileXioClose;
    _backup_ps2sdk_open = _ps2sdk_open;
    _ps2sdk_open = fileXioOpen;
    _backup_ps2sdk_read = _ps2sdk_read;
    _ps2sdk_read = fileXioRead;
    _backup_ps2sdk_lseek = _ps2sdk_lseek;
    _ps2sdk_lseek = fileXioLseek;
    _backup_ps2sdk_lseek64 = _ps2sdk_lseek64;
    _ps2sdk_lseek64 = fileXioLseek64;
    _backup_ps2sdk_write = _ps2sdk_write;
    _ps2sdk_write = fileXioWrite;
    _backup_ps2sdk_ioctl = _ps2sdk_ioctl;
    _ps2sdk_ioctl = fileXioIoctl;
    _backup_ps2sdk_remove = _ps2sdk_remove;
    _ps2sdk_remove = fileXioRemove;
    _backup_ps2sdk_rename = _ps2sdk_rename;
    _ps2sdk_rename = fileXioRename;
    _backup_ps2sdk_mkdir = _ps2sdk_mkdir;
    _ps2sdk_mkdir = fileXioMkdir;
    _backup_ps2sdk_rmdir = _ps2sdk_rmdir;
    _ps2sdk_rmdir = fileXioRmdir;
    _backup_ps2sdk_stat = _ps2sdk_stat;
    _ps2sdk_stat = __fileXioGetstatHelper;
    _backup_ps2sdk_readlink = _ps2sdk_readlink;
    _ps2sdk_readlink = fileXioReadlink;
    _backup_ps2sdk_symlink = _ps2sdk_symlink;
    _ps2sdk_symlink = fileXioSymlink;
    _backup_ps2sdk_dopen = _ps2sdk_dopen;
    _ps2sdk_dopen = fileXioDopen;
    _backup_ps2sdk_dread = _ps2sdk_dread;
    _ps2sdk_dread = __fileXioDreadHelper;
    _backup_ps2sdk_dclose = _ps2sdk_dclose;
    _ps2sdk_dclose = fileXioDclose;
}

void _ps2sdk_fileXio_deinit() {
    _ps2sdk_close = _backup_ps2sdk_close;
    _ps2sdk_open = _backup_ps2sdk_open;
    _ps2sdk_read = _backup_ps2sdk_read;
    _ps2sdk_lseek = _backup_ps2sdk_lseek;
    _ps2sdk_lseek64 = _backup_ps2sdk_lseek64;
    _ps2sdk_write = _backup_ps2sdk_write;
    _ps2sdk_ioctl = _backup_ps2sdk_ioctl;
    _ps2sdk_remove = _backup_ps2sdk_remove;
    _ps2sdk_rename = _backup_ps2sdk_rename;
    _ps2sdk_mkdir = _backup_ps2sdk_mkdir;
    _ps2sdk_rmdir = _backup_ps2sdk_rmdir;
    _ps2sdk_stat = _backup_ps2sdk_stat;
    _ps2sdk_readlink = _backup_ps2sdk_readlink;
    _ps2sdk_symlink = _backup_ps2sdk_symlink;
    _ps2sdk_dopen = _backup_ps2sdk_dopen;
    _ps2sdk_dread = _backup_ps2sdk_dread;
    _ps2sdk_dclose = _backup_ps2sdk_dclose;
}
#endif