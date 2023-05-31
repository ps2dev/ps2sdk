#ifndef PFS_OPT
#define PFS_OPT

#define PFS_PRINTF(args, ...) printf(args, ##__VA_ARGS__)
#define PFS_DRV_NAME          "fssk"

// Module version
#define PFS_MAJOR 1
#define PFS_MINOR 4

/*  Define PFS_OSD_VER in your Makefile to build an OSD version, which will:
    1. Enable the PIOCINVINODE IOCTL2 function.    */
#ifdef PFS_OSD_VER
#define PFS_IOCTL2_INC_CHECKSUM 1
#endif

#endif
