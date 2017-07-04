#include <stdio.h>
#include <errno.h>

#ifdef WIN32
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#else
#include <cdvdman.h>
#include <sysclib.h>
#endif

#include <usbhdfsd.h>
#include "usbhd_common.h"
#include "scache.h"
#include "mass_stor.h"

//#define DEBUG  //comment out this line when not debugging

#include "mass_debug.h"

#define READ_SECTOR(d, a, b)	scache_readSector((d)->cache, (a), (b))
#define WRITE_SECTOR(d, a)	scache_writeSector((d)->cache, (a))
#define FLUSH_SECTORS(d)	scache_flushSectors((d)->cache)

int UsbMassReadSector(fat_driver *fatd, void **buffer, u32 sector)
{
	return READ_SECTOR(fatd->dev, sector, buffer);
}

int UsbMassWriteSector(fat_driver *fatd, u32 sector)
{
	return WRITE_SECTOR(fatd->dev, sector);
}

void UsbMassFlushCache(fat_driver *fatd)
{
	FLUSH_SECTORS(fatd->dev);
}
