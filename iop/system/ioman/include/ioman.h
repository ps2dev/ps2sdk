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

#ifdef __cplusplus
extern "C" {
#endif

extern int open(const char *name, int mode);
extern int close(int fd);
extern int read(int fd, void *ptr, size_t size);
extern int write(int fd, void *ptr, size_t size);
extern int lseek(int fd, int pos, int mode);
extern int ioctl(int fd, int command, void *arg);
extern int remove(const char *name);
extern int mkdir(const char *path);
extern int rmdir(const char *path);
extern int dopen(const char *path, int mode);
extern int dclose(int fd);
extern int dread(int fd, io_dirent_t *buf);
extern int getstat(const char *name, io_stat_t *stat);
extern int chstat(const char *name, io_stat_t *stat, unsigned int statmask);
extern int format(const char *dev);

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

#define IOMAN_RETURN_VALUE_IMPL(val) \
	static inline int my_ioman_retval_##val##_int(void) {return -val;} \
	static inline signed long long my_ioman_retval_##val##_s64(void) {return -val;}	
#define IOMAN_RETURN_VALUE(val) ((void*)&my_ioman_retval_##val##_int)
#define IOMAN_RETURN_VALUE_S64(val) ((void*)&my_ioman_retval_##val##_s64)

typedef struct _iop_device_ops {
	int	(*init)(iop_device_t *device);
	int	(*deinit)(iop_device_t *device);
	int	(*format)(iop_file_t *f);
	int	(*open)(iop_file_t *f, const char *name, int flags);
	int	(*close)(iop_file_t *f);
	int	(*read)(iop_file_t *f, void *ptr, int size);
	int	(*write)(iop_file_t *f, void *ptr, int size);
	int	(*lseek)(iop_file_t *f, int offset, int mode);
	int	(*ioctl)(iop_file_t *f, int cmd, void *param);
	int	(*remove)(iop_file_t *f, const char *name);
	int	(*mkdir)(iop_file_t *f, const char *path);
	int	(*rmdir)(iop_file_t *f, const char *path);
	int	(*dopen)(iop_file_t *f, const char *path);
	int	(*dclose)(iop_file_t *f);
	int	(*dread)(iop_file_t *f, io_dirent_t *buf);
	int	(*getstat)(iop_file_t *f, const char *name, io_stat_t *stat_);
	int	(*chstat)(iop_file_t *f, const char *name, io_stat_t *stat_, unsigned int statmask);
} iop_device_ops_t;

extern int AddDrv(iop_device_t *device);
extern int DelDrv(const char *name);

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

#ifdef __cplusplus
}
#endif

#endif /* __IOMAN_H__ */
