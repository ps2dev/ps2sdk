/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Miscellaneous routines
*/

#include <errno.h>
#include <stdio.h>
#include <iomanX.h>
#ifdef _IOP
#include <intrman.h>
#include <sysmem.h>
#include <sysclib.h>
#include <thbase.h>
#include <cdvdman.h>
#else
#include <string.h>
#include <stdlib.h>
#include <time.h>
#endif
#include <ctype.h>
#include <hdd-ioctl.h>

#include "pfs-opt.h"
#include "libpfs.h"

///////////////////////////////////////////////////////////////////////////////
//   Function definitions

void *pfsAllocMem(int size)
{
#ifdef _IOP
	int intrStat;
	void *mem;

	CpuSuspendIntr(&intrStat);
	mem = AllocSysMemory(ALLOC_FIRST, size, NULL);
	CpuResumeIntr(intrStat);

	return mem;
#else
	return malloc(size);
#endif
}

void pfsFreeMem(void *buffer)
{
#ifdef _IOP
	int OldState;

	CpuSuspendIntr(&OldState);
	FreeSysMemory(buffer);
	CpuResumeIntr(OldState);
#else
	free(buffer);
#endif
}

int pfsGetTime(pfs_datetime_t *tm)
{
#ifdef _IOP
	int i;
	sceCdCLOCK	cdtime;
	static pfs_datetime_t timeBuf={
		0, 7, 6, 5, 4, 3, 2000	// used if can not get time...
	};

	for(i = 0; i < 20; i++)
	{
		int ret;

		ret = sceCdReadClock(&cdtime);

		if(ret!=0 && cdtime.stat==0)
		{
			timeBuf.sec=btoi(cdtime.second);
			timeBuf.min=btoi(cdtime.minute);
			timeBuf.hour=btoi(cdtime.hour);
			timeBuf.day=btoi(cdtime.day);
			timeBuf.month=btoi(cdtime.month & 0x7F);	//The old CDVDMAN sceCdReadClock() function does not automatically file off the highest bit.
			timeBuf.year=btoi(cdtime.year) + 2000;
			break;
		} else {
			if(!(cdtime.stat & 0x80))
				break;
		}

		DelayThread(100000);
	}
	memcpy(tm, &timeBuf, sizeof(pfs_datetime_t));
#else
	time_t rawtime;
	struct tm timeinfo;
	time(&rawtime);
	// Convert to JST
	rawtime += (-9 * 60 * 60);
#ifdef _WIN32
	gmtime_s(&timeinfo, &rawtime);
#else
	gmtime_r(&rawtime, &timeinfo);
#endif

	tm->sec = timeinfo.tm_sec;
	tm->min = timeinfo.tm_min;
	tm->hour = timeinfo.tm_hour;
	tm->day = timeinfo.tm_mday;
	tm->month = timeinfo.tm_mon + 1;
	tm->year = timeinfo.tm_year + 1900;
#endif

	return 0;
}

int pfsFsckStat(pfs_mount_t *pfsMount, pfs_super_block_t *superblock,
	u32 stat, int mode)
{	// mode 0=set flag, 1=remove flag, else check stat

	if(pfsMount->blockDev->transfer(pfsMount->fd, superblock, 0, PFS_SUPER_SECTOR, 1,
		PFS_IO_MODE_READ)==0)
	{
		switch(mode)
		{
			case PFS_MODE_SET_FLAG:
				superblock->pfsFsckStat|=stat;
				break;
			case PFS_MODE_REMOVE_FLAG:
				superblock->pfsFsckStat&=~stat;
				break;
			default/*PFS_MODE_CHECK_FLAG*/:
				return 0 < (superblock->pfsFsckStat & stat);
		}
		pfsMount->blockDev->transfer(pfsMount->fd, superblock, 0, PFS_SUPER_SECTOR, 1,
			PFS_IO_MODE_WRITE);
		pfsMount->blockDev->flushCache(pfsMount->fd);
	}
	return 0;
}

