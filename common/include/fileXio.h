/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# fileXio RPC client/server shared includes 
*/

#ifndef _FILEXIO_H
#define _FILEXIO_H

#include <errno.h>

// This header contains the common definitions for fileXio
// that are used by both IOP and EE sides

#define FILEXIO_IRX	0xb0b0b00
enum FILEXIO_CMDS{
	FILEXIO_DOPEN	= 0x01,
	FILEXIO_DREAD,
	FILEXIO_DCLOSE,
	FILEXIO_MOUNT,
	FILEXIO_UMOUNT,
	FILEXIO_GETDIR,
	FILEXIO_STOP,
	FILEXIO_COPYFILE,
	FILEXIO_OPEN,
	FILEXIO_CLOSE,
	FILEXIO_READ,
	FILEXIO_WRITE,
	FILEXIO_LSEEK,
	FILEXIO_IOCTL,
	FILEXIO_RMDIR,
	FILEXIO_GETSTAT,
	FILEXIO_CHSTAT,
	FILEXIO_FORMAT,
	FILEXIO_ADDDRV,
	FILEXIO_DELDRV,
	FILEXIO_RENAME,
	FILEXIO_CHDIR,
	FILEXIO_SYNC,
	FILEXIO_DEVCTL,
	FILEXIO_SYMLINK,
	FILEXIO_READLINK,
	FILEXIO_IOCTL2,
	FILEXIO_LSEEK64,
	FILEXIO_MKDIR,
	FILEXIO_REMOVE,
	FILEXIO_GETDEVICELIST,
	FILEXIO_SETRWBUFFSIZE
};

#define FILEXIO_MOUNTFLAG_NORMAL	0
#define FILEXIO_MOUNTFLAG_READONLY	1
#define FILEXIO_MOUNTFLAG_ROBUST	2

#define FILEXIO_DIRFLAGS_DIR	0xa0
#define FILEXIO_DIRFLAGS_FILE	0x80

#define CTL_BUF_SIZE	2048
#define IOCTL_BUF_SIZE	1024

#define FILEXIO_MAX_DEVICES 32

#define FILEXIO_DT_CHAR	0x01
#define FILEXIO_DT_CONS	0x02
#define FILEXIO_DT_BLOCK	0x04
#define FILEXIO_DT_RAW	0x08
#define FILEXIO_DT_FS	0x10
#define FILEXIO_DT_FSEXT	0x10000000	/* Supports calls after chstat().  */

struct fileXioDirEntry
{
	u32   fileSize;
	u8	fileProperties;
	u8	filename[128+1];
} __attribute__((aligned(64)));

struct fileXioDevice
{
	char name[15];
	unsigned int type;
	unsigned int version;	/* Not so sure about this one.  */
	char desc[128];
} __attribute__((aligned(64)));

struct devctl_packet {
	char name[CTL_BUF_SIZE];
	u8 arg[CTL_BUF_SIZE];
	int cmd;
	int arglen;
	void *buf;
	int buflen;
	void *intr_data;
};

struct ioctl_packet {
	int fd;
	u8 arg[IOCTL_BUF_SIZE];
	int cmd;
};

struct ioctl2_packet {
	int fd;
	u8 arg[CTL_BUF_SIZE];
	int cmd;
	int arglen;
	void *buf;
	int buflen;
	void *intr_data;
};

struct fxio_ctl_return_pkt {
	void *dest;
	int len;
	u8 buf[CTL_BUF_SIZE];
	int padding[2];
};

struct fxio_rwbuff{
	int size;
};

#endif // _FILEXIO_H
