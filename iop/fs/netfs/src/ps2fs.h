/*
 * ps2fs.h - Protocol and packet definitions for ps2netfs.
 *
 * Copyright (C) 2003 Tord Lindstrom   (pukko@home.se)
 * Copyright (C) 2004 adresd <adresd_ps2dev@yahoo.com>
 *
 * Protocol is based on the ps2link fileio one, and is reproduced in an
 * expanded form with permission from : Tord Lindstrom (pukko@home.se)
 *
 * Licensed under the AFL v2.0. See the file LICENSE included with this
 * distribution for licensing terms.
 */

/** \defgroup ps2netfs ps2netfs - TCP fs driver*/ 

#ifndef _PS2FS_H_
#define _PS2FS_H_

#define PS2NETFS_LISTEN_PORT  0x4713

//from iomanx, all operations
//  open
#define PS2NETFS_OPEN_CMD     0xbeef8011
#define PS2NETFS_OPEN_RLY     0xbeef8012
//  close
#define PS2NETFS_CLOSE_CMD    0xbeef8021
#define PS2NETFS_CLOSE_RLY    0xbeef8022
//  read
#define PS2NETFS_READ_CMD     0xbeef8031
#define PS2NETFS_READ_RLY     0xbeef8032
//  write
#define PS2NETFS_WRITE_CMD    0xbeef8041
#define PS2NETFS_WRITE_RLY    0xbeef8042
//  lseek
#define PS2NETFS_LSEEK_CMD    0xbeef8051
#define PS2NETFS_LSEEK_RLY    0xbeef8052
//  ioctl
#define PS2NETFS_IOCTL_CMD    0xbeef8061
#define PS2NETFS_IOCTL_RLY    0xbeef8062
//  remove
#define PS2NETFS_REMOVE_CMD   0xbeef8071
#define PS2NETFS_REMOVE_RLY   0xbeef8072
//  mkdir
#define PS2NETFS_MKDIR_CMD    0xbeef8081
#define PS2NETFS_MKDIR_RLY    0xbeef8082
//  rmdir
#define PS2NETFS_RMDIR_CMD    0xbeef8091
#define PS2NETFS_RMDIR_RLY    0xbeef8092
//  dopen
#define PS2NETFS_DOPEN_CMD    0xbeef80A1
#define PS2NETFS_DOPEN_RLY    0xbeef80A2
//  dclose
#define PS2NETFS_DCLOSE_CMD   0xbeef80B1
#define PS2NETFS_DCLOSE_RLY   0xbeef80B2
//  dread
#define PS2NETFS_DREAD_CMD    0xbeef80C1
#define PS2NETFS_DREAD_RLY    0xbeef80C2
//  getstat
#define PS2NETFS_GETSTAT_CMD  0xbeef80D1
#define PS2NETFS_GETSTAT_RLY  0xbeef80D2
//  chstat
#define PS2NETFS_CHSTAT_CMD   0xbeef80E1
#define PS2NETFS_CHSTAT_RLY   0xbeef80E2
//  format
#define PS2NETFS_FORMAT_CMD   0xbeef80F1
#define PS2NETFS_FORMAT_RLY   0xbeef80F2
// extended commands
//  rename
#define PS2NETFS_RENAME_CMD   0xbeef8111
#define PS2NETFS_RENAME_RLY   0xbeef8112
//  chdir
#define PS2NETFS_CHDIR_CMD    0xbeef8121
#define PS2NETFS_CHDIR_RLY    0xbeef8122
//  sync
#define PS2NETFS_SYNC_CMD     0xbeef8131
#define PS2NETFS_SYNC_RLY     0xbeef8132
//  mount
#define PS2NETFS_MOUNT_CMD    0xbeef8141
#define PS2NETFS_MOUNT_RLY    0xbeef8142
//  umount
#define PS2NETFS_UMOUNT_CMD   0xbeef8151
#define PS2NETFS_UMOUNT_RLY   0xbeef8152
//  lseek64
#define PS2NETFS_LSEEK64_CMD  0xbeef8161
#define PS2NETFS_LSEEK64_RLY  0xbeef8162
//  devctl
#define PS2NETFS_DEVCTL_CMD   0xbeef8171
#define PS2NETFS_DEVCTL_RLY   0xbeef8172
//  symlink
#define PS2NETFS_SYMLINK_CMD  0xbeef8181
#define PS2NETFS_SYMLINK_RLY  0xbeef8182
//  readlink
#define PS2NETFS_READLINK_CMD 0xbeef8191
#define PS2NETFS_READLINK_RLY 0xbeef8192
//  ioctl2
#define PS2NETFS_IOCTL2_CMD   0xbeef81A1
#define PS2NETFS_IOCTL2_RLY   0xbeef81A2
// added on
//  info/status
#define PS2NETFS_INFO_CMD     0xbeef8F01
#define PS2NETFS_INFO_RLY     0xbeef8F02
//  fstype
#define PS2NETFS_FSTYPE_CMD   0xbeef8F11
#define PS2NETFS_FSTYPE_RLY   0xbeef8F12
//  devlist
#define PS2NETFS_DEVLIST_CMD  0xbeef8F21
#define PS2NETFS_DEVLIST_RLY  0xbeef8F22

#define PS2NETFS_MAX_PATH   256

typedef struct
{
    unsigned int cmd;
    unsigned short len;
} __attribute__((packed)) ps2netfs_pkt_hdr;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    unsigned int retval;
} __attribute__((packed)) ps2netfs_pkt_file_rly;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int flags;
    char path[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_open_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int fd;
} __attribute__((packed)) ps2netfs_pkt_close_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int fd;
    int nbytes;
} __attribute__((packed)) ps2netfs_pkt_read_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int retval;
    int nbytes;
} __attribute__((packed)) ps2netfs_pkt_read_rly;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int fd;
    int nbytes;
} __attribute__((packed)) ps2netfs_pkt_write_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int fd;
    int offset;
    int whence;
} __attribute__((packed)) ps2netfs_pkt_lseek_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int fd;
    int command;
} __attribute__((packed)) ps2netfs_pkt_ioctl_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int retval;
    char buf[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_ioctl_rly;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int fd;
} __attribute__((packed)) ps2netfs_pkt_dread_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int retval;
    unsigned int mode;
    unsigned int attr;
    unsigned int size;
    unsigned char ctime[8];
    unsigned char atime[8];
    unsigned char mtime[8];
    unsigned int hisize;
    char name[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_dread_rly;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int retval;
    int count;
    char list[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_info_rly;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int retval;
    int count;
    char list[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_devlist_rly;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    char fsname[PS2NETFS_MAX_PATH];
    char devname[PS2NETFS_MAX_PATH];
    int flag;
    char arg[PS2NETFS_MAX_PATH];
    int arglen;
} __attribute__((packed)) ps2netfs_pkt_mount_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int arglen;
    char dev[PS2NETFS_MAX_PATH];
    char blockdev[PS2NETFS_MAX_PATH];
    char arg[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_format_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int flags;
    char oldpath[PS2NETFS_MAX_PATH];
    char newpath[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_symlink_req;

typedef struct
{
    unsigned int cmd;
    unsigned short len;
    int retval;
    char path[PS2NETFS_MAX_PATH];
} __attribute__((packed)) ps2netfs_pkt_readlink_rly;

#endif
