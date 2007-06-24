/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# IOMAN definitions and imports.
*/

#ifndef IOP_IOMAN_MOD_H
#define IOP_IOMAN_MOD_H

#include "types.h"
#include "irx.h"

#include "sys/fcntl.h"
#include "sys/stat.h"

#define ioman_mod_IMPORTS_start DECLARE_IMPORT_TABLE(ioman, 1, 1)
#define ioman_mod_IMPORTS_end END_IMPORT_TABLE

int io_open(const char *name, int mode);
#define I_io_open DECLARE_IMPORT(4, io_open)
int io_close(int fd);
#define I_io_close DECLARE_IMPORT(5, io_close)
int io_read(int fd, void *ptr, size_t size);
#define I_io_read DECLARE_IMPORT(6, io_read)
int io_write(int fd, void *ptr, size_t size);
#define I_io_write DECLARE_IMPORT(7, io_write)
int io_lseek(int fd, unsigned long pos, int mode);
#define I_io_lseek DECLARE_IMPORT(8, io_lseek)
int io_ioctl(int fd, unsigned long arg, void *param);
#define I_io_ioctl DECLARE_IMPORT(9, io_ioctl)
int io_remove(const char *name);
#define I_io_remove DECLARE_IMPORT(10, io_remove)
int io_mkdir(const char *path);
#define I_io_mkdir DECLARE_IMPORT(11, io_mkdir)
int io_rmdir(const char *path);
#define I_io_rmdir DECLARE_IMPORT(12, io_rmdir)
int io_dopen(const char *path, int mode);
#define I_io_dopen DECLARE_IMPORT(13, io_dopen)
int io_dclose(int fd);
#define I_io_dclose DECLARE_IMPORT(14, io_dclose)
int io_dread(int fd, io_dirent_t *buf);
#define I_io_dread DECLARE_IMPORT(15, io_dread)
int io_getstat(const char *name, io_stat_t *stat);
#define I_io_getstat DECLARE_IMPORT(16, io_getstat)
int io_chstat(const char *name, io_stat_t *stat, unsigned int statmask);
#define I_io_chstat DECLARE_IMPORT(17, io_chstat)

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR	0x01
#define IOP_DT_CONS	0x02
#define IOP_DT_BLOCK	0x04
#define IOP_DT_RAW	0x08
#define IOP_DT_FS	0x10

/* File objects passed to driver operations.  */
typedef struct _iop_io_file {
	int	mode;		/* File open mode.  */
	int	unit;		/* HW device unit number.  */
	struct _iop_io_device *device; /* Device driver.  */
	void	*privdata;	/* The device driver can use this however it
				   wants.  */
} iop_io_file_t;

typedef struct _iop_io_device {
	const char *name;
	unsigned int type;
	unsigned int version;	/* Not so sure about this one.  */
	const char *desc;
	struct _iop_io_device_ops *ops;
} iop_io_device_t;

typedef struct _iop_io_device_ops {
	int	(*io_init)(iop_io_device_t *);
	int	(*io_deinit)(iop_io_device_t *);
	int	(*io_format)(iop_io_file_t *);
	int	(*io_open)(iop_io_file_t *, const char *, int);
	int	(*io_close)(iop_io_file_t *);
	int	(*io_read)(iop_io_file_t *, void *, int);
	int	(*io_write)(iop_io_file_t *, void *, int);
	int	(*io_lseek)(iop_io_file_t *, unsigned long, int);
	int	(*io_ioctl)(iop_io_file_t *, unsigned long, void *);
	int	(*io_remove)(iop_io_file_t *, const char *);
	int	(*io_mkdir)(iop_io_file_t *, const char *);
	int	(*io_rmdir)(iop_io_file_t *, const char *);
	int	(*io_dopen)(iop_io_file_t *, const char *);
	int	(*io_dclose)(iop_io_file_t *);
	int	(*io_dread)(iop_io_file_t *, io_dirent_t *);
	int	(*io_getstat)(iop_io_file_t *, const char *, io_stat_t *);
	int	(*io_chstat)(iop_io_file_t *, const char *, io_stat_t *, unsigned int);
} iop_io_device_ops_t;

int io_AddDrv(iop_io_device_t *device);
#define I_io_AddDrv DECLARE_IMPORT(20, io_AddDrv);
int io_DelDrv(const char *name);
#define I_io_DelDrv DECLARE_IMPORT(21, io_DelDrv);

#endif /* IOP_IOMAN_H */
