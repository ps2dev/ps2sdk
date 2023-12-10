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

/** Setting fileXio functions */
#ifdef F__set_ps2sdk_close
uint8_t __ps2sdk_fileXio_close = 0;
void _set_ps2sdk_close() {
    _ps2sdk_close = fileXioClose;
}
#endif

#ifdef F__set_ps2sdk_open
uint8_t __ps2sdk_fileXio_open = 0;
void _set_ps2sdk_open() {
    _ps2sdk_open = fileXioOpen;
}
#endif

#ifdef F__set_ps2sdk_read
uint8_t __ps2sdk_fileXio_read = 0;
void _set_ps2sdk_read() {
    _ps2sdk_read = fileXioRead;
}
#endif

#ifdef F__set_ps2sdk_lseek
uint8_t __ps2sdk_fileXio_lseek = 0;
void _set_ps2sdk_lseek() {
    _ps2sdk_lseek = fileXioLseek;
}
#endif

#ifdef F__set_ps2sdk_lseek64
uint8_t __ps2sdk_fileXio_lseek64 = 0;
void _set_ps2sdk_lseek64() {
    _ps2sdk_lseek64 = fileXioLseek64;
}
#endif

#ifdef F__set_ps2sdk_write
uint8_t __ps2sdk_fileXio_write = 0;
void _set_ps2sdk_write() {
    _ps2sdk_write = fileXioWrite;
}
#endif

#ifdef F__set_ps2sdk_ioctl
uint8_t __ps2sdk_fileXio_ioctl = 0;
void _set_ps2sdk_ioctl() {
    _ps2sdk_ioctl = fileXioIoctl;
}
#endif

#ifdef F__set_ps2sdk_remove
uint8_t __ps2sdk_fileXio_remove = 0;
void _set_ps2sdk_remove() {
    _ps2sdk_remove = fileXioRemove;
}
#endif

#ifdef F__set_ps2sdk_rename
uint8_t __ps2sdk_fileXio_rename = 0;
void _set_ps2sdk_rename() {
    _ps2sdk_rename = fileXioRename;
}
#endif

#ifdef F__set_ps2sdk_mkdir
uint8_t __ps2sdk_fileXio_mkdir = 0;
void _set_ps2sdk_mkdir() {
    _ps2sdk_mkdir = fileXioMkdir;
}
#endif

#ifdef F__set_ps2sdk_rmdir
uint8_t __ps2sdk_fileXio_rmdir = 0;
void _set_ps2sdk_rmdir() {
    _ps2sdk_rmdir = fileXioRmdir;
}
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

#ifdef F__set_ps2sdk_stat
uint8_t __ps2sdk_fileXio_stat = 0;
void _set_ps2sdk_stat() {
    _ps2sdk_stat = __fileXioGetstatHelper;
}
#endif

#ifdef F__set_ps2sdk_readlink
uint8_t __ps2sdk_fileXio_readlink = 0;
void _set_ps2sdk_readlink() {
    _ps2sdk_readlink = fileXioReadlink;
}
#endif

#ifdef F__set_ps2sdk_symlink
uint8_t __ps2sdk_fileXio_symlink = 0;
void _set_ps2sdk_symlink() {
    _ps2sdk_symlink = fileXioSymlink;
}
#endif

#ifdef F__set_ps2sdk_dopen
uint8_t __ps2sdk_fileXio_dopen = 0;
void _set_ps2sdk_dopen() {
    _ps2sdk_dopen = fileXioDopen;
}
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

#ifdef F__set_ps2sdk_dread
uint8_t __ps2sdk_fileXio_dread = 0;
void _set_ps2sdk_dread() {
    _ps2sdk_dread = __fileXioDreadHelper;
}
#endif

#ifdef F__set_ps2sdk_dclose
uint8_t __ps2sdk_fileXio_dclose = 0;
void _set_ps2sdk_dclose() {
    _ps2sdk_dclose = fileXioDclose;
}
#endif