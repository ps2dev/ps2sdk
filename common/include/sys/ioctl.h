/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# ioctl(), ioctl2(), and devctl() definitions.
*/

#ifndef SYS_IOCTL_H
#define SYS_IOCTL_H

#define _IOC(type, nr)		(((type) << 8)|(nr))

#define DEV9_CTL_TYPE		'd'
#define HDD_IOC_TYPE		'H'
#define HDD_CTL_TYPE		'h'

/* DEV9 devctl().  */
#define DEV9CTLSHUTDOWN 	_IOC(DEV9_CTL_TYPE, 1)	/* Shutdown DEV9.  */
#define DEV9CTLTYPE		_IOC(DEV9_CTL_TYPE, 2)	/* Return type of device.  */

/* hddfsd ioctl2().  */
#define HDDIOCADDSUB		_IOC(HDD_IOC_TYPE, 1)	/* Add a subpartition.  */
#define HDDIOCDELSUB		_IOC(HDD_IOC_TYPE, 2)	/* Delete a subpartition.  */
#define HDDIOCGETSUBS		_IOC(HDD_IOC_TYPE, 3)	/* Get the number of subpartitions.  */

#define HDDIOCFLUSH		_IOC(HDD_IOC_TYPE, 4)	/* Flush the HDD's cache.  */

typedef struct {
	u32	sub;		/* Main or subpartition index.  */
	u32	lba;		/* Start LBA.  */
	u32	nsectors;	/* Number of sectors.  */
	int	dir;		/* Transfer direction, same as ata_device_dma_transfer(). */
	void	*buf;		/* Data buffer.  */
} hdd_ioc_devio_t;

#define HDDIOCDEVIO		_IOC(HDD_IOC_TYPE, 50)	/* Write to partition data area.  */

#define HDDIOCGETSIZE		_IOC(HDD_IOC_TYPE, 51)	/* Size of main or subpartition.  */
#define HDDIOCSETERROR		_IOC(HDD_IOC_TYPE, 52)	/* Set the last drive error.  */
#define HDDIOCGETERROR		_IOC(HDD_IOC_TYPE, 53)	/* Get the last drive error.  */

/* hddfsd devctl().  */
#define HDDCTLGETMAXSECT	_IOC(HDD_CTL_TYPE, 1)	/* Get max. partition sectors.  */
#define HDDCTLGETTOTALSECT	_IOC(HDD_CTL_TYPE, 2)	/* Get total number of sectors.  */
#define HDDCTLSETIDLE		_IOC(HDD_CTL_TYPE, 3)	/* Set the drive's idle timeout.  */
#define HDDCTLFLUSH		_IOC(HDD_CTL_TYPE, 4)	/* Flush the HDD's cache.  */
#define HDDCTLSWAPTMP		_IOC(HDD_CTL_TYPE, 5)	/* Swap this partition for _)tmp.  */
#define HDDCTLSHUTDOWN		_IOC(HDD_CTL_TYPE, 6)	/* Shutdown the drive.  */
#define HDDCTLGETSTATUS		_IOC(HDD_CTL_TYPE, 7)	/* Get status of drive.  */
#define HDDCTLGETFMTVER		_IOC(HDD_CTL_TYPE, 8)	/* Get version of APA format.  */
#define HDDCTLGETSMARTSTAT	_IOC(HDD_CTL_TYPE, 9)	/* Get SMART status for this drive.  */

/* hddfsd extended devctl().  */
#define HDDCTLGETDATE		_IOC(HDD_IOC_TYPE, 50)	/* Get the current date.  */
#define HDDCTLINSTALLOSD	_IOC(HDD_IOC_TYPE, 51)	/* Not supported by hddfsd.irx!  */
#define HDDCTLGETERROR		_IOC(HDD_IOC_TYPE, 52)	/* Get the last drive error.  */
#define HDDCTLGETERRORID	_IOC(HDD_IOC_TYPE, 53)  /* Get the id of the error partition.  */

typedef struct {
	u32	lba;
	u32	nsectors;
	u8	buf[0];
} hdd_ctl_driveio_t;

#define HDDCTLDRIVEREAD		_IOC(HDD_IOC_TYPE, 54)	/* ATA device read.  */
#define HDDCTLDRIVEWRITE	_IOC(HDD_IOC_TYPE, 55)	/* ATA device write.  */
#define HDDCTLSCEIDENTIFY	_IOC(HDD_IOC_TYPE, 56)	/* Not supported by hddfsd.irx!  */

#endif /* SYS_IOCTL_H */
