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
# Common HDD ioctls definition
*/

#ifndef _HDD_IOCTL_H
#define _HDD_IOCTL_H

///////////////////////////////////////////////////////////////////////////////
//	HDD.IRX

// Partition format/types (as returned via the mode field for getstat/dread)
#define APA_TYPE_FREE		0x0000
#define APA_TYPE_MBR		0x0001		// Master Boot Record
#define APA_TYPE_EXT2SWAP	0x0082
#define APA_TYPE_EXT2		0x0083
#define APA_TYPE_REISER		0x0088
#define APA_TYPE_PFS		0x0100
#define APA_TYPE_CFS		0x0101

#define APA_IDMAX		32
#define APA_MAXSUB		64		// Maximum # of sub-partitions
#define APA_PASSMAX		8
#define APA_FLAG_SUB		0x0001		// Sub-partition status for partitions (attr field)

//
// IOCTL2 commands
//
#define APA_IOCTL2_ADD_SUB		0x6801
#define APA_IOCTL2_DELETE_LAST_SUB	0x6802
#define APA_IOCTL2_NUMBER_OF_SUBS	0x6803
#define APA_IOCTL2_FLUSH_CACHE		0x6804

#define APA_IOCTL2_TRANSFER_DATA	0x6832	// used by pfs to read/write data :P
#define APA_IOCTL2_GETSIZE		0x6833	// for main(0)/subs(1+)
#define APA_IOCTL2_SET_PART_ERROR	0x6834	// set(sector of a apa header) that has a error :)
#define APA_IOCTL2_GET_PART_ERROR	0x6835	// get(sector of a apa header) that has a error

// I/O direction
#define APA_IO_MODE_READ		0x00
#define APA_IO_MODE_WRITE		0x01

// structs for IOCTL2 commands
typedef struct
{
	u32		sub;		// main(0)/subs(1+) to read/write
	u32		sector;
	u32		size;		// in sectors
	u32		mode;		// ATAD_MODE_READ/ATAD_MODE_WRITE.....
	void	*buffer;
} hddIoctl2Transfer_t;

//
// DEVCTL commands
//
#define APA_DEVCTL_MAX_SECTORS		0x4801	// max partition size(in sectors)
#define APA_DEVCTL_TOTAL_SECTORS	0x4802
#define APA_DEVCTL_IDLE			0x4803
#define APA_DEVCTL_FLUSH_CACHE		0x4804
#define APA_DEVCTL_SWAP_TMP		0x4805
#define APA_DEVCTL_DEV9_SHUTDOWN	0x4806
#define APA_DEVCTL_STATUS		0x4807
#define APA_DEVCTL_FORMAT		0x4808
#define APA_DEVCTL_SMART_STAT		0x4809
#define APA_DEVCTL_FREE_SECTORS		0x480A

#define APA_DEVCTL_GETTIME		0x6832
#define APA_DEVCTL_SET_OSDMBR		0x6833// arg = hddSetOsdMBR_t
#define APA_DEVCTL_GET_SECTOR_ERROR	0x6834
#define APA_DEVCTL_GET_ERROR_PART_NAME	0x6835// bufp = namebuffer[0x20]
#define APA_DEVCTL_ATA_READ		0x6836// arg  = hddAtaTransfer_t
#define APA_DEVCTL_ATA_WRITE		0x6837// arg  = hddAtaTransfer_t
#define APA_DEVCTL_SCE_IDENTIFY_DRIVE	0x6838// bufp = buffer for atadSceIdentifyDrive

// structs for DEVCTL commands

typedef struct
{
	u32 lba;
	u32 size;
	u8 data[0];
} hddAtaTransfer_t;

typedef struct
{
	u32 start;
	u32 size;
} hddSetOsdMBR_t;

//For backward-compatibility
// ioctl2 commands for ps2hdd.irx
#define HDDIO_ADD_SUB			APA_IOCTL2_ADD_SUB
#define HDDIO_DELETE_END_SUB		APA_IOCTL2_DELETE_LAST_SUB
#define HDDIO_NUMBER_OF_SUBS		APA_IOCTL2_NUMBER_OF_SUBS
#define HDDIO_FLUSH_CACHE		APA_IOCTL2_FLUSH_CACHE
#define HDDIO_GETSIZE			APA_IOCTL2_GETSIZE	// for main(0)/subs(1+)

// devctl commands for ps2hdd.irx
#define HDDCTL_MAX_SECTORS		APA_DEVCTL_MAX_SECTORS
#define HDDCTL_TOTAL_SECTORS		APA_DEVCTL_TOTAL_SECTORS
#define HDDCTL_IDLE			APA_DEVCTL_IDLE
#define HDDCTL_FLUSH_CACHE		APA_DEVCTL_FLUSH_CACHE
#define HDDCTL_SWAP_TMP			APA_DEVCTL_SWAP_TMP
#define HDDCTL_DEV9_SHUTDOWN		APA_DEVCTL_DEV9_SHUTDOWN
#define HDDCTL_STATUS			APA_DEVCTL_STATUS
#define HDDCTL_FORMAT			APA_DEVCTL_FORMAT
#define HDDCTL_SMART_STAT		APA_DEVCTL_SMART_STAT
#define HDDCTL_FREE_SECTORS		APA_DEVCTL_FREE_SECTORS


///////////////////////////////////////////////////////////////////////////////
//	PFS.IRX

// IOCTL2 commands
#define PFS_IOCTL2_ALLOC		0x7001
#define PFS_IOCTL2_FREE			0x7002
#define PFS_IOCTL2_ATTR_ADD		0x7003
#define PFS_IOCTL2_ATTR_DEL		0x7004
#define PFS_IOCTL2_ATTR_LOOKUP		0x7005
#define PFS_IOCTL2_ATTR_READ		0x7006

// DEVCTL commands
#define PFS_DEVCTL_GET_ZONE_SIZE	0x5001
#define PFS_DEVCTL_GET_ZONE_FREE	0x5002
#define PFS_DEVCTL_CLOSE_ALL		0x5003
#define PFS_DEVCTL_GET_STAT		0x5004
#define PFS_DEVCTL_CLEAR_STAT		0x5005

#define PFS_DEVCTL_SET_UID		0x5032
#define PFS_DEVCTL_SET_GID		0x5033

#define PFS_DEVCTL_SHOW_BITMAP		0xFF

// I/O direction
#define PFS_IO_MODE_READ		0x00
#define PFS_IO_MODE_WRITE		0x01

//For backward-compatibility
// ioctl2 commands for ps2fs.irx
#define PFSIO_ALLOC			PFS_IOCTL2_ALLOC
#define PFSIO_FREE			PFS_IOCTL2_FREE
#define PFSIO_ATTR_ADD			PFS_IOCTL2_ATTR_ADD
#define PFSIO_ATTR_DEL			PFS_IOCTL2_ATTR_DEL
#define PFSIO_ATTR_LOOKUP		PFS_IOCTL2_ATTR_LOOKUP
#define PFSIO_ATTR_READ			PFS_IOCTL2_ATTR_READ

// devctl commands for ps2fs.irx
#define PFSCTL_GET_ZONE_SIZE		PFS_DEVCTL_GET_ZONE_SIZE
#define PFSCTL_GET_ZONE_FREE		PFS_DEVCTL_GET_ZONE_FREE
#define PFSCTL_CLOSE_ALL		PFS_DEVCTL_CLOSE_ALL
#define PFSCTL_GET_STAT			PFS_DEVCTL_GET_STAT
#define PFSCTL_CLEAR_STAT		PFS_DEVCTL_CLEAR_STAT

#endif /* _HDD_IOCTL_H */
