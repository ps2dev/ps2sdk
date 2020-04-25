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
 * IOMAN definitions and imports.
 */

#ifndef __IOMAN_H__
#define __IOMAN_H__

#include <types.h>
#include <irx.h>
#include <io_common.h>

int open(const char *name, int mode);
int close(int fd);
int read(int fd, void *ptr, size_t size);
int write(int fd, void *ptr, size_t size);
int lseek(int fd, int pos, int mode);
int ioctl(int fd, int command, void *arg);
int remove(const char *name);
int mkdir(const char *path);
int rmdir(const char *path);
int dopen(const char *path, int mode);
int dclose(int fd);
int dread(int fd, io_dirent_t *buf);
int getstat(const char *name, io_stat_t *stat);
int chstat(const char *name, io_stat_t *stat, unsigned int statmask);
int format(const char *dev);

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR	0x01
#define IOP_DT_CONS	0x02
#define IOP_DT_BLOCK	0x04
#define IOP_DT_RAW	0x08
#define IOP_DT_FS	0x10

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
	int	(*format)(iop_file_t *);
	int	(*open)(iop_file_t *, const char *, int);
	int	(*close)(iop_file_t *);
	int	(*read)(iop_file_t *, void *, int);
	int	(*write)(iop_file_t *, void *, int);
	int	(*lseek)(iop_file_t *, int, int);
	int	(*ioctl)(iop_file_t *, int, void *);
	int	(*remove)(iop_file_t *, const char *);
	int	(*mkdir)(iop_file_t *, const char *);
	int	(*rmdir)(iop_file_t *, const char *);
	int	(*dopen)(iop_file_t *, const char *);
	int	(*dclose)(iop_file_t *);
	int	(*dread)(iop_file_t *, io_dirent_t *);
	int	(*getstat)(iop_file_t *, const char *, io_stat_t *);
	int	(*chstat)(iop_file_t *, const char *, io_stat_t *, unsigned int);
} iop_device_ops_t;

int AddDrv(iop_device_t *device);
int DelDrv(const char *name);

#define ioman_IMPORTS_start DECLARE_IMPORT_TABLE(ioman, 1, 1)
#define ioman_IMPORTS_end END_IMPORT_TABLE

#define I_open DECLARE_IMPORT(4, open)
#define I_close DECLARE_IMPORT(5, close)
#define I_read DECLARE_IMPORT(6, read)
#define I_write DECLARE_IMPORT(7, write)
#define I_lseek DECLARE_IMPORT(8, lseek)
#define I_ioctl DECLARE_IMPORT(9, ioctl)
#define I_remove DECLARE_IMPORT(10, remove)
#define I_mkdir DECLARE_IMPORT(11, mkdir)
#define I_rmdir DECLARE_IMPORT(12, rmdir)
#define I_dopen DECLARE_IMPORT(13, dopen)
#define I_dclose DECLARE_IMPORT(14, dclose)
#define I_dread DECLARE_IMPORT(15, dread)
#define I_getstat DECLARE_IMPORT(16, getstat)
#define I_chstat DECLARE_IMPORT(17, chstat)
#define I_format DECLARE_IMPORT(18, format)
#define I_AddDrv DECLARE_IMPORT(20, AddDrv);
#define I_DelDrv DECLARE_IMPORT(21, DelDrv);

#endif /* __IOMAN_H__ */
