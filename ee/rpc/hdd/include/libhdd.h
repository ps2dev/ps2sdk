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
*/

#ifndef _LIBHDD_H
#define _LIBHDD_H

#ifdef __cplusplus
extern "C" {
#endif

#define PFS_MT_ROBUST			0x02

#define FS_COMMON_PREFIX		'+'

#define FS_GROUP_SYSTEM			0x00
#define FS_GROUP_COMMON			0x01
#define FS_GROUP_APPLICATION	0x02

#define	FS_TYPE_EXT2			0x0083	
#define	FS_TYPE_EXT2_SWAP		0x0082
#define	FS_TYPE_PFS				0x0100
#define	FS_TYPE_EMPTY			0x0000

#define ATTR_MAIN_PARTITION		0x0000
#define ATTR_SUB_PARTITION		0x0001

#include "../../common/include/hdd-ioctl.h"

typedef struct {
	char name[32];			// Filesystem name
	char filename[40];		// Filename which can be used with fXioMount
	u32 size;				// Total filesystem size, in mega-bytes
	int formatted;			// 1 if filesystem is formatted, 0 otherwise
	u32 freeSpace;			// Reported free space, in mega-bytes
	int fileSystemGroup;	// Filesystem group (either system, common or application)
} t_hddFilesystem;

typedef struct {
	int hddSize;			// Total size of the HDD in mega-bytes
	int hddFree;			// Free space on the HDD in mega-bytes
	int hddMaxPartitionSize;// The maximum size allowed for a single partition, in mega-bytes
} t_hddInfo;

int hddCheckPresent();
int hddCheckFormatted();
int hddFormat();
int hddGetFilesystemList(t_hddFilesystem hddFs[], int maxEntries);
void hddGetInfo(t_hddInfo *info);
int hddMakeFilesystem(int fsSizeMB, char *name, int type);
int hddRemoveFilesystem(t_hddFilesystem *fs);
int hddExpandFilesystem(t_hddFilesystem *fs, int extraMB);

void hddPreparePoweroff();
void hddSetUserPoweroffCallback(void (*user_callback)(void *arg), void *arg);
void hddPowerOff();

#ifdef __cplusplus
}
#endif

#endif /* _LIBHDD_H */
