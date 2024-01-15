/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOMAN definitions and imports.
 */

#ifndef __IOMAN_MOD_H__
#define __IOMAN_MOD_H__

#include <types.h>
#include <irx.h>
#include <io_common.h>
#include <iox_stat.h>

#ifdef __cplusplus
extern "C" {
#endif

int io_open(const char *name, int mode);
int io_close(int fd);
int io_read(int fd, void *ptr, size_t size);
int io_write(int fd, void *ptr, size_t size);
int io_lseek(int fd, int pos, int mode);
int io_ioctl(int fd, unsigned long arg, void *param);
int io_remove(const char *name);
int io_mkdir(const char *path);
int io_rmdir(const char *path);
int io_dopen(const char *path, int mode);
int io_dclose(int fd);
int io_dread(int fd, io_dirent_t *buf);
int io_getstat(const char *name, io_stat_t *stat);
int io_chstat(const char *name, io_stat_t *stat, unsigned int statmask);
int io_format(const char *dev);

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR	0x01
#define IOP_DT_CONS	0x02
#define IOP_DT_BLOCK	0x04
#define IOP_DT_RAW	0x08
#define IOP_DT_FS	0x10

/** File objects passed to driver operations.  */
typedef struct _iop_io_file {
	/** File open mode.  */
	int	mode;		
	/** HW device unit number.  */
	int	unit;		
	/** Device driver.  */
	struct _iop_io_device *device; 
	/** The device driver can use this however it wants.  */
	void	*privdata;	
} iop_io_file_t;

typedef struct _iop_io_device {
	const char *name;
	unsigned int type;
	/** Not so sure about this one.  */
	unsigned int version;
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
	int	(*io_lseek)(iop_io_file_t *, int, int);
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
int io_DelDrv(const char *name);

#define ioman_mod_IMPORTS_start DECLARE_IMPORT_TABLE(ioman, 1, 1)
#define ioman_mod_IMPORTS_end END_IMPORT_TABLE

#define I_io_open DECLARE_IMPORT(4, io_open)
#define I_io_close DECLARE_IMPORT(5, io_close)
#define I_io_read DECLARE_IMPORT(6, io_read)
#define I_io_write DECLARE_IMPORT(7, io_write)
#define I_io_lseek DECLARE_IMPORT(8, io_lseek)
#define I_io_ioctl DECLARE_IMPORT(9, io_ioctl)
#define I_io_remove DECLARE_IMPORT(10, io_remove)
#define I_io_mkdir DECLARE_IMPORT(11, io_mkdir)
#define I_io_rmdir DECLARE_IMPORT(12, io_rmdir)
#define I_io_dopen DECLARE_IMPORT(13, io_dopen)
#define I_io_dclose DECLARE_IMPORT(14, io_dclose)
#define I_io_dread DECLARE_IMPORT(15, io_dread)
#define I_io_getstat DECLARE_IMPORT(16, io_getstat)
#define I_io_chstat DECLARE_IMPORT(17, io_chstat)
#define I_io_format DECLARE_IMPORT(18, io_format)
#define I_io_AddDrv DECLARE_IMPORT(20, io_AddDrv);
#define I_io_DelDrv DECLARE_IMPORT(21, io_DelDrv);

#ifdef __cplusplus
}
#endif

#endif /* __IOMAN_MOD_H__ */
