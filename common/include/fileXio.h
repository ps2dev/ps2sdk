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
 * fileXio RPC client/server shared includes
 * This header contains the common definitions for fileXio
 * that are used by both IOP and EE sides. This header
 * conflicts with ps2ip_rpc.h
 */

#ifndef __FILEXIO_H__
#define __FILEXIO_H__

#include <tamtypes.h>
#include <iox_stat.h>

#ifdef _EE
#ifndef NEWLIB_PORT_AWARE
#error "Using fio/fileXio functions directly in the newlib port will lead to problems."
#error "Use posix function calls instead."
#endif
#endif

#define FILEXIO_IRX 0xb0b0b00
enum FILEXIO_CMDS {
    FILEXIO_DOPEN = 0x01,
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

/** Used for buffer alignment correction when reading data. */
typedef struct
{
    int ssize;
    int esize;
    void *sbuf;
    void *ebuf;
    u8 sbuffer[64];
    u8 ebuffer[64];
} rests_pkt; // sizeof = 144

#define FILEXIO_MOUNTFLAG_NORMAL   0
#define FILEXIO_MOUNTFLAG_READONLY 1
#define FILEXIO_MOUNTFLAG_ROBUST   2

#define FILEXIO_DIRFLAGS_DIR  0xa0
#define FILEXIO_DIRFLAGS_FILE 0x80

#define CTL_BUF_SIZE   2048
#define IOCTL_BUF_SIZE 1024

#define FILEXIO_MAX_DEVICES 32

#define FILEXIO_DT_CHAR  0x01
#define FILEXIO_DT_CONS  0x02
#define FILEXIO_DT_BLOCK 0x04
#define FILEXIO_DT_RAW   0x08
#define FILEXIO_DT_FS    0x10
/** Supports calls after chstat().  */
#define FILEXIO_DT_FSEXT 0x10000000

struct fileXioDirEntry
{
    u32 fileSize;
    u8 fileProperties;
    char filename[128 + 1];
} __attribute__((aligned(64)));

struct fileXioDevice
{
    char name[16];
    unsigned int type;
    /** Not so sure about this one.  */
    unsigned int version;
    char desc[128];
} __attribute__((aligned(64)));

struct fxio_devlist_packet
{
    struct fileXioDevice *deviceEntry;
    unsigned int reqEntries;
};

struct fxio_getdir_packet
{
    char pathname[512];
    struct fileXioDirEntry *dirEntry;
    unsigned int reqEntries;
};

struct fxio_mount_packet
{
    char blockdevice[512];
    char mountpoint[512];
    int flags;
};

struct fxio_unmount_packet
{
    char mountpoint[512];
};

struct fxio_copyfile_packet
{
    char source[512];
    char dest[512];
    int mode;
};

struct fxio_mkdir_packet
{
    char pathname[512];
    int mode;
};

/** Also used for rmdir, chdir and dopen. */
struct fxio_pathsel_packet
{
    char pathname[512];
};

/** Also used for symlink. */
struct fxio_rename_packet
{
    char source[512];
    char dest[512];
};

struct fxio_readlink_packet
{
    char source[512];
    void *buffer;
    unsigned int buflen;
};

struct fxio_open_packet
{
    char pathname[512];
    int flags, mode;
};

/** Also used by dclose. */
struct fxio_close_packet
{
    int fd;
};

struct fxio_read_packet
{
    int fd;
    void *buffer;
    int size;
    void *intrData;
};

struct fxio_write_packet
{
    int fd;
    const void *buffer;
    int size;
    unsigned int unalignedDataLen;
    unsigned char unalignedData[64];
};

struct fxio_lseek_packet
{
    int fd;
    u32 offset;
    int whence;
};

struct fxio_lseek64_packet
{
    int fd;
    u32 offset_lo;
    u32 offset_hi;
    int whence;
};

struct fxio_chstat_packet
{
    char pathname[512];
    iox_stat_t *stat;
    int mask;
};

struct fxio_getstat_packet
{
    char pathname[512];
    iox_stat_t *stat;
};

struct fxio_format_packet
{
    char device[128];
    char blockDevice[512];
    char args[512];
    int arglen;
};

struct fxio_sync_packet
{
    char device[512];
    int flags;
};

struct fxio_dread_packet
{
    int fd;
    iox_dirent_t *dirent;
};

struct fxio_devctl_packet
{
    char name[CTL_BUF_SIZE];
    u8 arg[CTL_BUF_SIZE];
    int cmd;
    int arglen;
    void *buf;
    int buflen;
    void *intr_data;
};

struct fxio_ioctl_packet
{
    int fd;
    u8 arg[IOCTL_BUF_SIZE];
    int cmd;
};

struct fxio_ioctl2_packet
{
    int fd;
    u8 arg[CTL_BUF_SIZE];
    int cmd;
    int arglen;
    void *buf;
    int buflen;
    void *intr_data;
};

struct fxio_ctl_return_pkt
{
    void *dest;
    int len;
    u8 buf[CTL_BUF_SIZE];
    int padding[2];
};

struct fxio_lseek64_return_pkt
{
    u32 pos_lo, pos_hi;
};

struct fxio_rwbuff
{
    int size;
};

#endif /* __FILEXIO_H__ */
