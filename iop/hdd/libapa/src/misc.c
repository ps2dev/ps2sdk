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
#ifdef _IOP
#include <intrman.h>
#include <cdvdman.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#else
#include <string.h>
#include <time.h>
#include <malloc.h>
#endif
#include <iomanX.h>
#include <stdio.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include "libapa.h"

void *apaAllocMem(int size)
{
#ifdef _IOP
	int intrStat;
	void *mem;

	CpuSuspendIntr(&intrStat);
	mem = AllocSysMemory(ALLOC_FIRST, size, NULL);
	if(mem == NULL)
		APA_PRINTF(APA_DRV_NAME": error: out of memory\n");
	CpuResumeIntr(intrStat);

	return mem;
#else
	return malloc(size);
#endif
}

void apaFreeMem(void *ptr)
{
#ifdef _IOP
	int intrStat;

	CpuSuspendIntr(&intrStat);
	FreeSysMemory(ptr);
	CpuResumeIntr(intrStat);
#else
	free(ptr);
#endif
}

int apaGetTime(apa_ps2time_t *tm)
{
#ifdef _IOP
	int ret, i;
	sceCdCLOCK	cdtime;
	static apa_ps2time_t timeBuf={
		0, 7, 6, 5, 4, 3, 2000	// used if can not get time...
	};

	for(i = 0; i < 20; i++)
	{
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

	memcpy(tm, &timeBuf, sizeof(apa_ps2time_t));
#else
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo=localtime (&rawtime);

	tm->sec=timeinfo->tm_sec;
	tm->min=timeinfo->tm_min;
	tm->hour=timeinfo->tm_hour;
	tm->day=timeinfo->tm_mday;
	tm->month=timeinfo->tm_mon;
	tm->year=timeinfo->tm_year+1900;
#endif

	return 0;
}

int apaGetIlinkID(u8 *idbuf)
{
#ifdef _IOP
	u32 stat;
	int i;

	for(i = 0; ; i++)
	{
		stat=0;
		memset(idbuf, 0, 32);
		if((sceCdRI(idbuf, &stat) != 0) && (stat == 0))
		{
			return 0;
		}

		if(i >= 20)
			break;

		DelayThread(100000);
	}

	APA_PRINTF(APA_DRV_NAME": Error: cannot get id\n");
	return -EIO;
#else
	memset(idbuf, 0, 32);
	return 0;
#endif
}
