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
*/

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <string.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopheap.h>
#include <malloc.h>

// PS2DRV includes
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fileXio_rpc.h"
#include "errno.h"

#include "libhdd.h"

#define PFS_ZONE_SIZE	8192
#define PFS_FRAGMENT	0x00000000

#define _OMIT_SYSTEM_PARTITION
//#define DEBUG


static void hddUpdateInfo();

static int hddStatusCurrent = 0;
static int hddSize;
static int hddFree;
static int hddMaxPartitionSize;

int hddCheckPresent()
{
	int rv;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	rv = fileXioDevctl("hdd0:", HDDCTL_STATUS, NULL, 0, NULL, 0);

	if((rv >= 3) || (rv < 0))
		return -1;
	else
		return 0;
}

int hddCheckFormatted()
{
	int rv;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	rv = fileXioDevctl("hdd0:", HDDCTL_STATUS, NULL, 0, NULL, 0);
	if((rv >= 1) || (rv < 0))
		return -1;
	else
		return 0;	
}


int hddFormat()
{
	int formatArg[3] = { PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT };
	int retVal;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	retVal = fileXioFormat("hdd0:", NULL, NULL, 0);
	if(retVal < 0)
		return retVal;

	retVal = fileXioFormat("pfs:", "hdd0:__net", (const char*)&formatArg, sizeof(formatArg));
	if(retVal < 0)
		return retVal;

	retVal = fileXioFormat("pfs:", "hdd0:__system", (const char*)&formatArg, sizeof(formatArg));
	if(retVal < 0)
		return retVal;

	retVal = fileXioFormat("pfs:", "hdd0:__common", (const char*)&formatArg, sizeof(formatArg));
	if(retVal < 0)
		return retVal;

	retVal = fileXioFormat("pfs:", "hdd0:__sysconf", (const char*)&formatArg, sizeof(formatArg));
	if(retVal < 0)
		return retVal;

	retVal = hddMakeFilesystem(512, "boot", FS_GROUP_SYSTEM);
	if(retVal < 0)
		return retVal;

	hddUpdateInfo();

	return 0;
}

int hddGetFilesystemList(t_hddFilesystem hddFs[], int maxEntries)
{
	iox_dirent_t dirEnt;
	int count = 0;
	u32 size = 0;
	int hddFd;
	int rv;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	hddFd = fileXioDopen("hdd0:");
	
	if(hddFd < 0)
		return hddFd;

	rv = fileXioDread(hddFd, &dirEnt);

	while((rv > 0) && (count < maxEntries))
	{
		int i;
		int partitionFd;
		u32 zoneFree, zoneSize;

		// We only want to know about main partitions (non-empty ones at that :P)
		if((dirEnt.stat.attr & ATTR_SUB_PARTITION) || (dirEnt.stat.mode == FS_TYPE_EMPTY))
		{
			rv = fileXioDread(hddFd, &dirEnt);
			continue;
		}

		memset(&hddFs[count], 0, sizeof(t_hddFilesystem));
		sprintf(hddFs[count].filename, "hdd0:%s", dirEnt.name);

		// Work out filesystem type
		if((dirEnt.name[0] == '_') && (dirEnt.name[1] == '_'))
		{
			hddFs[count].fileSystemGroup = FS_GROUP_SYSTEM;
			strcpy(hddFs[count].name, &dirEnt.name[2]);
		} 
		else if(dirEnt.name[0] == FS_COMMON_PREFIX)
		{
			hddFs[count].fileSystemGroup = FS_GROUP_COMMON;
			strcpy(hddFs[count].name, &dirEnt.name[1]);
		} 
		else
		{
			hddFs[count].fileSystemGroup = FS_GROUP_APPLICATION;
			strcpy(hddFs[count].name, dirEnt.name);
		}

#ifdef	_OMIT_SYSTEM_PARTITION
			if((hddFs[count].fileSystemGroup == FS_GROUP_SYSTEM) &&
				strcmp(hddFs[count].name, "boot"))
			{
				rv = fileXioDread(hddFd, &dirEnt);
				continue;
			}
#endif

#ifdef DEBUG
		printf("> Filename: %s\n> Name: %s\n> Type: %d\n", hddFs[count].filename, hddFs[count].name, hddFs[count].fileSystemGroup);
#endif

		// Calculate filesystem size
		partitionFd = fileXioOpen(hddFs[count].filename, O_RDONLY, 0);

		// If we failed to open the partition, then a password is probably set
		// (usually this means we have tried to access a game partition). We
		// dont want to return un-accessible game partitions in the filesystem list..
		if(partitionFd < 0)
		{
			rv = fileXioDread(hddFd, &dirEnt);
			continue;
		}

		for(i = 0, size = 0; i < dirEnt.stat.private_0 + 1; i++)
		{
			rv = fileXioIoctl2(partitionFd, HDDIO_GETSIZE, &i, 4, NULL, 0);
			size += rv * 512 / 1024 / 1024;
		}

		fileXioClose(partitionFd);

		hddFs[count].size = size;

		// Get filesystem free space & format status
		hddFs[count].freeSpace = 0;
		hddFs[count].formatted = 0;

		if(dirEnt.stat.mode == FS_TYPE_PFS)
		{
			rv = fileXioMount("pfs0:", hddFs[count].filename, FIO_MT_RDONLY);
			if(rv == 0)
			{

				zoneFree = fileXioDevctl("pfs0:", PFSCTL_GET_ZONE_FREE, NULL, 0, NULL, 0);
				zoneSize = fileXioDevctl("pfs0:", PFSCTL_GET_ZONE_SIZE, NULL, 0, NULL, 0);

				hddFs[count].freeSpace = zoneFree * zoneSize / 1024 / 1024;
				hddFs[count].formatted = 1;

				fileXioUmount("pfs0:");
			}
		}

#ifdef DEBUG
		printf("> Formatted: %d\n> Size: %d\n> Free: %d\n", hddFs[count].formatted, (int)hddFs[count].size, (int)hddFs[count].freeSpace);
#endif

		count++;
		rv = fileXioDread(hddFd, &dirEnt);
	}

	rv = fileXioDclose(hddFd);

	return count;
}

