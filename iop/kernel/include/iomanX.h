/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Definitions and imports for iomanX.
*/

#ifndef IOP_IOMANX_H
#define IOP_IOMANX_H

#include "types.h"
#include "irx.h"

#include "sys/fcntl.h"

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR	0x01
#define IOP_DT_CONS	0x02
#define IOP_DT_BLOCK	0x04
#define IOP_DT_RAW	0x08
#define IOP_DT_FS	0x10
#ifndef IOMAN_NO_EXTENDED
#define IOP_DT_FSEXT	0x10000000	/* Supports calls after chstat().  */
#endif

/* File objects passed to driver operations.  */
typedef struct _iop_file {
	int	mode;		/* File open mode.  */
	int	unit;		/* HW device unit number.  */
	struct _iop_device *device; /* Device driver.  */
	void	*privdata;	/* The device driver can use this however it
				   wants.  */
} iop_file_t;

typedef struct _iop_device {
	const char *name;
	unsigned int type;
	unsigned int version;	/* Not so sure about this one.  */
	const char *desc;
	struct _iop_device_ops *ops;
} iop_device_t;

typedef struct _iop_device_ops {
	int	(*init)(iop_device_t *);
	int	(*deinit)(iop_device_t *);
	int	(*format)(iop_file_t *, const char *, const char *, void *, size_t);
	int	(*open)(iop_file_t *, const char *, int, ...);
	int	(*close)(iop_file_t *);
	int	(*read)(iop_file_t *, void *, int);
	int	(*write)(iop_file_t *, void *, int);
	int	(*lseek)(iop_file_t *, unsigned long, int);
	int	(*ioctl)(iop_file_t *, unsigned long, void *);
	int	(*remove)(iop_file_t *, const char *);
	int	(*mkdir)(iop_file_t *, const char *, int);
	int	(*rmdir)(iop_file_t *, const char *);
	int	(*dopen)(iop_file_t *, const char *);
	int	(*dclose)(iop_file_t *);
	int	(*dread)(iop_file_t *, void *);
	int	(*getstat)(iop_file_t *, const char *, void *);
	int	(*chstat)(iop_file_t *, const char *, void *, unsigned int);

#ifndef IOMAN_NO_EXTENDED
	/* Extended ops start here.  */
	int	(*rename)(iop_file_t *, const char *, const char *);
	int	(*chdir)(iop_file_t *, const char *);
	int	(*sync)(iop_file_t *, const char *, int);
	int	(*mount)(iop_file_t *, const char *, const char *, int, void *, unsigned int);
	int	(*umount)(iop_file_t *, const char *);
	int	(*lseek64)(iop_file_t *, long long, int);
	int	(*devctl)(iop_file_t *, const char *, int, void *, unsigned int, void *, unsigned int);
	int	(*symlink)(iop_file_t *, const char *, const char *);
	int	(*readlink)(iop_file_t *, const char *, char *, unsigned int);
	int	(*ioctl2)(iop_file_t *, int, void *, unsigned int, void *, unsigned int);
#endif /* IOMAN_NO_EXTENDED */
} iop_device_ops_t;

#define iomanX_IMPORTS_start DECLARE_IMPORT_TABLE(iomanx, 1, 1)
#define iomanX_IMPORTS_end END_IMPORT_TABLE

iop_device_t **GetDeviceList(void);
#define I_GetDeviceList DECLARE_IMPORT(3, GetDeviceList)

/* open() takes an optional mode argument.  */
int open(const char *name, int flags, ...);
#define I_open DECLARE_IMPORT(4, open)
int close(int fd);
#define I_close DECLARE_IMPORT(5, close)
int read(int fd, void *ptr, size_t size);
#define I_read DECLARE_IMPORT(6, read)
int write(int fd, void *ptr, size_t size);
#define I_write DECLARE_IMPORT(7, write)
int lseek(int fd, int offset, int mode);
#define I_lseek DECLARE_IMPORT(8, lseek)

int ioctl(int fd, unsigned long cmd, void *param);
#define I_ioctl DECLARE_IMPORT(9, ioctl)
int remove(const char *name);
#define I_remove DECLARE_IMPORT(10, remove)

/* mkdir() takes an optional mode argument.  */
int mkdir(const char *path, ...);
#define I_mkdir DECLARE_IMPORT(11, mkdir)
int rmdir(const char *path);
#define I_rmdir DECLARE_IMPORT(12, rmdir)
int dopen(const char *path);
#define I_dopen DECLARE_IMPORT(13, dopen)
int dclose(int fd);
#define I_dclose DECLARE_IMPORT(14, dclose)
int dread(int fd, void *buf);
#define I_dread DECLARE_IMPORT(15, dread)

int getstat(const char *name, void *stat);
#define I_getstat DECLARE_IMPORT(16, getstat)
int chstat(const char *name, void *stat, unsigned int statmask);
#define I_chstat DECLARE_IMPORT(17, chstat)

/* This can take take more than one form.  */
int format(const char *dev, const char *blockdev, void *arg, size_t arglen);
#define I_format DECLARE_IMPORT(18, format)

#ifndef IOMAN_NO_EXTENDED
/* The newer calls - these are NOT supported by the older IOMAN.  */
int rename(const char *old, const char *new);
#define I_rename DECLARE_IMPORT(25, rename)
int chdir(const char *name);
#define I_chdir DECLARE_IMPORT(26, chdir)
int sync(const char *dev, int flag);
#define I_sync DECLARE_IMPORT(27, sync)
int mount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen);
#define I_mount DECLARE_IMPORT(28, mount)
int umount(const char *fsname);
#define I_umount DECLARE_IMPORT(29, umount)
int lseek64(int fd, long long offset, int whence);
#define I_lseek64 DECLARE_IMPORT(30, lseek64)
int devctl(const char *name, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
#define I_devctl DECLARE_IMPORT(31, devctl)
int symlink(const char *old, const char *new);
#define I_symlink DECLARE_IMPORT(32, symlink)
int readlink(const char *path, char *buf, size_t buflen);
#define I_readlink DECLARE_IMPORT(33, readlink)
int ioctl2(int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
#define I_ioctl2 DECLARE_IMPORT(34, ioctl2)
#endif /* IOMAN_NO_EXTENDED */

int AddDrv(iop_device_t *device);
#define I_AddDrv DECLARE_IMPORT(20, AddDrv);
int DelDrv(const char *name);
#define I_DelDrv DECLARE_IMPORT(21, DelDrv);

#endif /* IOP_IOMANX_H */
