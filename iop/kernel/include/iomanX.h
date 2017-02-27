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

#ifndef IOP_IOMANX_H
#define IOP_IOMANX_H

#include "types.h"
#include "irx.h"

#include "sys/fcntl.h"
#include "sys/stat.h"

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
typedef struct _iop_file {
	/** File open mode.  */
	int	mode;		
	/** HW device unit number.  */
	int	unit;		
	/** Device driver.  */
	struct _iop_device *device; 
	/** The device driver can use this however it wants.  */
	void	*privdata;	
} iop_file_t;

typedef struct _iop_device {
	const char *name;
	unsigned int type;
	/** Not so sure about this one.  */
	unsigned int version;
	const char *desc;
	struct _iop_device_ops *ops;
} iop_device_t;

typedef struct _iop_device_ops {
	int	(*init)(iop_device_t *);
	int	(*deinit)(iop_device_t *);
	int	(*format)(iop_file_t *, const char *, const char *, void *, int);
	int	(*open)(iop_file_t *, const char *, int, int);
	int	(*close)(iop_file_t *);
	int	(*read)(iop_file_t *, void *, int);
	int	(*write)(iop_file_t *, void *, int);
	int	(*lseek)(iop_file_t *, int, int);
	int	(*ioctl)(iop_file_t *, int, void *);
	int	(*remove)(iop_file_t *, const char *);
	int	(*mkdir)(iop_file_t *, const char *, int);
	int	(*rmdir)(iop_file_t *, const char *);
	int	(*dopen)(iop_file_t *, const char *);
	int	(*dclose)(iop_file_t *);
	int	(*dread)(iop_file_t *, iox_dirent_t *);
	int	(*getstat)(iop_file_t *, const char *, iox_stat_t *);
	int	(*chstat)(iop_file_t *, const char *, iox_stat_t *, unsigned int);

#ifndef IOMAN_NO_EXTENDED
	/* Extended ops start here.  */
	int	(*rename)(iop_file_t *, const char *, const char *);
	int	(*chdir)(iop_file_t *, const char *);
	int	(*sync)(iop_file_t *, const char *, int);
	int	(*mount)(iop_file_t *, const char *, const char *, int, void *, int);
	int	(*umount)(iop_file_t *, const char *);
	s64	(*lseek64)(iop_file_t *, s64, int);
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

int open(const char *name, int flags, ...);
#define I_open DECLARE_IMPORT(4, open)
int close(int fd);
#define I_close DECLARE_IMPORT(5, close)
int read(int fd, void *ptr, int size);
#define I_read DECLARE_IMPORT(6, read)
int write(int fd, void *ptr, int size);
#define I_write DECLARE_IMPORT(7, write)
int lseek(int fd, int offset, int mode);
#define I_lseek DECLARE_IMPORT(8, lseek)

int ioctl(int fd, int cmd, void *param);
#define I_ioctl DECLARE_IMPORT(9, ioctl)
int remove(const char *name);
#define I_remove DECLARE_IMPORT(10, remove)

int mkdir(const char *path, int mode);
#define I_mkdir DECLARE_IMPORT(11, mkdir)
int rmdir(const char *path);
#define I_rmdir DECLARE_IMPORT(12, rmdir)
int dopen(const char *path);
#define I_dopen DECLARE_IMPORT(13, dopen)
int dclose(int fd);
#define I_dclose DECLARE_IMPORT(14, dclose)
int dread(int fd, iox_dirent_t *buf);
#define I_dread DECLARE_IMPORT(15, dread)

int getstat(const char *name, iox_stat_t *stat);
#define I_getstat DECLARE_IMPORT(16, getstat)
int chstat(const char *name, iox_stat_t *stat, unsigned int statmask);
#define I_chstat DECLARE_IMPORT(17, chstat)

/** This can take take more than one form.  */
int format(const char *dev, const char *blockdev, void *arg, int arglen);
#define I_format DECLARE_IMPORT(18, format)

#ifndef IOMAN_NO_EXTENDED
/* The newer calls - these are NOT supported by the older IOMAN.  */
int rename(const char *old, const char *new);
#define I_rename DECLARE_IMPORT(25, rename)
int chdir(const char *name);
#define I_chdir DECLARE_IMPORT(26, chdir)
int sync(const char *dev, int flag);
#define I_sync DECLARE_IMPORT(27, sync)
int mount(const char *fsname, const char *devname, int flag, void *arg, int arglen);
#define I_mount DECLARE_IMPORT(28, mount)
int umount(const char *fsname);
#define I_umount DECLARE_IMPORT(29, umount)
s64 lseek64(int fd, s64 offset, int whence);
#define I_lseek64 DECLARE_IMPORT(30, lseek64)
int devctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
#define I_devctl DECLARE_IMPORT(31, devctl)
int symlink(const char *old, const char *new);
#define I_symlink DECLARE_IMPORT(32, symlink)
int readlink(const char *path, char *buf, unsigned int buflen);
#define I_readlink DECLARE_IMPORT(33, readlink)
int ioctl2(int fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
#define I_ioctl2 DECLARE_IMPORT(34, ioctl2)
#endif /* IOMAN_NO_EXTENDED */

int AddDrv(iop_device_t *device);
#define I_AddDrv DECLARE_IMPORT(20, AddDrv);
int DelDrv(const char *name);
#define I_DelDrv DECLARE_IMPORT(21, DelDrv);

void StdioInit(int mode);
#define I_StdioInit DECLARE_IMPORT(23, StdioInit);

#endif /* IOP_IOMANX_H */
