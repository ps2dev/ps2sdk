/*
 * fileXio_iop.c - fileXio RPC client/server shared includes 
 *
 * Copyright (c) 2003 adresd <adresd_ps2dev@yahoo.com>
 *
 * This is the main header that users of the fileXio RPC need to include 
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */
#ifndef _FILEXIO_H
#define _FILEXIO_H

#include "errno.h"

// This header contains the common definitions for fileXio
// that are used by both IOP and EE sides

#define FILEXIO_IRX	0xb0b0b00
#define FILEXIO_DOPEN	0x01
#define FILEXIO_DREAD	0x02
#define FILEXIO_DCLOSE	0x03
#define FILEXIO_MOUNT	0x04
#define FILEXIO_UMOUNT	0x05
#define FILEXIO_GETDIR	0x06
#define FILEXIO_STOP	0x07
#define FILEXIO_COPYFILE	0x08
#define FILEXIO_OPEN	0x09
#define FILEXIO_CLOSE	0x0a
#define FILEXIO_READ	0x0b
#define FILEXIO_WRITE	0x0c
#define FILEXIO_LSEEK	0x0d
#define FILEXIO_IOCTL	0x0e
#define FILEXIO_RMDIR	0x0f
#define FILEXIO_GETSTAT	0x10
#define FILEXIO_CHSTAT	0x11
#define FILEXIO_FORMAT	0x12
#define FILEXIO_ADDDRV	0x13
#define FILEXIO_DELDRV	0x14
#define FILEXIO_RENAME	0x15
#define FILEXIO_CHDIR	0x16
#define FILEXIO_SYNC	0x17
#define FILEXIO_DEVCTL	0x18
#define FILEXIO_SYMLINK	0x19
#define FILEXIO_READLINK	0x1a
#define FILEXIO_IOCTL2	0x1b
#define FILEXIO_LSEEK64	0x1c
#define FILEXIO_MKDIR	0x1d
#define FILEXIO_REMOVE	0x1e

#define FILEXIO_MOUNTFLAG_NORMAL	0
#define FILEXIO_MOUNTFLAG_READONLY	1
#define FILEXIO_MOUNTFLAG_ROBUST	2

#define FILEXIO_DIRFLAGS_DIR	0xa0
#define FILEXIO_DIRFLAGS_FILE	0x80

#define CTL_BUF_SIZE	2048

struct fileXioDirEntry
{
	u32   fileSize;
	u8	fileProperties;
	u8	filename[128+1];
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

#endif // _FILEXIO_H
