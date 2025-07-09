/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Free space calculation routines
*/

#include <errno.h>
#include <iomanX.h>
#ifdef _IOP
#include <sysclib.h>
#else
#include <string.h>
#endif
#include <stdio.h>
#include <hdd-ioctl.h>

#include "libapa.h"

static void apaCalculateFreeSpace(u32 *free, u32 sectors)
{
    u32 maxsize = 0x1FFFFF; // 1GB
    u32 minsize = 0x3FFFF;  // 128MB
#ifdef APA_8MB_PARTITION_SIZE
    minsize = 0x3FFF; // 8Mb
#endif
    if (maxsize < sectors) {
        *free += sectors;
        return;
    }

    if ((*free & sectors) == 0) {
        *free |= sectors;
        return;
    }
    for (sectors /= 2; minsize < sectors; sectors /= 2)
        *free |= sectors;
}

int apaGetFreeSectors(s32 device, u32 *free, apa_device_t *deviceinfo)
{
	u32 sectors, partMax;
	int rv;
	apa_cache_t *clink;
    u32 minsize = 0x3FFFF;  // 128MB
#ifdef APA_8MB_PARTITION_SIZE
    minsize = 0x3FFF; // 8Mb
#endif

	sectors = 0;
	*free = 0;
	if((clink = apaCacheGetHeader(device, APA_SECTOR_MBR, APA_IO_MODE_READ, &rv)) != NULL)
	{
		do{
			if(clink->header->type == 0)
				apaCalculateFreeSpace(free, clink->header->length);
			sectors += clink->header->length;
		}while((clink = apaGetNextHeader(clink, &rv)) != NULL);
	}

	if(rv == 0)
	{
		for(partMax = deviceinfo[device].partitionMaxSize; minsize < partMax; partMax = deviceinfo[device].partitionMaxSize)
		{	//As weird as it looks, this was how it was done in the original HDD.IRX.
			for( ; minsize < partMax; partMax /= 2)
			{
				//Non-SONY: Perform 64-bit arithmetic here to avoid overflows when dealing with large disks.
				if((sectors % partMax == 0) && ((u64)sectors + partMax < deviceinfo[device].totalLBA))
				{
					apaCalculateFreeSpace(free, partMax);
					sectors += partMax;
					break;
				}
			}

			if(minsize >= partMax)
				break;
		}

		APA_PRINTF(APA_DRV_NAME": total = %08lx sectors, installable = %08lx sectors.\n", sectors, *free);
	}

	return rv;
}
