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
 * Common definitions for libmc on the EE and IOP
 */

#ifndef __LIBMC_COMMON_H__
#define __LIBMC_COMMON_H__

#include <tamtypes.h>

typedef struct _sceMcStDateTime
{
    u8 Resv2;
    u8 Sec;
    u8 Min;
    u8 Hour;
    u8 Day;
    u8 Month;
    u16 Year;
} sceMcStDateTime;

/* MCMAN public structures */
typedef struct
{                             // size = 128
    int mode;                 // 0
    int length;               // 4
    s16 linked_block;         // 8
    char name[20];            // 10
    u8 field_1e;              // 30
    u8 field_1f;              // 31
    sceMcStDateTime created;  // 32
    int field_28;             // 40
    u16 field_2c;             // 44
    u16 field_2e;             // 46
    sceMcStDateTime modified; // 48
    int field_38;             // 56
    u8 unused2[65];           // 60
    u8 field_7d;              // 125
    u8 field_7e;              // 126
    u8 edc;                   // 127
} McFsEntryPS1;

typedef struct
{                             // size = 512
    u16 mode;                 // 0
    u16 unused;               // 2
    u32 length;               // 4
    sceMcStDateTime created;  // 8
    u32 cluster;              // 16
    u32 dir_entry;            // 20
    sceMcStDateTime modified; // 24
    u32 attr;                 // 32
    u32 unused2[7];           // 36
    char name[32];            // 64
    u8 unused3[416];          // 96
} McFsEntry;

typedef struct _MCCacheEntry
{
    int cluster;  // 0
    u8 *cl_data;  // 4
    u16 mc_slot;  // 8
    u8 wr_flag;   // 10
    u8 mc_port;   // 11
    u8 rd_flag;   // 12
    u8 unused[3]; // 13
} McCacheEntry;

/** file descriptor related mc command
 * used by: McInit, McClose, McSeek, McRead, McWrite, McGetinfo, McFormat, McFlush, McUnformat
 */
typedef struct
{                 // size = 48
    int fd;       // 0
    int port;     // 4
    int slot;     // 8
    int size;     // 12
    int offset;   // 16
    int origin;   // 20
    void *buffer; // 24
    void *param;  // 28
    u8 data[16];  // 32
} mcDescParam_t;

/** endParamenter struct
 * used by: McRead, McGetInfo, McReadPage
 */
typedef struct
{ // size = 64
    union
    {
        s32 size1; // 0
        s32 type;
    };
    union
    {
        s32 size2; // 4
        s32 free;
    };
    void *dest1;   // 8
    void *dest2;   // 12
    u8 src1[16];   // 16
    u8 src2[16];   // 32
    u8 unused[16]; // 48
} mcEndParam_t;

/** endParamenter2 struct
 * used by: McRead2, McGetInfo2
 */
typedef struct
{ // size = 192
    union
    {
        s32 size1; // 0
        s32 type;
    };
    union
    {
        s32 size2; // 4
        s32 free;
    };
    void *dest1; // 8
    void *dest2; // 12
    u8 src1[64]; // 16
    u8 src2[64]; // 80
    union
    {
        s32 formatted; // 144
        u8 unused[48];
    };
} mcEndParam2_t;

typedef struct
{
    s32 result;
    u32 mcserv_version;
    u32 mcman_version;
} mcRpcStat_t;

// in addition to errno
#define EFORMAT 140

// MCMAN basic error codes
#define sceMcResSucceed         0
#define sceMcResChangedCard     -1
#define sceMcResNoFormat        -2
#define sceMcResFullDevice      -3
#define sceMcResNoEntry         -4
#define sceMcResDeniedPermit    -5
#define sceMcResNotEmpty        -6
#define sceMcResUpLimitHandle   -7
#define sceMcResFailReplace     -8
#define sceMcResFailResetAuth   -11
#define sceMcResFailDetect      -12
#define sceMcResFailDetect2     -13
#define sceMcResDeniedPS1Permit -51
#define sceMcResFailAuth        -90

// Memory Card device types
#define sceMcTypeNoCard 0
#define sceMcTypePS1    1
#define sceMcTypePS2    2
#define sceMcTypePDA    3

/* High-Level File I/O */
// Used with the statmask field of chstat() to indicate which field(s) to change.
#define SCE_CST_MODE 0x01
#define SCE_CST_ATTR 0x02
#define SCE_CST_SIZE 0x04
#define SCE_CST_CT   0x08
#define SCE_CST_AT   0x10
#define SCE_CST_MT   0x20
#define SCE_CST_PRVT 0x40

// Used with the mode field of chstat() to indicate what to change.
#define SCE_STM_R 0x01
#define SCE_STM_W 0x02
#define SCE_STM_X 0x04
#define SCE_STM_C 0x08
#define SCE_STM_F 0x10
#define SCE_STM_D 0x20

/* file attributes */
#define sceMcFileAttrReadable    SCE_STM_R // Readable
#define sceMcFileAttrWriteable   SCE_STM_W // Writable
#define sceMcFileAttrExecutable  SCE_STM_X // Executable
#define sceMcFileAttrDupProhibit SCE_STM_C // Copy Protected
#define sceMcFileAttrFile        SCE_STM_F // Is a file.
#define sceMcFileAttrSubdir      SCE_STM_D // Is a sub-directory
#define sceMcFileCreateDir       0x0040    // Used internally to create directories.
#define sceMcFileAttrClosed      0x0080    // Indicates whether a file _may_ not have been written properly. Earlier browsers do not copy this flag.
#define sceMcFileCreateFile      0x0200    // Equivalent in value and functionality to O_CREAT.
#define sceMcFile0400            0x0400    // Set during creation.
#define sceMcFileAttrPDAExec     0x0800    // PDA Application (1st Generation PDA Download)
#define sceMcFileAttrPS1         0x1000    // PlayStation-format data
#define sceMcFileAttrHidden      0x2000    // Indicates whether the file is a hidden file (but not to the browser).
#define sceMcFileAttrExists      0x8000    // Indicates whether the file exists.

/* Valid information bit fields for sceMcSetFileInfo */
#define sceMcFileInfoCreate 0x01 // Creation Date/Time
#define sceMcFileInfoModify 0x02 // Modification Date/Time
#define sceMcFileInfoAttr   0x04 // File Attributes

#endif /* __LIBMC_COMMON_H__ */
