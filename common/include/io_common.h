/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Shared IO structures and definitions
 */

#ifndef __IO_COMMON_H__
#define __IO_COMMON_H__

#include <sys/fcntl.h>
#include <sys/unistd.h>

#define FIO_MT_RDWR		0x00
#define	FIO_MT_RDONLY		0x01

typedef struct {
	unsigned int mode;
	unsigned int attr;
	unsigned int size;
	unsigned char ctime[8];
	unsigned char atime[8];
	unsigned char mtime[8];
	unsigned int hisize;
} fio_stat_t;

typedef struct {
	fio_stat_t stat;
	char name[256];
	unsigned int unknown;
} fio_dirent_t;

#endif /* __IO_COMMON_H__ */