void hddGetInfo(t_hddInfo *info)
{
	hddUpdateInfo();	

	info->hddSize = hddSize;
	info->hddFree = hddFree;
	info->hddMaxPartitionSize = hddMaxPartitionSize;
}

static void hddUpdateInfo()
{
	iox_dirent_t infoDirEnt;
	int rv;
	int hddFd, hddUsed = 0;

	hddSize = fileXioDevctl("hdd0:", HDDCTL_TOTAL_SECTORS, NULL, 0, NULL, 0) * 512 / 1024 / 1024;

/* This seems to give in-accurate results
	fileXioDevctl("hdd0:", HDDCTL_FREE_SECTORS, NULL, 0, &rv, 4);
	hddFree = rv * 512 / 1024 / 1024;
*/
	hddFd = fileXioDopen("hdd0:");
	if(hddFd < 0) // For when a HDD is not connected!
		return;

	rv = fileXioDread(hddFd, &infoDirEnt);
	while(rv > 0)
	{
		if(infoDirEnt.stat.mode != FS_TYPE_EMPTY)
			hddUsed += infoDirEnt.stat.size * 512 / 1024 / 1024;

		rv = fileXioDread(hddFd, &infoDirEnt);
	}
	fileXioDclose(hddFd);
	hddFree = hddSize - hddUsed;

	hddMaxPartitionSize = fileXioDevctl("hdd0:", HDDCTL_MAX_SECTORS, NULL, 0, NULL, 0) * 512 / 1024 / 1024;

	hddStatusCurrent = 1;
}

static char *sizesString[9] = {
		"128M",
		"256M",
		"512M",
		"1G",
		"2G",
		"4G",
		"8G",
		"16G",
		"32G"
};

static int sizesMB[9] = {
		128,
		256,
		512,
		1024,
		2048,
		4096,
		8192,
		16384,
		32768
};

int hddMakeFilesystem(int fsSizeMB, char *name, int type)
{
	int formatArg[3] = { PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT };
	int maxIndex;
	int useIndex;
	int partSize;
	int fsSizeLeft = fsSizeMB;
	int partFd;
	char openString[256];
	char fsName[256];
	int retVal;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	if(fsSizeMB % 128)
		return -EINVAL;

	switch(type)
	{
		case FS_GROUP_SYSTEM:
			sprintf(fsName, "__%s", name);
			break;
		case FS_GROUP_COMMON:
			sprintf(fsName, "+%s", name);
			break;
		default:
			strcpy(fsName, name);
			break;
	}

	// Check if filesystem already exists
	sprintf(openString, "hdd0:%s", fsName);
	partFd = fileXioOpen(openString, O_RDONLY, 0);
	if(partFd > 0)	// Filesystem already exists
	{
		fileXioClose(partFd);
		return -1;
	}

	// Get index for max partition size
	for(maxIndex = 0; maxIndex < 9; maxIndex++)
		if(sizesMB[maxIndex] == hddMaxPartitionSize)
			break;

	// Get index of size we will use to create main partition
	for(useIndex = maxIndex; sizesMB[useIndex] > fsSizeMB; useIndex--);
		
	partSize = sizesMB[useIndex];
#ifdef DEBUG
	printf(">>> Attempting to create main partition, size %d MB\n", partSize);
#endif

	sprintf(openString, "hdd0:%s,%s", fsName, sizesString[useIndex]);
#ifdef DEBUG
	printf(">>> openString = %s\n", openString);
#endif

	partFd = fileXioOpen(openString, O_RDWR | O_CREAT, 0);
	if(partFd < 0)
		return partFd;

	fsSizeLeft -= partSize;
#ifdef DEBUG
	printf(">>> Main partition of %d MB created!\n", partSize);
#endif

	while(fsSizeLeft)
	{
		
		// Adjust size if necessary
		if(fsSizeLeft < partSize)
		{
#ifdef DEBUG
			printf(">>> Adjusting sub size: %d MB to ", sizesMB[useIndex]);
#endif
			for(useIndex = maxIndex; sizesMB[useIndex] > fsSizeLeft; useIndex--);
			partSize = sizesMB[useIndex];
			maxIndex = useIndex;
#ifdef DEBUG
			printf("%d MB\n", sizesMB[useIndex]);
#endif
		}

		// Try and allocate sub
#ifdef DEBUG
		printf(">>> Attempting to create sub partition of size %d MB\n", sizesMB[useIndex]);
#endif
		retVal = fileXioIoctl2(partFd, HDDIO_ADD_SUB, sizesString[useIndex], strlen(sizesString[useIndex]) + 1, NULL, 0);
		if(retVal == -ENOSPC)
		{
			// If sub alloc fails due to size, we decrease size and try again.
			// If we've run out of sizes, break the loop (give up)
			useIndex--;
			partSize = sizesMB[useIndex];
			maxIndex = useIndex;

			if(useIndex < 0)
			{
#ifdef DEBUG
				printf(">>> Out of sizes to try. Giving up.\n");
#endif
				break;
			}
#ifdef DEBUG
			printf(">>> Subpartition alloc FAILED! Trying with size of %d MB\n", partSize);
#endif

			continue;
		}
		// If we've reached the max number of subs, bail.
		else if(retVal == -EFBIG)
			break;
		else if(retVal >= 0)
		{
#ifdef DEBUG
			printf(">>> Sub creation successfull!\n");
#endif
		}
		else
		{
#ifdef DEBUG
			printf(">>> Unknown error while creating sub: %d\n", retVal);
#endif
		}

		fsSizeLeft -= sizesMB[useIndex];
	}

	fileXioClose(partFd);

	sprintf(openString, "hdd0:%s", fsName);
	retVal = fileXioFormat("pfs:", openString, (const char*)&formatArg, sizeof(formatArg));
	if(retVal < 0)
	{
#ifdef DEBUG
		printf(">>> Failed to format new partition: %d\n", retVal);
#endif
		return retVal;
	}

	hddUpdateInfo();

	return fsSizeMB - fsSizeLeft;
}

