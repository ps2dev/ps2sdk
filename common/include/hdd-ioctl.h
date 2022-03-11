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
 * Common HDD IOCTL, DEVCTL and IOCTL2 command definitions
 */

#ifndef __HDD_IOCTL_H__
#define __HDD_IOCTL_H__

#include <tamtypes.h>

///////////////////////////////////////////////////////////////////////////////
// DEV9.IRX

#define DEV9_TYPE_PCMCIA 0
#define DEV9_TYPE_EXPBAY 1

#define DDIOC_MODEL   0x4401
#define DDIOC_OFF     0x4402
#define DDIOC_SETPIO3 0x4403
#define DDIOC_LED2CTL 0x4404

///////////////////////////////////////////////////////////////////////////////
// HDD.IRX

// Partition format/types (as returned via the mode field for getstat/dread)
#define APA_TYPE_FREE     0x0000
/** Master Boot Record */
#define APA_TYPE_MBR      0x0001
#define APA_TYPE_EXT2SWAP 0x0082
#define APA_TYPE_EXT2     0x0083
#define APA_TYPE_REISER   0x0088
#define APA_TYPE_PFS      0x0100
#define APA_TYPE_CFS      0x0101

#define APA_IDMAX    32
/** Maximum # of sub-partitions */
#define APA_MAXSUB   64
#define APA_PASSMAX  8
/** Sub-partition status for partitions (attr field) */
#define APA_FLAG_SUB 0x0001

//
// IOCTL2 commands
//
#define HIOCADDSUB 0x6801
#define HIOCDELSUB 0x6802
#define HIOCNSUB   0x6803
#define HIOCFLUSH  0x6804

// Arbitrarily-named commands
/** Used by PFS.IRX to read/write data */
#define HIOCTRANSFER     0x6832
/** For main(0)/subs(1+) */
#define HIOCGETSIZE      0x6833
/** Set (sector of a partition) that has an error */
#define HIOCSETPARTERROR 0x6834
/** Get (sector of a partition) that has an error */
#define HIOCGETPARTERROR 0x6835

// I/O direction
#define APA_IO_MODE_READ  0x00
#define APA_IO_MODE_WRITE 0x01

// structs for IOCTL2 commands
typedef struct
{
    /** main(0)/subs(1+) to read/write */
    u32 sub;
    u32 sector;
    /** in sectors */
    u32 size;
    /** ATAD_MODE_READ/ATAD_MODE_WRITE..... */
    u32 mode;
    void *buffer;
} hddIoctl2Transfer_t;

//
// DEVCTL commands
//
// 'H' set
/** Maximum partition size (in sectors) */
#define HDIOC_MAXSECTOR   0x4801
/** Capacity of the disk (in sectors) */
#define HDIOC_TOTALSECTOR 0x4802
#define HDIOC_IDLE        0x4803
#define HDIOC_FLUSH       0x4804
#define HDIOC_SWAPTMP     0x4805
#define HDIOC_DEV9OFF     0x4806
#define HDIOC_STATUS      0x4807
#define HDIOC_FORMATVER   0x4808
#define HDIOC_SMARTSTAT   0x4809
/** Returns the approximate amount of free space */
#define HDIOC_FREESECTOR  0x480A
#define HDIOC_IDLEIMM     0x480B

// 'h' command set
// Arbitrarily-named commands
#define HDIOC_GETTIME           0x6832
/** arg = hddSetOsdMBR_t */
#define HDIOC_SETOSDMBR         0x6833
#define HDIOC_GETSECTORERROR    0x6834
/** bufp = namebuffer[0x20] */
#define HDIOC_GETERRORPARTNAME  0x6835
/** arg  = hddAtaTransfer_t */
#define HDIOC_READSECTOR        0x6836
/** arg  = hddAtaTransfer_t */
#define HDIOC_WRITESECTOR       0x6837
/** bufp = buffer for atadSceIdentifyDrive */
#define HDIOC_SCEIDENTIFY       0x6838
// Only available using dvr_hdd0:
#define HDIOC_INSTSEC           0x6839
/** arg = u32 */
#define HDIOC_SETMAXLBA28       0x683A
#define HDIOC_GETMAXLBA48       0x683B
#define HDIOC_ISLBA48           0x683C
#define HDIOC_PRESETMAXLBA28    0x683D
#define HDIOC_POSTSETMAXLBA28   0x683E
#define HDIOC_ENABLEWRITECACHE  0x683F
#define HDIOC_DISABLEWRITECACHE 0x6840

// structs for DEVCTL commands

typedef struct
{
    u32 lba;
    u32 size;
    u8 data[];
} hddAtaTransfer_t;

typedef struct
{
    u32 start;
    u32 size;
} hddSetOsdMBR_t;

// For backward-compatibility
//  ioctl2 commands for ps2hdd.irx
#define HDDIO_ADD_SUB        HIOCADDSUB
#define HDDIO_DELETE_END_SUB HIOCDELSUB
#define HDDIO_NUMBER_OF_SUBS HIOCNSUB
#define HDDIO_FLUSH_CACHE    HIOCFLUSH
#define HDDIO_GETSIZE        HIOCGETSIZE

#define APA_IOCTL2_ADD_SUB         HIOCADDSUB
#define APA_IOCTL2_DELETE_LAST_SUB HIOCDELSUB
#define APA_IOCTL2_NUMBER_OF_SUBS  HIOCNSUB
#define APA_IOCTL2_FLUSH_CACHE     HIOCFLUSH

