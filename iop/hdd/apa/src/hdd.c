/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Start Up routines
*/

#include <stdio.h>
#ifdef _IOP
#include <sysclib.h>
#include <loadcore.h>
#else
#include <string.h>
#include <stdlib.h>
#endif
#include <atad.h>
#include <dev9.h>
#include <errno.h>
#include <irx.h>
#include <iomanX.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include <libapa.h>
#include "hdd.h"
#include "hdd_fio.h"

#ifdef _IOP
IRX_ID("hdd_driver", APA_MODVER_MAJOR, APA_MODVER_MINOR);
#endif

static iop_device_ops_t hddOps={
	hddInit,
	hddDeinit,
	hddFormat,
	hddOpen,
	hddClose,
	hddRead,
	hddWrite,
	hddLseek,
	(void*)hddUnsupported,
	hddRemove,
	(void*)hddUnsupported,
	(void*)hddUnsupported,
	hddDopen,
	hddClose,
	hddDread,
	hddGetStat,
	(void*)hddUnsupported,
	hddReName,
	(void*)hddUnsupported,
	(void*)hddUnsupported,
	(void*)hddUnsupported,
	(void*)hddUnsupported,
	(void*)hddUnsupported,
	hddDevctl,
	(void*)hddUnsupported,
	(void*)hddUnsupported,
	hddIoctl2,
};
static iop_device_t hddFioDev={
	"hdd",
	IOP_DT_BLOCK | IOP_DT_FSEXT,
	1,
	"HDD",
	(struct _iop_device_ops *)&hddOps,
};

apa_device_t hddDevices[2]={
	{0, 0, 0, 3},
	{0, 0, 0, 3}
};

extern int apaMaxOpen;
extern hdd_file_slot_t *hddFileSlots;

static int inputError(char *input);
static int unlockDrive(s32 device);
static void hddShutdownCb(void);
static int hddInitError(void);

int hddCheckPartitionMax(s32 device, u32 size)
{
	return (hddDevices[device].partitionMaxSize >= size) ? 0 : -EINVAL;
}

apa_cache_t *hddAddPartitionHere(s32 device, const apa_params_t *params, u32 *emptyBlocks,
				u32 sector, int *err)
{
	apa_cache_t	*clink_this;
	apa_cache_t	*clink_next;
	apa_cache_t	*clink_new;
	apa_header_t	*header;
	u32			i;
	u32			tmp, some_size, part_end;
	u32			tempSize;

	// walk empty blocks in case can use one :)
	for(i=0;i< 32;i++)
	{
		if((u32)(1 << i) >= params->size && emptyBlocks[i]!=0)
			return apaInsertPartition(device, params, emptyBlocks[i], err);
	}
	clink_this=apaCacheGetHeader(device, sector, APA_IO_MODE_READ, err);
	header=clink_this->header;
	part_end=header->start+header->length;
	some_size=(part_end%params->size);
	tmp = some_size ? params->size - some_size : 0;

	if(hddDevices[device].totalLBA < (part_end + params->size + tmp)
		//Non-SONY: when dealing with large disks, this check may overflow (therefore, check for overflows!).
		|| (part_end < sector))
	{
		*err=-ENOSPC;
		apaCacheFree(clink_this);
		return NULL;
	}

	if((clink_next=apaCacheGetHeader(device, 0, APA_IO_MODE_READ, err))==NULL)
	{
		apaCacheFree(clink_this);
		return NULL;
	}

	tempSize=params->size;
	while(part_end%params->size)
	{
		tempSize=params->size>>1;
		while(0x3FFFF<tempSize)
		{
			if(!(part_end%tempSize))
			{
				clink_new=apaRemovePartition(device, part_end, 0,
					clink_this->header->start, tempSize);
				clink_this->header->next=part_end;
				clink_this->flags|=APA_CACHE_FLAG_DIRTY;
				clink_next->header->prev=clink_new->header->start;
				part_end+=tempSize;
				clink_next->flags|=APA_CACHE_FLAG_DIRTY;
				apaCacheFlushAllDirty(device);
				apaCacheFree(clink_this);
				clink_this=clink_new;
				break;
			}
			tempSize>>=1;
		}
	}
	if((clink_new=apaFillHeader(device, params, part_end, 0, clink_this->header->start,
		params->size, err))!=NULL)
	{
		clink_this->header->next=part_end;
		clink_this->flags|=APA_CACHE_FLAG_DIRTY;
		clink_next->header->prev=clink_new->header->start;
		clink_next->flags|=APA_CACHE_FLAG_DIRTY;
		apaCacheFlushAllDirty(device);
	}
	apaCacheFree(clink_this);
	apaCacheFree(clink_next);
	return clink_new;
}

static int inputError(char *input)
{
	APA_PRINTF(APA_DRV_NAME": Error: Usage: %s [-o <apaMaxOpen>] [-n <maxcache>]\n", input);
	return MODULE_NO_RESIDENT_END;
}

