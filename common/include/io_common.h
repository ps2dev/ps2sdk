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

#ifdef _EE
#ifndef NEWLIB_PORT_AWARE
#error "Using fio/fileXio functions directly in the newlib port will lead to problems."
#error "Use posix function calls instead."
#endif
#endif

#define FIO_O_RDONLY  0x0001
#define FIO_O_WRONLY  0x0002
#define FIO_O_RDWR    0x0003
#define FIO_O_DIROPEN 0x0008 // Internal use for dopen
#define FIO_O_NBLOCK  0x0010
#define FIO_O_APPEND  0x0100
#define FIO_O_CREAT   0x0200
#define FIO_O_TRUNC   0x0400
#define FIO_O_EXCL    0x0800
#define FIO_O_NOWAIT  0x8000

#define FIO_MT_RDWR   0x00
#define FIO_MT_RDONLY 0x01

#define FIO_SEEK_SET 0
#define FIO_SEEK_CUR 1
#define FIO_SEEK_END 2

typedef struct
{
    unsigned int mode;
    unsigned int attr;
    unsigned int size;
    unsigned char ctime[8];
    unsigned char atime[8];
    unsigned char mtime[8];
    unsigned int hisize;
} io_stat_t;

typedef struct
{
    io_stat_t stat;
    char name[256];
    void *privdata;
} io_dirent_t;

#endif /* __IO_COMMON_H__ */
