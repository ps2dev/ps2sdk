/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Common definitions for fileio between the client and server sides of the FILEIO protocol.
 */

#ifndef __FILEIO_COMMON_H__
#define __FILEIO_COMMON_H__

#include <tamtypes.h>
#include <io_common.h>

// fileio common definitions

enum _fio_functions {
    FIO_F_OPEN = 0,
    FIO_F_CLOSE,
    FIO_F_READ,
    FIO_F_WRITE,
    FIO_F_LSEEK,
    FIO_F_IOCTL,
    FIO_F_REMOVE,
    FIO_F_MKDIR,
    FIO_F_RMDIR,
    FIO_F_DOPEN,
    FIO_F_DCLOSE,
    FIO_F_DREAD,
    FIO_F_GETSTAT,
    FIO_F_CHSTAT,
    FIO_F_FORMAT,
    FIO_F_ADDDRV,
    FIO_F_DELDRV,
};

/** Shared between _fio_read_intr and fio_read.  The updated modules shipped
   with licensed games changed the size of the buffers from 16 to 64.  */
struct _fio_read_data
{
    u32 size1;
    u32 size2;
    void *dest1;
    void *dest2;
    u8 buf1[16];
    u8 buf2[16];
};

#define FIO_PATH_MAX 256

struct _fio_open_arg
{
    int mode;
    char name[FIO_PATH_MAX];
} __attribute__((aligned(16)));

struct _fio_read_arg
{
    int fd;
    void *ptr;
    int size;
    struct _fio_read_data *read_data;
} __attribute__((aligned(16)));

struct _fio_write_arg
{
    int fd;
    const void *ptr;
    u32 size;
    u32 mis;
    u8 aligned[16];
} __attribute__((aligned(16)));

struct _fio_lseek_arg
{
    union
    {
        int fd;
        int result;
    } p;
    int offset;
    int whence;
} __attribute__((aligned(16)));

struct _fio_ioctl_arg
{
    union
    {
        int fd;
        int result;
    } p;
    int request;
    u8 data[1024]; // Will this be ok ?
} __attribute__((aligned(16)));

struct _fio_dread_arg
{
    union
    {
        int fd;
        int result;
    } p;
    io_dirent_t *buf;
} __attribute__((aligned(16)));

struct _fio_getstat_arg
{
    union
    {
        io_stat_t *buf;
        int result;
    } p;
    char name[FIO_PATH_MAX];
} __attribute__((aligned(16)));

struct _fio_chstat_arg
{
    union
    {
        int cbit;
        int result;
    } p;
    io_stat_t stat;
    char name[FIO_PATH_MAX];
};

#endif /* __FILEIO_COMMON_H__ */
