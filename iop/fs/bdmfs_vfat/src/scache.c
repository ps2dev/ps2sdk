/*
 * scache.c - USB Mass storage driver for PS2
 *
 * (C) 2004, Marek Olejnik (ole00@post.cz)
 * (C) 2004  Hermes (support for sector sizes from 512 to 4096 bytes)
 *
 * Sector cache
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */
//---------------------------------------------------------------------------
#include <stdio.h>

#ifdef WIN32
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#else
#include <sysmem.h>
#include <tamtypes.h>
#endif

//#define SCACHE_RECORD_STATS	1

#include "common.h"
#include "scache.h"

//---------------------------------------------------------------------------
#define READ_SECTOR(d, s, a, c) (d)->bd->read((d)->bd, s, a, c)
#define WRITE_SECTOR(d, s, a, c) (d)->bd->write((d)->bd, s, a, c)

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define BLOCK_SIZE (4 * 1024)

//when the flushCounter reaches FLUSH_TRIGGER then flushSectors is called
//#define FLUSH_TRIGGER 16

//---------------------------------------------------------------------------
static void initRecords(cache_set* cache)
{
    unsigned int i;

    for (i = 0; i < CACHE_SIZE; i++) {
        cache->rec[i].sector     = 0xFFFFFFF0;
        cache->rec[i].tax        = 0;
        cache->rec[i].writeDirty = 0;
    }

    cache->writeFlag = 0;
}

//---------------------------------------------------------------------------
/* search cache records for the sector number stored in cache
  returns cache record (slot) number
 */
static int getSlot(cache_set* cache, unsigned int sector)
{
    int i;

    for (i = 0; i < CACHE_SIZE; i++) {
        if (sector >= cache->rec[i].sector && sector < (cache->rec[i].sector + cache->indexLimit)) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------------------------
/* search cache records for the sector number stored in cache */
static int getIndexRead(cache_set* cache, unsigned int sector)
{
    unsigned int i;
    int index = -1;

    for (i = 0; i < CACHE_SIZE; i++) {
        if (sector >= cache->rec[i].sector && sector < (cache->rec[i].sector + cache->indexLimit)) {
            if (cache->rec[i].tax < 0)
                cache->rec[i].tax = 0;
            cache->rec[i].tax += 2;
            index = i;
        }
        if (cache->rec[i].tax > 1)
            cache->rec[i].tax--; //apply tax penalty
    }
    if (index < 0)
        return index;
    else
        return ((index * cache->indexLimit) + (sector - cache->rec[index].sector));
}

//---------------------------------------------------------------------------
/* select the best record where to store new sector */
static int getIndexWrite(cache_set* cache, unsigned int sector)
{
    int ret;
    int minTax = 0x0FFFFFFF;
    unsigned int i, index = 0;

    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache->rec[i].tax < minTax) {
            index  = i;
            minTax = cache->rec[i].tax;
        }
    }

    //this sector is dirty - we need to flush it first
    if (cache->rec[index].writeDirty) {
        M_DEBUG("scache: getIndexWrite: sector is dirty : %u   index=%d \n", cache->rec[index].sector, index);
        ret = WRITE_SECTOR(cache, cache->rec[index].sector, cache->sectorBuf + (index * BLOCK_SIZE), BLOCK_SIZE / cache->sectorSize);
        if (ret < 0) {
            M_PRINTF("scache: ERROR writing sector to disk! sector=%u\n", sector);
            return ret;
        }

        cache->rec[index].writeDirty = 0;
    }
    cache->rec[index].tax += 2;
    cache->rec[index].sector = sector;

    return index * cache->indexLimit;
}

//---------------------------------------------------------------------------
/*
	flush dirty sectors
 */
int scache_flushSectors(cache_set* cache)
{
    unsigned int i;
    int counter = 0, ret;

    M_DEBUG("scache: flushSectors devId = %i \n", cache->bd->devNr);

    M_DEBUG("scache: flushSectors writeFlag=%d\n", cache->writeFlag);
    //no write operation occured since last flush
    if (cache->writeFlag == 0) {
        return 0;
    }

    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache->rec[i].writeDirty) {
            M_DEBUG("scache: flushSectors dirty index=%d sector=%u \n", i, cache->rec[i].sector);
            ret = WRITE_SECTOR(cache, cache->rec[i].sector, cache->sectorBuf + (i * BLOCK_SIZE), BLOCK_SIZE / cache->sectorSize);
            if (ret < 0) {
                M_PRINTF("scache: ERROR writing sector to disk! sector=%u\n", cache->rec[i].sector);
                return ret;
            }

            cache->rec[i].writeDirty = 0;
            counter++;
        }
    }
    cache->writeFlag = 0;
    return counter;
}

