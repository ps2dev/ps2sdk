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
# IOMAN definitions and imports.
*/

#ifndef IOP_IOMAN_H
#define IOP_IOMAN_H

#include "types.h"
#include "irx.h"

#include "sys/fcntl.h"

#define ioman_IMPORTS_start DECLARE_IMPORT_TABLE(ioman, 1, 1)
#define ioman_IMPORTS_end END_IMPORT_TABLE

int open(const char *name, int mode);
#define I_open DECLARE_IMPORT(4, open)
int close(int fd);
#define I_close DECLARE_IMPORT(5, close)
int read(int fd, void *ptr, size_t size);
#define I_read DECLARE_IMPORT(6, read)
int write(int fd, void *ptr, size_t size);
#define I_write DECLARE_IMPORT(7, write)
int lseek(int fd, unsigned long pos, int mode);
#define I_lseek DECLARE_IMPORT(8, lseek)

int format(const char *dev);
#define I_format DECLARE_IMPORT(18, format)

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR	0x01
#define IOP_DT_CONS	0x02
#define IOP_DT_BLOCK	0x04
#define IOP_DT_RAW	0x08
#define IOP_DT_FS	0x10

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
	int	(*format)(iop_file_t *, ...);
	int	(*open)(iop_file_t *, const char *, int, ...);
	int	(*close)(iop_file_t *);
	int	(*read)(iop_file_t *, void *, int);
	int	(*write)(iop_file_t *, void *, int);
	int	(*lseek)(iop_file_t *, unsigned long, int);
	int	(*ioctl)(iop_file_t *, unsigned long, void *);
	int	(*remove)(iop_file_t *, const char *);
	int	(*mkdir)(iop_file_t *, const char *);
	int	(*rmdir)(iop_file_t *, const char *);
	int	(*dopen)(iop_file_t *, const char *);
	int	(*dclose)(iop_file_t *);
	int	(*dread)(iop_file_t *, void *);
	int	(*getstat)(iop_file_t *, const char *, void *);
	int	(*chstat)(iop_file_t *, const char *, void *, unsigned int);
} iop_device_ops_t;

int AddDrv(iop_device_t *device);
#define I_AddDrv DECLARE_IMPORT(20, AddDrv);
int DelDrv(const char *name);
#define I_DelDrv DECLARE_IMPORT(21, DelDrv);

#endif /* IOP_IOMAN_H */