int hddRemoveFilesystem(t_hddFilesystem *fs)
{
	int rv;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	rv = fileXioRemove(fs->filename);

	hddUpdateInfo();

	return rv;
}

int hddExpandFilesystem(t_hddFilesystem *fs, int extraMB)
{
	int maxIndex;
	int useIndex;
	int partSize;
	int fsSizeLeft = extraMB;
	int partFd;
	int retVal;

	if(!hddStatusCurrent)
		hddUpdateInfo();

	if(extraMB % 128)
		return -EINVAL;

	// Get index for max partition size
	for(maxIndex = 0; maxIndex < 9; maxIndex++)
		if(sizesMB[maxIndex] == hddMaxPartitionSize)
			break;

	// Get index of size we will use to create new subs
	for(useIndex = maxIndex; sizesMB[useIndex] > extraMB; useIndex--);

	partSize = sizesMB[useIndex];

	// Open partition
	partFd = fileXioOpen(fs->filename, O_RDWR, 0);
	if(partFd < 0)
		return partFd;

	while(fsSizeLeft)
	{
		// Adjust size if necessary
		if(fsSizeLeft < partSize)
		{
#ifdef DEBUG
			printf(">>> Adjusting sub size: %d MB to ", sizesMB[useIndex]);
#endif
			for(useIndex = maxIndex; sizesMB[useIndex] > fsSizeLeft; useIndex--);
			partSize = sizesMB[useIndex];
			maxIndex = useIndex;
#ifdef DEBUG
			printf("%d MB\n", sizesMB[useIndex]);
#endif
		}

		// Try and allocate new sub
#ifdef DEBUG
		printf(">>> Attempting to create sub partition of size %d MB\n", sizesMB[useIndex]);
#endif
		retVal = fileXioIoctl2(partFd, HDDIO_ADD_SUB, sizesString[useIndex], strlen(sizesString[useIndex]) + 1, NULL, 0);
		if(retVal == -ENOSPC)
		{
			// If sub alloc fails due to size, we decrease size and try again.
			// If we've run out of sizes, break the loop (give up)
			useIndex--;
			partSize = sizesMB[useIndex];
			maxIndex = useIndex;

			if(useIndex < 0)
			{
#ifdef DEBUG
				printf(">>> Out of sizes to try. Giving up.\n");
#endif
				break;
			}
#ifdef DEBUG
			printf(">>> Subpartition alloc FAILED! Trying with size of %d MB\n", partSize);
#endif

			continue;
		}
		// If we've reached the max number of subs, bail.
		else if(retVal == -EFBIG)
			break;
		else if(retVal >= 0)
		{
#ifdef DEBUG
			printf(">>> Sub creation successfull!\n");
#endif
		}
		else
		{
#ifdef DEBUG
			printf(">>> Unknown error while creating sub: %d\n", retVal);
#endif
		}

		fsSizeLeft -= sizesMB[useIndex];
	}

	fileXioClose(partFd);

	hddUpdateInfo();

	return extraMB - fsSizeLeft;
}
