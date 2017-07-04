#ifndef _PFS_OPT_H
#define _PFS_OPT_H

#define PFS_PRINTF(format,...)	printf(format, ##__VA_ARGS__)
#define PFS_DRV_NAME		"pfs"

#define PFS_MAJOR	2
#define PFS_MINOR	2

/*	Define PFS_OSD_VER in your Makefile to build an OSD version, which will:
	1. Enable the PIOCINVINODE IOCTL2 function.
	2. (Unofficial) Return the LBA of the inode in private_5 and sub number in private_4 fields of the stat structure (returned by getstat and dread). */
#ifdef PFS_OSD_VER
#define PFS_IOCTL2_INC_CHECKSUM		1
#define PFS_STAT_RETURN_INODE_LBA	1
#endif

#endif
