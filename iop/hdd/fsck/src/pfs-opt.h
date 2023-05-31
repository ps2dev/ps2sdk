#ifndef _PFS_OPT_H
#define _PFS_OPT_H

#define PFS_PRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define PFS_DRV_NAME            "fsck"

#define PFS_NO_WRITE_ERROR_STAT 1
// #define FSCK100                 1 // If desired, uncomment to build a version of FSCK without the v1.10 features.

// Module version
#define PFS_MAJOR 1
#define PFS_MINOR 4

/*  Define PFS_OSD_VER in your Makefile to build an OSD version, which will:
    1. Enable the PIOCINVINODE IOCTL2 function.    */
#ifdef PFS_OSD_VER
#define PFS_IOCTL2_INC_CHECKSUM 1
#endif

#endif
