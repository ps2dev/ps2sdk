/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * ioctl(), ioctl2(), and devctl() definitions.
 */

#ifndef __SYS_IOCTL_H__
#define __SYS_IOCTL_H__

#include <tamtypes.h>

#define _IOC(type, nr) (((type) << 8) | (nr))

#define HDD_IOC_TYPE 'H'
#define HDD_CTL_TYPE 'h'

/* hddfsd ioctl2().  */
/** Add a subpartition.  */
#define HDDIOCADDSUB  _IOC(HDD_IOC_TYPE, 1)
/** Delete a subpartition.  */
#define HDDIOCDELSUB  _IOC(HDD_IOC_TYPE, 2)
/** Get the number of subpartitions.  */
#define HDDIOCGETSUBS _IOC(HDD_IOC_TYPE, 3)

/** Flush the HDD's cache.  */
#define HDDIOCFLUSH _IOC(HDD_IOC_TYPE, 4)

typedef struct
{
    /** Main or subpartition index.  */
    u32 sub;
    /** Start LBA.  */
    u32 lba;
    /** Number of sectors.  */
    u32 nsectors;
    /** Transfer direction, same as sceAtaDmaTransfer(). */
    int dir;
    /** Data buffer.  */
    void *buf;
} hdd_ioc_devio_t;

/** Write to partition data area.  */
#define HDDIOCDEVIO _IOC(HDD_IOC_TYPE, 50)

/** Size of main or subpartition.  */
#define HDDIOCGETSIZE  _IOC(HDD_IOC_TYPE, 51)
/** Set the last drive error.  */
#define HDDIOCSETERROR _IOC(HDD_IOC_TYPE, 52)
/** Get the last drive error.  */
#define HDDIOCGETERROR _IOC(HDD_IOC_TYPE, 53)

/* hddfsd devctl().  */
/** Get max. partition sectors.  */
#define HDDCTLGETMAXSECT   _IOC(HDD_CTL_TYPE, 1)
/** Get total number of sectors.  */
#define HDDCTLGETTOTALSECT _IOC(HDD_CTL_TYPE, 2)
/** Set the drive's idle timeout.  */
#define HDDCTLSETIDLE      _IOC(HDD_CTL_TYPE, 3)
/** Flush the HDD's cache.  */
#define HDDCTLFLUSH        _IOC(HDD_CTL_TYPE, 4)
/** Swap this partition for _)tmp.  */
#define HDDCTLSWAPTMP      _IOC(HDD_CTL_TYPE, 5)
/** Shutdown the drive.  */
#define HDDCTLSHUTDOWN     _IOC(HDD_CTL_TYPE, 6)
/** Get status of drive.  */
#define HDDCTLGETSTATUS    _IOC(HDD_CTL_TYPE, 7)
/** Get version of APA format.  */
#define HDDCTLGETFMTVER    _IOC(HDD_CTL_TYPE, 8)
/** Get SMART status for this drive.  */
#define HDDCTLGETSMARTSTAT _IOC(HDD_CTL_TYPE, 9)

/* hddfsd extended devctl().  */
/** Get the current date.  */
#define HDDCTLGETDATE    _IOC(HDD_IOC_TYPE, 50)
/** Not supported by hddfsd.irx!  */
#define HDDCTLINSTALLOSD _IOC(HDD_IOC_TYPE, 51)
/** Get the last drive error.  */
#define HDDCTLGETERROR   _IOC(HDD_IOC_TYPE, 52)
/** Get the id of the error partition.  */
#define HDDCTLGETERRORID _IOC(HDD_IOC_TYPE, 53)

typedef struct
{
    u32 lba;
    u32 nsectors;
    u8 buf[];
} hdd_ctl_driveio_t;

/** ATA device read.  */
#define HDDCTLDRIVEREAD   _IOC(HDD_IOC_TYPE, 54)
/** ATA device write.  */
#define HDDCTLDRIVEWRITE  _IOC(HDD_IOC_TYPE, 55)
/** Not supported by hddfsd.irx!  */
#define HDDCTLSCEIDENTIFY _IOC(HDD_IOC_TYPE, 56)

#endif /* __SYS_IOCTL_H__ */