void pfsPrintBitmap(const u32 *bitmap) {
	u32 i, j;
	char a[48+1], b[16+1];

	b[16]=0;
	for (i=0; i < 32; i++){
		memset(a, 0, 49);
		for (j=0; j < 16; j++){
			const char *c=(const char*)bitmap+j+i*16;

			sprintf(a+j*3, "%02x ", *c);
			b[j] = ((*c>=0) && (isgraph(*c))) ?
				*c : '.';
		}
		PFS_PRINTF("%s%s\n", a, b);
	}
}

u32 pfsGetScale(u32 num, u32 size)
{
	u32 scale = 0;

	while((size << scale) != num)
		scale++;

	return scale;
}

u32 pfsFixIndex(u32 index)
{
	if(index < PFS_INODE_MAX_BLOCKS)
		return index;

	index -= PFS_INODE_MAX_BLOCKS;
	return index % 123;
}

///////////////////////////////////////////////////////////////////////////////
//   Functions to work with hdd.irx

static int pfsHddTransfer(int fd, void *buffer, u32 sub/*0=main 1+=subs*/, u32 sector,
		u32 size/* in sectors*/, u32 mode);
static u32 pfsHddGetSubCount(int fd);
static u32 pfsHddGetPartSize(int fd, u32 sub/*0=main 1+=subs*/);
static void pfsHddSetPartError(int fd);
static int pfsHddFlushCache(int fd);

#ifdef PFS_SUPPORT_BHDD
#define NUM_SUPPORTED_DEVICES	2
#else
#define NUM_SUPPORTED_DEVICES	1
#endif
pfs_block_device_t pfsBlockDeviceCallTable[NUM_SUPPORTED_DEVICES] = {
	{
		"hdd",
		&pfsHddTransfer,
		&pfsHddGetSubCount,
		&pfsHddGetPartSize,
		&pfsHddSetPartError,
		&pfsHddFlushCache,
	},
#ifdef PFS_SUPPORT_BHDD
	{
		"bhdd",
		&pfsHddTransfer,
		&pfsHddGetSubCount,
		&pfsHddGetPartSize,
		&pfsHddSetPartError,
		&pfsHddFlushCache,
	},
#endif
};

pfs_block_device_t *pfsGetBlockDeviceTable(const char *name)
{
	char *end;
	char devname[32];
	char *tmp;
	u32 len;
	int i;

	while(name[0] == ' ')
		name++;

	end = strchr(name, ':');
	if(!end) {
		PFS_PRINTF(PFS_DRV_NAME": Error: Unknown block device '%s'\n", name);
		return NULL;
	}

	len = (u8*)end - (u8*)name;
	strncpy(devname, name, len);
	devname[len] = '\0';

	// Loop until digit is found, then terminate string at that digit.
	// Should then have just the device name left, minus any front spaces or trailing digits.
	tmp = devname;
	while(!(isdigit(tmp[0])))
		tmp++;
	tmp[0] = '\0';

	for(i = 0; i < NUM_SUPPORTED_DEVICES; i++)
		if(!strcmp(pfsBlockDeviceCallTable[i].devName, devname))
			return &pfsBlockDeviceCallTable[i];

	return NULL;
}

static int pfsHddTransfer(int fd, void *buffer, u32 sub/*0=main 1+=subs*/, u32 sector,
				u32 size/* in sectors*/, u32 mode)
{
	hddIoctl2Transfer_t t;

	t.sub=sub;
	t.sector=sector;
	t.size=size;
	t.mode=mode;
	t.buffer=buffer;

	return iomanX_ioctl2(fd, HIOCTRANSFER, &t, 0, NULL, 0);
}

static u32 pfsHddGetSubCount(int fd)
{
	return iomanX_ioctl2(fd, HIOCNSUB, NULL, 0, NULL, 0);
}

static u32 pfsHddGetPartSize(int fd, u32 sub/*0=main 1+=subs*/)
{	// of a partition
	return iomanX_ioctl2(fd, HIOCGETSIZE, &sub, 0, NULL, 0);
}

static void pfsHddSetPartError(int fd)
{
	iomanX_ioctl2(fd, HIOCSETPARTERROR, NULL, 0, NULL, 0);
}

static int pfsHddFlushCache(int fd)
{
	return iomanX_ioctl2(fd,HIOCFLUSH, NULL, 0, NULL, 0);
}
