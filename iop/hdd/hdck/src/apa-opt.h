#ifndef _APA_OPT_H
#define _APA_OPT_H

#define APA_PRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define APA_DRV_NAME            "hdck"

/*  Define APA_OSD_VER in your Makefile to build an OSD version, which will:
    1. (currently disabled) When formatting, do not create any partitions other than __mbr.
    2. __mbr will be formatted with its password.
    3. All partitions can be accessed, even without the right password.
    4. The starting LBA of the partition will be returned in
        the private_5 field of the stat structure (returned by getstat and dread). */

#ifdef APA_OSD_VER
#define APA_STAT_RETURN_PART_LBA   1
#define APA_FORMAT_LOCK_MBR        1
#define APA_FORMAT_MAKE_PARTITIONS 1 // For now, define this because I don't think we're ready (and want to) deal with the official passwords.
#else
#define APA_ENABLE_PASSWORDS       1
#define APA_FORMAT_MAKE_PARTITIONS 1
#endif

// Module version
#define APA_MODVER_MAJOR 1
#define APA_MODVER_MINOR 4

#endif