#define APA_IOCTL2_TRANSFER_DATA  HIOCTRANSFER
#define APA_IOCTL2_GETSIZE        HIOCGETSIZE
#define APA_IOCTL2_SET_PART_ERROR HIOCSETPARTERROR
#define APA_IOCTL2_GET_PART_ERROR HIOCGETPARTERROR

// devctl commands for ps2hdd.irx
#define HDDCTL_MAX_SECTORS   HDIOC_MAXSECTOR
#define HDDCTL_TOTAL_SECTORS HDIOC_TOTALSECTOR
#define HDDCTL_IDLE          HDIOC_IDLE
#define HDDCTL_FLUSH_CACHE   HDIOC_FLUSH
#define HDDCTL_SWAP_TMP      HDIOC_SWAPTMP
#define HDDCTL_DEV9_SHUTDOWN HDIOC_DEV9OFF
#define HDDCTL_STATUS        HDIOC_STATUS
#define HDDCTL_FORMAT        HDIOC_FORMATVER
#define HDDCTL_SMART_STAT    HDIOC_SMARTSTAT
#define HDDCTL_FREE_SECTORS  HDIOC_FREESECTOR

#define APA_DEVCTL_MAX_SECTORS   HDIOC_MAXSECTOR
#define APA_DEVCTL_TOTAL_SECTORS HDIOC_TOTALSECTOR
#define APA_DEVCTL_IDLE          HDIOC_IDLE
#define APA_DEVCTL_FLUSH_CACHE   HDIOC_FLUSH
#define APA_DEVCTL_SWAP_TMP      HDIOC_SWAPTMP
#define APA_DEVCTL_DEV9_SHUTDOWN HDIOC_DEV9OFF
#define APA_DEVCTL_STATUS        HDIOC_STATUS
#define APA_DEVCTL_FORMAT        HDIOC_FORMATVER
#define APA_DEVCTL_SMART_STAT    HDIOC_SMARTSTAT
#define APA_DEVCTL_FREE_SECTORS  HDIOC_FREESECTOR

#define APA_DEVCTL_GETTIME             HDIOC_GETTIME
#define APA_DEVCTL_SET_OSDMBR          HDIOC_SETOSDMBR
#define APA_DEVCTL_GET_SECTOR_ERROR    HDIOC_GETSECTORERROR
#define APA_DEVCTL_GET_ERROR_PART_NAME HDIOC_GETERRORPARTNAME
#define APA_DEVCTL_ATA_READ            HDIOC_READSECTOR
#define APA_DEVCTL_ATA_WRITE           HDIOC_WRITESECTOR
#define APA_DEVCTL_SCE_IDENTIFY_DRIVE  HDIOC_SCEIDENTIFY

///////////////////////////////////////////////////////////////////////////////
// PFS.IRX

// IOCTL2 commands
// Command set 'p'
#define PIOCALLOC      0x7001
#define PIOCFREE       0x7002
#define PIOCATTRADD    0x7003
#define PIOCATTRDEL    0x7004
#define PIOCATTRLOOKUP 0x7005
#define PIOCATTRREAD   0x7006
#define PIOCINVINODE   0x7032 // Only available in OSD version. Arbitrarily named.

// DEVCTL commands
// Command set 'P'
#define PDIOC_ZONESZ      0x5001
#define PDIOC_ZONEFREE    0x5002
#define PDIOC_CLOSEALL    0x5003
#define PDIOC_GETFSCKSTAT 0x5004
#define PDIOC_CLRFSCKSTAT 0x5005

// Arbitrarily-named commands
#define PDIOC_SHOWBITMAP 0xFF

// I/O direction
#define PFS_IO_MODE_READ  0x00
#define PFS_IO_MODE_WRITE 0x01

// For backward-compatibility
//  ioctl2 commands for ps2fs.irx
#define PFSIO_ALLOC       PIOCALLOC
#define PFSIO_FREE        PIOCFREE
#define PFSIO_ATTR_ADD    PIOCATTRADD
#define PFSIO_ATTR_DEL    PIOCATTRDEL
#define PFSIO_ATTR_LOOKUP PIOCATTRLOOKUP
#define PFSIO_ATTR_READ   PIOCATTRREAD

#define PFS_IOCTL2_ALLOC       PIOCALLOC
#define PFS_IOCTL2_FREE        PIOCFREE
#define PFS_IOCTL2_ATTR_ADD    PIOCATTRADD
#define PFS_IOCTL2_ATTR_DEL    PIOCATTRDEL
#define PFS_IOCTL2_ATTR_LOOKUP PIOCATTRLOOKUP
#define PFS_IOCTL2_ATTR_READ   PIOCATTRREAD

// devctl commands for ps2fs.irx
#define PFSCTL_GET_ZONE_SIZE PDIOC_ZONESZ
#define PFSCTL_GET_ZONE_FREE PDIOC_ZONEFREE
#define PFSCTL_CLOSE_ALL     PDIOC_CLOSEALL
#define PFSCTL_GET_STAT      PDIOC_GETFSCKSTAT
#define PFSCTL_CLEAR_STAT    PDIOC_CLRFSCKSTAT

#define PFS_DEVCTL_GET_ZONE_SIZE PDIOC_ZONESZ
#define PFS_DEVCTL_GET_ZONE_FREE PDIOC_ZONEFREE
#define PFS_DEVCTL_CLOSE_ALL     PDIOC_CLOSEALL
#define PFS_DEVCTL_GET_STAT      PDIOC_GETFSCKSTAT
#define PFS_DEVCTL_CLEAR_STAT    PDIOC_CLRFSCKSTAT

#define PFS_DEVCTL_SHOW_BITMAP PDIOC_SHOWBITMAP

#endif /* __HDD_IOCTL_H__ */
