/*
 * journal.c - APA journal related routines
 *
 * Copyright (c) 2003 Vector
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "hdd.h"

//  Globals
apa_journal_t journalBuf;


int journalFlush(u32 device)
{// this write any thing that in are journal buffer :)
	if(atadFlushCache(device))
		return -EIO;
	if(atadDmaTransfer(device, &journalBuf, APA_SECTOR_APAL, 1, ATAD_MODE_WRITE))
		return -EIO;
	if(atadFlushCache(device))
		return -EIO;
	return 0;
}

int journalReset(u32 device)
{
	memset(&journalBuf, 0, sizeof(apa_journal_t));
	journalBuf.magic=APAL_MAGIC;
	return journalFlush(device);
}

int journalWrite(apa_cache *clink)
{
	clink->header->checksum=journalCheckSum(clink->header);
	if(atadDmaTransfer(clink->device, clink->header, 
		(journalBuf.num << 1)+APA_SECTOR_APAL_HEADERS, 2, ATAD_MODE_WRITE))
			return -EIO;
	journalBuf.sectors[journalBuf.num]=clink->sector;
	journalBuf.num++;
	return 0;
}

int journalResetore(u32 device)
{	// copys apa headers from apal to apa system
	int i;
	u32 sector;
	apa_cache *clink;

	dprintf1("ps2hdd: checking log...\n");
	if(atadDmaTransfer(device, &journalBuf, APA_SECTOR_APAL, sizeof(apa_journal_t)/512, ATAD_MODE_READ)){
		journalReset(device);
		return -EIO;
	}
	if(journalBuf.magic==APAL_MAGIC)
	{
		if(journalBuf.num==0)
			return 0;
		clink=cacheGetFree();
		for(i=0, sector=APA_SECTOR_APAL_HEADERS;i<journalBuf.num;i++, sector+=2)
		{
			if(atadDmaTransfer(device, clink->header, sector, 2, ATAD_MODE_READ))
				break;
			if(atadDmaTransfer(device, clink->header, journalBuf.sectors[i], 2, ATAD_MODE_WRITE))
				break;
		}
		cacheAdd(clink);
		return journalReset(device);// only do if journal..
	}
	memset(&journalBuf, 0, sizeof(apa_journal_t));// safe e
	journalBuf.magic=APAL_MAGIC;
	return 0;//-EINVAL;
}