//---------------------------------------------------------------------------
int scache_readSector(cache_set* cache, unsigned int sector, void** buf)
{
    int index; //index is given in single sectors not octal sectors
    int ret;
    unsigned int alignedSector;

    M_DEBUG("scache: readSector devId = %i %p sector = %u \n", cache->bd->devNr, cache, sector);
    if (cache == NULL) {
        M_PRINTF("scache: devId cache not created = %i \n", cache->bd->devNr);
        return -1;
    }

#ifdef SCACHE_RECORD_STATS
    cache->cacheAccess++;
#endif
    index = getIndexRead(cache, sector);
    M_DEBUG("scache: indexRead=%i \n", index);
    if (index >= 0) { //sector found in cache
#ifdef SCACHE_RECORD_STATS
        cache->cacheHits++;
#endif
        *buf = cache->sectorBuf + (index * cache->sectorSize);
        M_DEBUG("scache: hit and done reading sector \n");

        return cache->sectorSize;
    }

    //compute alignedSector - to prevent storage of duplicit sectors in slots
    alignedSector = (sector / cache->indexLimit) * cache->indexLimit;
    index         = getIndexWrite(cache, alignedSector);
    M_DEBUG("scache: indexWrite=%i slot=%d  alignedSector=%u\n", index, index / cache->indexLimit, alignedSector);
    ret = READ_SECTOR(cache, alignedSector, cache->sectorBuf + (index * cache->sectorSize), BLOCK_SIZE / cache->sectorSize);

    if (ret < 0) {
        M_PRINTF("scache: ERROR reading sector from disk! sector=%u\n", alignedSector);
        return ret;
    }
    *buf = cache->sectorBuf + (index * cache->sectorSize) + ((sector % cache->indexLimit) * cache->sectorSize);
    M_DEBUG("scache: done reading physical sector \n");

    //write precaution
    //cache->flushCounter++;
    //if (cache->flushCounter == FLUSH_TRIGGER) {
    //scache_flushSectors(cache);
    //}

    return cache->sectorSize;
}

//---------------------------------------------------------------------------
int scache_allocSector(cache_set* cache, unsigned int sector, void** buf)
{
    int index; //index is given in single sectors not octal sectors
    //int ret;
    unsigned int alignedSector;

    M_DEBUG("scache: allocSector devId = %i sector = %u \n", cache->bd->devNr, sector);

    index = getIndexRead(cache, sector);
    M_DEBUG("scache: indexRead=%i \n", index);
    if (index >= 0) { //sector found in cache
        *buf = cache->sectorBuf + (index * cache->sectorSize);
        M_DEBUG("scache: hit and done allocating sector \n");
        return cache->sectorSize;
    }

    //compute alignedSector - to prevent storage of duplicit sectors in slots
    alignedSector = (sector / cache->indexLimit) * cache->indexLimit;
    index         = getIndexWrite(cache, alignedSector);
    M_DEBUG("scache: indexWrite=%i \n", index);
    *buf = cache->sectorBuf + (index * cache->sectorSize) + ((sector % cache->indexLimit) * cache->sectorSize);
    M_DEBUG("scache: done allocating sector\n");
    return cache->sectorSize;
}

//---------------------------------------------------------------------------
int scache_writeSector(cache_set* cache, unsigned int sector)
{
    int index; //index is given in single sectors not octal sectors
    //int ret;

    M_DEBUG("scache: writeSector devId = %i sector = %u \n", cache->bd->devNr, sector);

    index = getSlot(cache, sector);
    if (index < 0) { //sector not found in cache
        M_PRINTF("scache: writeSector: ERROR! the sector is not allocated! \n");
        return -1;
    }
    M_DEBUG("scache: slotFound=%i \n", index);

    //prefere written sectors to stay in cache longer than read sectors
    cache->rec[index].tax += 2;

    //set dirty status
    cache->rec[index].writeDirty = 1;
    cache->writeFlag++;

    M_DEBUG("scache: done soft writing sector \n");

    //write precaution
    //cache->flushCounter++;
    //if (cache->flushCounter == FLUSH_TRIGGER) {
    //scache_flushSectors(devId);
    //}

    return cache->sectorSize;
}

//---------------------------------------------------------------------------
cache_set* scache_init(struct block_device* bd)
{
    cache_set* cache;
    M_DEBUG("scache: init devId = %i sectorSize = %u \n", bd->devNr, bd->sectorSize);

    cache = malloc(sizeof(cache_set));
    if (cache == NULL) {
        M_PRINTF("scache init! Sector cache: can't alloate cache!\n");
        return NULL;
    }

    M_DEBUG("scache init!\n");
    cache->bd = bd;

    cache->sectorBuf = (unsigned char*)malloc(BLOCK_SIZE * CACHE_SIZE);
    if (cache->sectorBuf == NULL) {
        M_PRINTF("scache: can't alloate memory of size:%d \n", BLOCK_SIZE * CACHE_SIZE);
        free(cache);
        return NULL;
    }
    M_DEBUG("scache: allocated memory at:%p of size:%d \n", cache->sectorBuf, BLOCK_SIZE * CACHE_SIZE);

    //added by Hermes
    cache->sectorSize = bd->sectorSize;
    cache->indexLimit = BLOCK_SIZE / cache->sectorSize; //number of sectors per 1 cache slot
#ifdef SCACHE_RECORD_STATS
    cache->cacheAccess = 0;
    cache->cacheHits   = 0;
#endif
    initRecords(cache);
    return cache;
}

#ifdef SCACHE_RECORD_STATS
//---------------------------------------------------------------------------
void scache_getStat(cache_set* cache, unsigned int* access, unsigned int* hits)
{
    *access = cache->cacheAccess;
    *hits   = cache->cacheHits;
}
#endif

//---------------------------------------------------------------------------
void scache_kill(cache_set* cache) //dlanor: added for disconnection events (flush impossible)
{
    M_DEBUG("scache: kill devId = %i \n", cache->bd->devNr);
    if (cache->sectorBuf != NULL) {
        free(cache->sectorBuf);
        cache->sectorBuf = NULL;
    }
    free(cache);
}
//---------------------------------------------------------------------------
void scache_close(cache_set* cache)
{
    M_DEBUG("scache: close devId = %i \n", cache->bd->devNr);
    scache_flushSectors(cache);
    scache_kill(cache);
}
