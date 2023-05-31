/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# PFS superblock manipulation routines
*/

#include <errno.h>
#include <stdio.h>
#ifdef _IOP
#include <sysclib.h>
#else
#include <string.h>
#endif
#include <hdd-ioctl.h>

#include "pfs-opt.h"
#include "libpfs.h"

u32 pfsBlockSize = 1;// block size scale in sectors (512). Note that 0 = 1x
u32 pfsMetaSize = 1024; // size of each metadata structure

int pfsCheckZoneSize(u32 zone_size)
{
	if((zone_size & (zone_size - 1)) || (zone_size < (2 * 1024)) || (zone_size > (128 * 1024)))
	{
		PFS_PRINTF(PFS_DRV_NAME": Error: Invalid zone size\n");
		return 0;
	}

	return 1;
}

#ifdef PFS_SUPPORT_BHDD
int pfsCheckExtendedZoneSize(u32 zone_size)
{
	// Note: in XOSD pfs IRX (compared to DVRP firmware), zone size upper bound is 1024 * 1024
	if ((zone_size & (zone_size - 1)) || (zone_size < (2 * 1024)) || (zone_size > (16384 * 1024)))
	{
		PFS_PRINTF(PFS_DRV_NAME": error: invalid  extended zone size %d,%d\n", (zone_size & (zone_size - 1)) == 0, zone_size);
		return 0;
	}
	
	return 1;
}
#endif

// Returns the number of sectors (512 byte units) which will be used
// for bitmaps, given the zone size and partition size
u32 pfsGetBitmapSizeSectors(int zoneScale, u32 partSize)
{
	int w, zones = partSize / (1 << zoneScale);

	w = (zones & 7);
	zones = zones / 8 + w;

	w = (zones & 511);
	return zones / 512 + w;
}

// Returns the number of blocks/zones which will be used for bitmaps
u32 pfsGetBitmapSizeBlocks(int scale, u32 mainsize)
{
	u32 a=pfsGetBitmapSizeSectors(scale, mainsize);
	return a / (1<<scale) + ((a % (1<<scale))>0);
}