static void printStartup(void)
{
	APA_PRINTF(APA_DRV_NAME": PS2 APA Driver v%d.%d (c) 2003 Vector\n", APA_MODVER_MAJOR, APA_MODVER_MINOR);
	return;
}

static int unlockDrive(s32 device)
{
	u8 id[32];
	int rv;
	if((rv=apaGetIlinkID(id))==0)
		return ata_device_sce_sec_unlock(device, id);
	return rv;
}

int _start(int argc, char **argv)
{
	int 	i, ret;
	char	*input;
	int		cacheSize=3;
	apa_ps2time_t tm;
	ata_devinfo_t *hddInfo;

	printStartup();

	if((input=strrchr(argv[0], '/')))
		input++;
	else
		input=argv[0];

	argc--; argv++;
	while(argc)
	{
		if(argv[0][0] != '-')
			break;
		if(strcmp("-o", argv[0])==0)
		{
			argc--; argv++;
			if(!argc)
				return inputError(input);
			i=strtol(argv[0], 0, 10);
			if(i-1<32)
				apaMaxOpen=i;
		}
		else if(strcmp("-n", argv[0])==0)
		{
			argc--; argv++;
			if(!argc)
				return inputError(input);
			i=strtol(*argv, 0, 10);
			if(cacheSize<i)
				cacheSize=i;
		}
		argc--; argv++;
	}

	APA_PRINTF(APA_DRV_NAME": max open = %d, %d buffers\n", apaMaxOpen, cacheSize);
	if(dev9RegisterShutdownCb(0, &hddShutdownCb) != 0)
	{
		APA_PRINTF(APA_DRV_NAME": error: dev9 may not be resident.\n");
		return hddInitError();
	}

	if(apaGetTime(&tm) != 0)
	{
		APA_PRINTF(APA_DRV_NAME": error: could not get date.\n");
		return hddInitError();
	}

	APA_PRINTF(APA_DRV_NAME": %02d:%02d:%02d %02d/%02d/%d\n",
		tm.hour, tm.min, tm.sec, tm.month, tm.day, tm.year);
	for(i=0;i < 2;i++)
	{
		if(!(hddInfo=ata_get_devinfo(i)))
		{
			APA_PRINTF(APA_DRV_NAME": Error: ata initialization failed.\n");
			return hddInitError();
		}
		if(hddInfo->exists!=0 && hddInfo->has_packet==0)
		{
				hddDevices[i].status--;
				hddDevices[i].totalLBA=hddInfo->total_sectors;
				hddDevices[i].partitionMaxSize=apaGetPartitionMax(hddInfo->total_sectors);
				if(unlockDrive(i)==0)
					hddDevices[i].status--;
				APA_PRINTF(APA_DRV_NAME": disk%d: 0x%08lx sectors, max 0x%08lx\n", i,
					hddDevices[i].totalLBA, hddDevices[i].partitionMaxSize);
		}
	}
	hddFileSlots=apaAllocMem(apaMaxOpen*sizeof(hdd_file_slot_t));
	ret = (hddFileSlots == NULL) ? -ENOMEM : 0;
	if(ret != 0)
	{
		APA_PRINTF(APA_DRV_NAME": error: file descriptor initialization failed.\n");
		return hddInitError();
	}

	memset(hddFileSlots, 0, apaMaxOpen*sizeof(hdd_file_slot_t));

	if(apaCacheInit(cacheSize) != 0)
	{
		APA_PRINTF(APA_DRV_NAME": error: cache buffer initialization failed.\n");
		return hddInitError();
	}

	for(i=0;i < 2;i++)
	{
		if(hddDevices[i].status<2)
		{
			if(apaJournalRestore(i) != 0)
			{
				APA_PRINTF(APA_DRV_NAME": error: log check failed.\n");
				return hddInitError();
			}
			if(apaGetFormat(i, &hddDevices[i].format))
				hddDevices[i].status--;
			APA_PRINTF(APA_DRV_NAME": drive status %d, format version %08x\n",
				hddDevices[i].status, hddDevices[i].format);
		}
	}
	DelDrv("hdd");
	if(AddDrv(&hddFioDev) == 0)
	{
#ifdef APA_OSD_VER
		APA_PRINTF(APA_DRV_NAME": version %04x driver start. This is OSD version!\n", IRX_VER(APA_MODVER_MAJOR, APA_MODVER_MINOR));
#else
		APA_PRINTF(APA_DRV_NAME": version %04x driver start.\n", IRX_VER(APA_MODVER_MAJOR, APA_MODVER_MINOR));
#endif
		return MODULE_RESIDENT_END;
	}
	else
	{
		APA_PRINTF(APA_DRV_NAME": error: add device failed.\n");
		return hddInitError();
	}
}

static void hddShutdownCb(void)
{
	int i;

	for(i = 0; i < 2; i++)
	{
		if(hddDevices[i].status == 0)
			ata_device_smart_save_attr(i);
	}
}

static int hddInitError(void)
{
	dev9RegisterShutdownCb(0, NULL);
	return MODULE_NO_RESIDENT_END;
}
