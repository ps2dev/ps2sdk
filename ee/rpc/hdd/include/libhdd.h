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
 * HDD library functions
 */

#ifndef __LIBHDD_H__
#define __LIBHDD_H__

#include <tamtypes.h>
#include <hdd-ioctl.h>
#include <libpwroff.h>

#define PFS_MT_ROBUST 0x02

#define FS_COMMON_PREFIX '+'

#define FS_GROUP_SYSTEM      0x00
#define FS_GROUP_COMMON      0x01
#define FS_GROUP_APPLICATION 0x02

#define FS_TYPE_EXT2_SWAP 0x0082
#define FS_TYPE_EXT2      0x0083
#define FS_TYPE_REISER    0x0088
#define FS_TYPE_PFS       0x0100
#define FS_TYPE_CFS       0x0101
#define FS_TYPE_EMPTY     0x0000

#define ATTR_MAIN_PARTITION 0x0000
#define ATTR_SUB_PARTITION  0x0001

typedef struct
{
    /** Filesystem name */
    char name[32];
    /** Filename which can be used with fXioMount */
    char filename[40];
    /** Total filesystem size, in mega-bytes */
    u32 size;
    /** 1 if filesystem is formatted, 0 otherwise */
    int formatted;
    /** Reported free space, in mega-bytes */
    u32 freeSpace;
    /** Filesystem group (either system, common or application) */
    int fileSystemGroup;
} t_hddFilesystem;

typedef struct
{
    /** Total size of the HDD in mega-bytes */
    u32 hddSize;
    /** Free space on the HDD in mega-bytes */
    u32 hddFree;
    /** The maximum size allowed for a single partition, in mega-bytes */
    u32 hddMaxPartitionSize;
} t_hddInfo;

#ifdef __cplusplus
extern "C" {
#endif

int hddCheckPresent();
int hddCheckFormatted();
int hddFormat();
int hddGetFilesystemList(t_hddFilesystem hddFs[], int maxEntries);
void hddGetInfo(t_hddInfo *info);
int hddMakeFilesystem(int fsSizeMB, char *name, int type);
int hddRemoveFilesystem(t_hddFilesystem *fs);
int hddExpandFilesystem(t_hddFilesystem *fs, int extraMB);

#ifdef __cplusplus
}
#endif

// These hdd* functions are deprecated
// Use the poweroff* version instead

#define hddPreparePoweroff         poweroffInit
#define hddSetUserPoweroffCallback poweroffSetCallback
#define hddPowerOff                poweroffShutdown

#endif /* __LIBHDD_H__ */
