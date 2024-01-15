/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Definitions and imports for iomanX.
 */

#ifndef __IOMANX_H__
#define __IOMANX_H__

#include <types.h>
#ifdef _IOP
#include <irx.h>
#endif
#include <stdarg.h>
#include <io_common.h>
#include <iox_stat.h>

#ifdef __cplusplus
extern "C" {
#endif

// Rename prototype names in order to avoid clashes with other standard I/O libraries.
#ifndef IOMANX_OLD_NAME_COMPATIBILITY
#ifdef _IOP
#define IOMANX_OLD_NAME_COMPATIBILITY 1
#else
#define IOMANX_OLD_NAME_COMPATIBILITY 0
#endif /* _IOP */
#endif /* IOMANX_OLD_NAME_COMPATIBILITY */

// Avoid renaming AddDrv and DelDrv to handle the case where the old ioman AddDrv/DelDrv is linked against.
#ifndef IOMANX_OLD_NAME_ADDDELDRV
#ifdef _IOP
#define IOMANX_OLD_NAME_ADDDELDRV 1
#else
#define IOMANX_OLD_NAME_ADDDELDRV 0
#endif /* _IOP */
#endif /* IOMANX_OLD_NAME_ADDDELDRV */

#if IOMANX_OLD_NAME_COMPATIBILITY
#if IOMANX_OLD_NAME_ADDDELDRV
#define iomanX_AddDrv AddDrv
#define iomanX_DelDrv DelDrv
#endif
#endif

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR	0x01
#define IOP_DT_CONS	0x02
#define IOP_DT_BLOCK	0x04
#define IOP_DT_RAW	0x08
#define IOP_DT_FS	0x10
#ifndef IOMAN_NO_EXTENDED
/** Supports calls after chstat().  */
#define IOP_DT_FSEXT	0x10000000	
#endif

/** File objects passed to driver operations.  */
typedef struct _iomanX_iop_file {
	/** File open mode.  */
	int	mode;		
	/** HW device unit number.  */
	int	unit;		
	/** Device driver.  */
	struct _iomanX_iop_device *device;
	/** The device driver can use this however it wants.  */
	void	*privdata;	
} iomanX_iop_file_t;

typedef struct _iomanX_iop_device {
	const char *name;
	unsigned int type;
	/** Not so sure about this one.  */
	unsigned int version;
	const char *desc;
	struct _iomanX_iop_device_ops *ops;
} iomanX_iop_device_t;

typedef struct _iomanX_iop_device_ops {
	int	(*init)(iomanX_iop_device_t *);
	int	(*deinit)(iomanX_iop_device_t *);
	int	(*format)(iomanX_iop_file_t *, const char *, const char *, void *, int);
	int	(*open)(iomanX_iop_file_t *, const char *, int, int);
	int	(*close)(iomanX_iop_file_t *);
	int	(*read)(iomanX_iop_file_t *, void *, int);
	int	(*write)(iomanX_iop_file_t *, void *, int);
	int	(*lseek)(iomanX_iop_file_t *, int, int);
	int	(*ioctl)(iomanX_iop_file_t *, int, void *);
	int	(*remove)(iomanX_iop_file_t *, const char *);
	int	(*mkdir)(iomanX_iop_file_t *, const char *, int);
	int	(*rmdir)(iomanX_iop_file_t *, const char *);
	int	(*dopen)(iomanX_iop_file_t *, const char *);
	int	(*dclose)(iomanX_iop_file_t *);
	int	(*dread)(iomanX_iop_file_t *, iox_dirent_t *);
	int	(*getstat)(iomanX_iop_file_t *, const char *, iox_stat_t *);
	int	(*chstat)(iomanX_iop_file_t *, const char *, iox_stat_t *, unsigned int);

#ifndef IOMAN_NO_EXTENDED
	/* Extended ops start here.  */
	int	(*rename)(iomanX_iop_file_t *, const char *, const char *);
	int	(*chdir)(iomanX_iop_file_t *, const char *);
	int	(*sync)(iomanX_iop_file_t *, const char *, int);
	int	(*mount)(iomanX_iop_file_t *, const char *, const char *, int, void *, int);
	int	(*umount)(iomanX_iop_file_t *, const char *);
	s64	(*lseek64)(iomanX_iop_file_t *, s64, int);
	int	(*devctl)(iomanX_iop_file_t *, const char *, int, void *, unsigned int, void *, unsigned int);
	int	(*symlink)(iomanX_iop_file_t *, const char *, const char *);
	int	(*readlink)(iomanX_iop_file_t *, const char *, char *, unsigned int);
	int	(*ioctl2)(iomanX_iop_file_t *, int, void *, unsigned int, void *, unsigned int);
#endif /* IOMAN_NO_EXTENDED */
} iomanX_iop_device_ops_t;

extern iomanX_iop_device_t **iomanX_GetDeviceList(void);

extern int iomanX_open(const char *name, int flags, ...);
extern int iomanX_close(int fd);
extern int iomanX_read(int fd, void *ptr, int size);
extern int iomanX_write(int fd, void *ptr, int size);
extern int iomanX_lseek(int fd, int offset, int mode);

extern int iomanX_ioctl(int fd, int cmd, void *param);
extern int iomanX_remove(const char *name);

extern int iomanX_mkdir(const char *path, int mode);
extern int iomanX_rmdir(const char *path);
extern int iomanX_dopen(const char *path);
extern int iomanX_dclose(int fd);
extern int iomanX_dread(int fd, iox_dirent_t *buf);

extern int iomanX_getstat(const char *name, iox_stat_t *stat);
extern int iomanX_chstat(const char *name, iox_stat_t *stat, unsigned int statmask);

/** This can take take more than one form.  */
extern int iomanX_format(const char *dev, const char *blockdev, void *arg, int arglen);

#ifndef IOMAN_NO_EXTENDED
/* The newer calls - these are NOT supported by the older IOMAN.  */
extern int iomanX_rename(const char *old, const char *new);
extern int iomanX_chdir(const char *name);
extern int iomanX_sync(const char *dev, int flag);
extern int iomanX_mount(const char *fsname, const char *devname, int flag, void *arg, int arglen);
extern int iomanX_umount(const char *fsname);
extern s64 iomanX_lseek64(int fd, s64 offset, int whence);
extern int iomanX_devctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int iomanX_symlink(const char *old, const char *new);
extern int iomanX_readlink(const char *path, char *buf, unsigned int buflen);
extern int iomanX_ioctl2(int fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
#endif /* IOMAN_NO_EXTENDED */

extern int iomanX_AddDrv(iomanX_iop_device_t *device);
extern int iomanX_DelDrv(const char *name);

extern void iomanX_StdioInit(int mode);

#ifdef _IOP
#define iomanX_IMPORTS_start DECLARE_IMPORT_TABLE(iomanx, 1, 1)
#define iomanX_IMPORTS_end END_IMPORT_TABLE

#define I_iomanX_GetDeviceList DECLARE_IMPORT(3, iomanX_GetDeviceList)
#define I_iomanX_open DECLARE_IMPORT(4, iomanX_open)
#define I_iomanX_close DECLARE_IMPORT(5, iomanX_close)
#define I_iomanX_read DECLARE_IMPORT(6, iomanX_read)
#define I_iomanX_write DECLARE_IMPORT(7, iomanX_write)
#define I_iomanX_lseek DECLARE_IMPORT(8, iomanX_lseek)
#define I_iomanX_ioctl DECLARE_IMPORT(9, iomanX_ioctl)
#define I_iomanX_remove DECLARE_IMPORT(10, iomanX_remove)
#define I_iomanX_mkdir DECLARE_IMPORT(11, iomanX_mkdir)
#define I_iomanX_rmdir DECLARE_IMPORT(12, iomanX_rmdir)
#define I_iomanX_dopen DECLARE_IMPORT(13, iomanX_dopen)
#define I_iomanX_dclose DECLARE_IMPORT(14, iomanX_dclose)
#define I_iomanX_dread DECLARE_IMPORT(15, iomanX_dread)
#define I_iomanX_getstat DECLARE_IMPORT(16, iomanX_getstat)
#define I_iomanX_chstat DECLARE_IMPORT(17, iomanX_chstat)
#define I_iomanX_format DECLARE_IMPORT(18, iomanX_format)
#define I_iomanX_rename DECLARE_IMPORT(25, iomanX_rename)
#define I_iomanX_chdir DECLARE_IMPORT(26, iomanX_chdir)
#define I_iomanX_sync DECLARE_IMPORT(27, iomanX_sync)
#define I_iomanX_mount DECLARE_IMPORT(28, iomanX_mount)
#define I_iomanX_umount DECLARE_IMPORT(29, iomanX_umount)
#define I_iomanX_lseek64 DECLARE_IMPORT(30, iomanX_lseek64)
#define I_iomanX_devctl DECLARE_IMPORT(31, iomanX_devctl)
#define I_iomanX_symlink DECLARE_IMPORT(32, iomanX_symlink)
#define I_iomanX_readlink DECLARE_IMPORT(33, iomanX_readlink)
#define I_iomanX_ioctl2 DECLARE_IMPORT(34, iomanX_ioctl2)
#if IOMANX_OLD_NAME_ADDDELDRV
#define I_AddDrv DECLARE_IMPORT(20, AddDrv)
#define I_DelDrv DECLARE_IMPORT(21, DelDrv)
#else
#define I_iomanX_AddDrv DECLARE_IMPORT(20, iomanX_AddDrv)
#define I_iomanX_DelDrv DECLARE_IMPORT(21, iomanX_DelDrv)
#endif
#define I_iomanX_StdioInit DECLARE_IMPORT(23, iomanX_StdioInit)
#endif

#if IOMANX_OLD_NAME_COMPATIBILITY

typedef iomanX_iop_file_t iop_file_t;
typedef iomanX_iop_device_t iop_device_t;
typedef iomanX_iop_device_ops_t iop_device_ops_t;

static inline iop_device_t **GetDeviceList(void)
{
	return iomanX_GetDeviceList();
}

static inline int open(const char *name, int flags, ...)
{
	va_list alist;
	int mode;

	va_start(alist, flags);
	mode = va_arg(alist, int);
	va_end(alist);

	return iomanX_open(name, flags, mode);
}
static inline int close(int fd)
{
	return iomanX_close(fd);
}
static inline int read(int fd, void *ptr, int size)
{
	return iomanX_read(fd, ptr, size);
}
static inline int write(int fd, void *ptr, int size)
{
	return iomanX_write(fd, ptr, size);
}
static inline int lseek(int fd, int offset, int mode)
{
	return iomanX_lseek(fd, offset, mode);
}

static inline int ioctl(int fd, int cmd, void *param)
{
	return iomanX_ioctl(fd, cmd, param);
}
static inline int remove(const char *name)
{
	return iomanX_remove(name);
}

static inline int mkdir(const char *path, int mode)
{
	return iomanX_mkdir(path, mode);
}
static inline int rmdir(const char *path)
{
	return iomanX_rmdir(path);
}
static inline int dopen(const char *path)
{
	return iomanX_dopen(path);
}
static inline int dclose(int fd)
{
	return iomanX_dclose(fd);
}
static inline int dread(int fd, iox_dirent_t *buf)
{
	return iomanX_dread(fd, buf);
}

static inline int getstat(const char *name, iox_stat_t *stat)
{
	return iomanX_getstat(name, stat);
}
static inline int chstat(const char *name, iox_stat_t *stat, unsigned int statmask)
{
	return iomanX_chstat(name, stat, statmask);
}

/** This can take take more than one form.  */
static inline int format(const char *dev, const char *blockdev, void *arg, int arglen)
{
	return iomanX_format(dev, blockdev, arg, arglen);
}

#ifndef IOMAN_NO_EXTENDED
/* The newer calls - these are NOT supported by the older IOMAN.  */
static inline int rename(const char *old, const char *new)
{
	return iomanX_rename(old, new);
}
static inline int chdir(const char *name)
{
	return iomanX_chdir(name);
}
static inline int sync(const char *dev, int flag)
{
	return iomanX_sync(dev, flag);
}
static inline int mount(const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
	return iomanX_mount(fsname, devname, flag, arg, arglen);
}
static inline int umount(const char *fsname)
{
	return iomanX_umount(fsname);
}
static inline s64 lseek64(int fd, s64 offset, int whence)
{
	return iomanX_lseek64(fd, offset, whence);
}
static inline int devctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	return iomanX_devctl(name, cmd, arg, arglen, buf, buflen);
}
static inline int symlink(const char *old, const char *new)
{
	return iomanX_symlink(old, new);
}
static inline int readlink(const char *path, char *buf, unsigned int buflen)
{
	return iomanX_readlink(path, buf, buflen);
}
static inline int ioctl2(int fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	return iomanX_ioctl2(fd, cmd, arg, arglen, buf, buflen);
}
#endif /* IOMAN_NO_EXTENDED */

#if !IOMANX_OLD_NAME_ADDDELDRV
static inline int AddDrv(iop_device_t *device)
{
	return iomanX_AddDrv(device);
}
static inline int DelDrv(const char *name)
{
	return iomanX_DelDrv(name);
}
#endif

static inline void StdioInit(int mode)
{
	return iomanX_StdioInit(mode);
}

#ifdef _IOP
#define I_GetDeviceList I_iomanX_GetDeviceList
#define I_open I_iomanX_open
#define I_close I_iomanX_close
#define I_read I_iomanX_read
#define I_write I_iomanX_write
#define I_lseek I_iomanX_lseek
#define I_ioctl I_iomanX_ioctl
#define I_remove I_iomanX_remove
#define I_mkdir I_iomanX_mkdir
#define I_rmdir I_iomanX_rmdir
#define I_dopen I_iomanX_dopen
#define I_dclose I_iomanX_dclose
#define I_dread I_iomanX_dread
#define I_getstat I_iomanX_getstat
#define I_chstat I_iomanX_chstat
#define I_format I_iomanX_format
#define I_rename I_iomanX_rename
#define I_chdir I_iomanX_chdir
#define I_sync I_iomanX_sync
#define I_mount I_iomanX_mount
#define I_umount I_iomanX_umount
#define I_lseek64 I_iomanX_lseek64
#define I_devctl I_iomanX_devctl
#define I_symlink I_iomanX_symlink
#define I_readlink I_iomanX_readlink
#define I_ioctl2 I_iomanX_ioctl2
#if !IOMANX_OLD_NAME_ADDDELDRV
#define I_AddDrv I_iomanX_AddDrv
#define I_DelDrv I_iomanX_DelDrv
#endif
#define I_StdioInit I_iomanX_StdioInit
#endif

#endif /* IOMANX_OLD_NAME_COMPATIBILITY */

#ifdef __cplusplus
}
#endif

#endif /* __IOMANX_H__ */
