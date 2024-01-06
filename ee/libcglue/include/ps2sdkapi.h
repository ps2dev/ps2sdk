/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2SDKAPI_H__
#define __PS2SDKAPI_H__

#include <dirent.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <timer.h>
#include <errno.h>

/** Inter-library helpers */
typedef int (*_libcglue_fdman_getfd_cb_t)(void *userdata);
typedef char *(*_libcglue_fdman_getfilename_cb_t)(void *userdata);
typedef int (*_libcglue_fdman_close_cb_t)(void *userdata);
typedef int (*_libcglue_fdman_read_cb_t)(void *userdata, void *buf, int nbytes);
typedef int (*_libcglue_fdman_lseek_cb_t)(void *userdata, int offset, int whence);
typedef int64_t (*_libcglue_fdman_lseek64_cb_t)(void *userdata, int64_t offset, int whence);
typedef int (*_libcglue_fdman_write_cb_t)(void *userdata, const void *buf, int nbytes);
typedef int (*_libcglue_fdman_ioctl_cb_t)(void *userdata, int request, void *data);
typedef int (*_libcglue_fdman_dread_cb_t)(void *userdata, struct dirent *dir);

typedef struct _libcglue_fdman_fd_ops_
{
	_libcglue_fdman_getfd_cb_t getfd;
	_libcglue_fdman_getfilename_cb_t getfilename;
	_libcglue_fdman_close_cb_t close;
	_libcglue_fdman_read_cb_t read;
	_libcglue_fdman_lseek_cb_t lseek;
	_libcglue_fdman_lseek64_cb_t lseek64;
	_libcglue_fdman_write_cb_t write;
	_libcglue_fdman_ioctl_cb_t ioctl;
	_libcglue_fdman_dread_cb_t dread;
} _libcglue_fdman_fd_ops_t;

typedef struct _libcglue_fdman_fd_info_
{
	void *userdata;
	_libcglue_fdman_fd_ops_t *ops;
} _libcglue_fdman_fd_info_t;

typedef int (*_libcglue_fdman_open_cb_t)(_libcglue_fdman_fd_info_t *info, const char *buf, int flags, mode_t mode);
typedef int (*_libcglue_fdman_remove_cb_t)(const char *path);
typedef int (*_libcglue_fdman_rename_cb_t)(const char *old, const char *new_);
typedef int (*_libcglue_fdman_mkdir_cb_t)(const char *path, int mode);
typedef int (*_libcglue_fdman_rmdir_cb_t)(const char *path);
typedef int (*_libcglue_fdman_stat_cb_t)(const char *path, struct stat *buf);
typedef int (*_libcglue_fdman_readlink_cb_t)(const char *path, char *buf, size_t bufsiz);
typedef int (*_libcglue_fdman_symlink_cb_t)(const char *target, const char *linkpath);

typedef struct _libcglue_fdman_path_ops_
{
	_libcglue_fdman_open_cb_t open;
	_libcglue_fdman_remove_cb_t remove;
	_libcglue_fdman_rename_cb_t rename;
	_libcglue_fdman_mkdir_cb_t mkdir;
	_libcglue_fdman_rmdir_cb_t rmdir;
	_libcglue_fdman_stat_cb_t stat;
	_libcglue_fdman_readlink_cb_t readlink;
	_libcglue_fdman_symlink_cb_t symlink;
} _libcglue_fdman_path_ops_t;

extern _libcglue_fdman_path_ops_t *_libcglue_fdman_path_ops;

/* Functions from cwd.c */
extern char __cwd[MAXNAMLEN + 1];
int __path_absolute(const char *in, char *out, int len);

#define PS2_CLOCKS_PER_SEC kBUSCLKBY256 // 576.000
#define PS2_CLOCKS_PER_MSEC (PS2_CLOCKS_PER_SEC / 1000) // 576

/* Disable the auto start of pthread on init for reducing binary size if not used. */
#define PS2_DISABLE_AUTOSTART_PTHREAD() \
	void __libpthreadglue_init() {} \
    void __libpthreadglue_deinit() {}

typedef uint64_t ps2_clock_t;
static inline ps2_clock_t ps2_clock(void) {
    // DEPRECATED VERSION USE INSTEAD GetTimerSystemTime
    return (ps2_clock_t)(GetTimerSystemTime() >> 8);
}

extern s64 _ps2sdk_rtc_offset_from_busclk;
extern void _libcglue_rtc_update();

// The newlib port does not support 64bit
// this should have been defined in unistd.h
typedef int64_t off64_t;
off64_t lseek64(int fd, off64_t offset, int whence);

// Functions to be used related to timezone
extern void _libcglue_timezone_update();

void ps2sdk_setTimezone(int timezone);
void ps2sdk_setDaylightSaving(int daylightSaving);

_libcglue_fdman_fd_info_t *libcglue_get_fd_info(int fd);
int __libcglue_init_stdio(_libcglue_fdman_fd_info_t *info, int fd);

/* The fd we provide to final user aren't actually the same than IOP's fd
* so this function allow you to get actual IOP's fd from public fd
*/
static inline int ps2sdk_get_iop_fd(int fd)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->getfd == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->getfd(fdinfo->userdata);
}

static inline char *ps2sdk_get_iop_filename(int fd)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return NULL;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->getfilename == NULL)
	{
		return NULL;
	}
	return fdinfo->ops->getfilename(fdinfo->userdata);
}

static inline int _ps2sdk_close(int fd)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->close == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->close(fdinfo->userdata);
}

static inline int _ps2sdk_dclose(int fd)
{
	return _ps2sdk_close(fd);
}

static inline int _ps2sdk_read(int fd, void *buf, int nbytes)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->read == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->read(fdinfo->userdata, buf, nbytes);
}

static inline int _ps2sdk_lseek(int fd, int offset, int whence)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->lseek == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->lseek(fdinfo->userdata, offset, whence);
}

static inline int64_t _ps2sdk_lseek64(int fd, int64_t offset, int whence)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->lseek64 == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->lseek64(fdinfo->userdata, offset, whence);
}

static inline int _ps2sdk_write(int fd, const void *buf, int nbytes)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->write == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->write(fdinfo->userdata, buf, nbytes);
}

static inline int _ps2sdk_ioctl(int fd, int request, void *data)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->ioctl == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->ioctl(fdinfo->userdata, request, data);
}

static inline int _ps2sdk_dread(int fd, struct dirent *dir)
{
	_libcglue_fdman_fd_info_t *fdinfo;
	fdinfo = libcglue_get_fd_info(fd);
	if (fdinfo == NULL)
	{
		return -EBADF;
	}
	if (fdinfo->ops == NULL || fdinfo->ops->dread == NULL)
	{
		return -ENOSYS;
	}
	return fdinfo->ops->dread(fdinfo->userdata, dir);
}

#endif /* __PS2SDKAPI_H__ */
