//---------------------------------------------------------------------------
// File name:    scache.c
//---------------------------------------------------------------------------
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
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#else
#include <tamtypes.h>
#include <sysmem.h>
#endif

// #define SCACHE_RECORD_STATS 1

#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
#include <bdm.h>
#endif
#ifdef BUILDING_USBHDFSD
#include <usbhdfsd.h>
#endif /* BUILDING_USBHDFSD */
#include "usbhd_common.h"
#ifdef BUILDING_USBHDFSD
#include "mass_stor.h"
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
#include "sbp2_disk.h"
#include "scsi.h"
#endif /* BUILDING_IEEE1394_DISK */
#include "scache.h"

//---------------------------------------------------------------------------
#ifdef BUILDING_USBHDFSD
#define READ_SECTOR  mass_stor_readSector
#define WRITE_SECTOR mass_stor_writeSector
#define DEV_ACCESSOR(d) ((d)->dev)
#define DEVID_ACCESSOR(d) ((d)->dev->devId)
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
#define READ_SECTOR  scsiReadSector
#define WRITE_SECTOR scsiWriteSector
#define DEV_ACCESSOR(d) ((d)->dev)
#define DEVID_ACCESSOR(d) ((d)->dev->nodeID)
#endif /* BUILDING_IEEE1394_DISK */
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
#define READ_SECTOR(d, s, a, c) (d)->bd->read((d)->bd, s, a, c)
#define WRITE_SECTOR(d, s, a, c) (d)->bd->write((d)->bd, s, a, c)
#define DEV_ACCESSOR(d) (d)
#define DEVID_ACCESSOR(d) ((d)->bd->devNr)
#endif

// #define DEBUG  //comment out this line when not debugging

#include "mass_debug.h"

#ifdef BUILDING_USBHDFSD
// when the flushCounter reaches FLUSH_TRIGGER then flushSectors is called
// #define FLUSH_TRIGGER 16

static int scache_flushSector(cache_set *cache, int index);
#endif /* BUILDING_USBHDFSD */

//---------------------------------------------------------------------------
static void initRecords(cache_set *cache)
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
static int getSlot(cache_set *cache, unsigned int sector)
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
static int getIndexRead(cache_set *cache, unsigned int sector)
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
        {
            cache->rec[i].tax--; // apply tax penalty
        }
    }
    if (index < 0)
        return index;
    else
        return ((index * cache->indexLimit) + (sector - cache->rec[index].sector));
}

//---------------------------------------------------------------------------
/* select the best record where to store new sector */
static int getIndexWrite(cache_set *cache, unsigned int sector)
{
    int minTax = 0x0FFFFFFF;
    unsigned int i, index = 0;

    for (i = 0; i < CACHE_SIZE; i++) {
        if (cache->rec[i].tax < minTax) {
            index  = i;
            minTax = cache->rec[i].tax;
        }
    }

    // this sector is dirty - we need to flush it first
#ifdef BUILDING_USBHDFSD
    {
        int ret;
        ret = scache_flushSector(cache, index);
        if (ret != 1)
            return ret;
    }
#else
    if (cache->rec[index].writeDirty) {
        int ret;

        XPRINTF("scache: getIndexWrite: sector is dirty : %d   index=%d \n", cache->rec[index].sector, index);
        ret                          = WRITE_SECTOR(DEV_ACCESSOR(cache), cache->rec[index].sector, cache->sectorBuf + (index * BLOCK_SIZE), BLOCK_SIZE / cache->sectorSize);
        cache->rec[index].writeDirty = 0;
        // TODO - error handling
        if (ret < 0) {
            M_PRINTF("scache: ERROR writing sector to disk! sector=%u\n", sector);
        }
    }
#endif
    cache->rec[index].tax += 2;
    cache->rec[index].sector = sector;

    return index * cache->indexLimit;
}

//---------------------------------------------------------------------------
/*
    flush dirty sectors
 */
#ifdef BUILDING_USBHDFSD
static int scache_flushSector(cache_set *cache, int index)
{
    if (cache->rec[index].writeDirty) {
        int ret;

        XPRINTF("scache: flushSector dirty index=%d sector=%u \n", index, cache->rec[index].sector);
        ret = WRITE_SECTOR(DEV_ACCESSOR(cache), cache->rec[index].sector, cache->sectorBuf + (index * BLOCK_SIZE), BLOCK_SIZE / cache->sectorSize);
        if (ret < 0) {
            M_PRINTF("scache: ERROR writing sector to disk! sector=%u\n", cache->rec[index].sector);
            return ret;
        }

        cache->rec[index].writeDirty = 0;
    }
    return 1;
}
#endif /* BUILDING_USBHDFSD */

int scache_flushSectors(cache_set *cache)
{
    unsigned int i;
    int counter = 0;

    XPRINTF("cache: flushSectors devId = %i \n", DEVID_ACCESSOR(cache));

    XPRINTF("scache: flushSectors writeFlag=%d\n", cache->writeFlag);
    // no write operation occured since last flush
    if (cache->writeFlag == 0) {
        return 0;
    }

    for (i = 0; i < CACHE_SIZE; i++) {
        int ret;
#ifdef BUILDING_USBHDFSD
        if ((ret = scache_flushSector(cache, i)) >= 0)
#else
        if (cache->rec[i].writeDirty)
#endif
        {
#ifndef BUILDING_USBHDFSD
            XPRINTF("scache: flushSectors dirty index=%d sector=%u \n", i, cache->rec[i].sector);
            ret                      = WRITE_SECTOR(DEV_ACCESSOR(cache), cache->rec[i].sector, cache->sectorBuf + (i * BLOCK_SIZE), BLOCK_SIZE);
#if defined(BUILDING_IEEE1394_DISK) || defined(BUILDING_USBHDFSD)
            cache->rec[i].writeDirty = 0;
#endif
            // TODO - error handling
            if (ret < 0) {
                M_PRINTF("scache: ERROR writing sector to disk! sector=%d\n", cache->rec[i].sector);
                return ret;
            }
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
            cache->rec[i].writeDirty = 0;
#endif
#endif /* BUILDING_USBHDFSD */
            counter++;
        }
#ifdef BUILDING_USBHDFSD
        else
            return ret;
#endif /* BUILDING_USBHDFSD */
    }
    cache->writeFlag = 0;
    return counter;
}

//---------------------------------------------------------------------------
int scache_readSector(cache_set *cache, unsigned int sector, void **buf)
{
    int index; // index is given in single sectors not octal sectors
    int ret;
    unsigned int alignedSector;

    if (cache != NULL) {
        XPRINTF("cache: readSector devId = %i %p sector = %u \n", DEVID_ACCESSOR(cache), cache, sector);
    }
    if (cache == NULL) {
        M_PRINTF("cache: devId cache not created \n");
        return -1;
    }

#ifdef SCACHE_RECORD_STATS
    cache->cacheAccess++;
#endif
    index = getIndexRead(cache, sector);
    XPRINTF("cache: indexRead=%i \n", index);
    if (index >= 0) { // sector found in cache
#ifdef SCACHE_RECORD_STATS
        cache->cacheHits++;
#endif
        *buf = cache->sectorBuf + (index * cache->sectorSize);
        XPRINTF("cache: hit and done reading sector \n");

        return cache->sectorSize;
    }

    // compute alignedSector - to prevent storage of duplicit sectors in slots
    alignedSector = (sector / cache->indexLimit) * cache->indexLimit;
    index         = getIndexWrite(cache, alignedSector);
    XPRINTF("cache: indexWrite=%i slot=%d  alignedSector=%u\n", index, index / cache->indexLimit, alignedSector);
    ret = READ_SECTOR(DEV_ACCESSOR(cache), alignedSector, cache->sectorBuf + (index * cache->sectorSize), BLOCK_SIZE / cache->sectorSize);

    if (ret < 0) {
        M_PRINTF("scache: ERROR reading sector from disk! sector=%u\n", alignedSector);
        return ret;
    }
    *buf = cache->sectorBuf + (index * cache->sectorSize) + ((sector % cache->indexLimit) * cache->sectorSize);
    XPRINTF("cache: done reading physical sector \n");

    // write precaution
    /* cache->flushCounter++;
    if (cache->flushCounter == FLUSH_TRIGGER) {
        scache_flushSectors(cache);
    } */

    return cache->sectorSize;
}


//---------------------------------------------------------------------------
/* SP193: this function is dangerous if not used correctly.
   As scache's blocks are aligned to the start of the disk, the clusters of the partition
   must also be aligned to a multiple of the scache block size.
   Otherwise, it is possible to cause the adjacent cluster to lose data, if the block spans across more than one cluster.
*/
#if 0
int scache_allocSector(cache_set *cache, unsigned int sector, void **buf)
{
    int index; // index is given in single sectors not octal sectors
    // int ret;
    unsigned int alignedSector;

    XPRINTF("cache: allocSector devId = %i sector = %u \n", DEVID_ACCESSOR(cache), sector);

    index = getIndexRead(cache, sector);
    XPRINTF("cache: indexRead=%i \n", index);
    if (index >= 0) { // sector found in cache
        *buf = cache->sectorBuf + (index * cache->sectorSize);
        XPRINTF("cache: hit and done allocating sector \n");
        return cache->sectorSize;
    }

    // compute alignedSector - to prevent storage of duplicit sectors in slots
    alignedSector = (sector / cache->indexLimit) * cache->indexLimit;
    index         = getIndexWrite(cache, alignedSector);
    XPRINTF("cache: indexWrite=%i \n", index);
    *buf = cache->sectorBuf + (index * cache->sectorSize) + ((sector % cache->indexLimit) * cache->sectorSize);
    XPRINTF("cache: done allocating sector\n");
    return cache->sectorSize;
}
#endif

//---------------------------------------------------------------------------
int scache_writeSector(cache_set *cache, unsigned int sector)
{
    int index; // index is given in single sectors not octal sectors
    // int ret;

    XPRINTF("cache: writeSector devId = %i sector = %u \n", DEVID_ACCESSOR(cache), sector);

    index = getSlot(cache, sector);
    if (index < 0) { // sector not found in cache
        M_PRINTF("cache: writeSector: ERROR! the sector is not allocated! \n");
        return -1;
    }
    XPRINTF("cache: slotFound=%i \n", index);

    // prefere written sectors to stay in cache longer than read sectors
    cache->rec[index].tax += 2;

    // set dirty status
    cache->rec[index].writeDirty = 1;
    cache->writeFlag++;

    XPRINTF("cache: done soft writing sector \n");

    // write precaution
    /* cache->flushCounter++;
    if (cache->flushCounter == FLUSH_TRIGGER) {
        scache_flushSectors(devId);
    } */

    return cache->sectorSize;
}

#ifdef BUILDING_USBHDFSD
void scache_invalidate(cache_set *cache, unsigned int sector, int count)
{
    int i;

    XPRINTF("cache: invalidate devId = %i sector = %u count = %d \n", DEVID_ACCESSOR(cache), sector, count);

    for (i = 0; i < count; i++, sector++) {
        int index; // index is given in single sectors not octal sectors

        index = getSlot(cache, sector);
        if (index >= 0) { // sector found in cache. Write back and invalidate the block it belongs to.
            scache_flushSector(cache, index);

            cache->rec[index].sector     = 0xFFFFFFF0;
            cache->rec[index].tax        = 0;
            cache->rec[index].writeDirty = 0;
        }
    }
}
#endif /* BUILDING_USBHDFSD */

//---------------------------------------------------------------------------
#if defined(BUILDING_USBHDFSD)
cache_set *scache_init(mass_dev *dev, int sectSize)
#elif defined(BUILDING_IEEE1394_DISK)
cache_set *scache_init(struct SBP2Device *dev, int sectSize)
#else
cache_set *scache_init(struct block_device *bd)
#endif
{
    cache_set *cache;
#ifdef BUILDING_USBHDFSD
    XPRINTF("cache: init devId = %i sectSize = %u \n", dev->devId, sectSize);
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
    XPRINTF("cache: init devId = %i sectSize = %i \n", dev->nodeID, sectSize);
#endif /* BUILDING_IEEE1394_DISK */

    cache = malloc(sizeof(cache_set));
    if (cache == NULL) {
        M_PRINTF("scache init! Sector cache: can't alloate cache!\n");
        return NULL;
    }

    XPRINTF("scache init! \n");
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    cache->bd = bd;
#else
    cache->dev = dev;
#endif

    cache->sectorBuf = (unsigned char *)malloc(BLOCK_SIZE * CACHE_SIZE);
    if (cache->sectorBuf == NULL) {
        M_PRINTF("Sector cache: can't alloate memory of size:%d \n", BLOCK_SIZE * CACHE_SIZE);
        free(cache);
        return NULL;
    }
    XPRINTF("Sector cache: allocated memory at:%p of size:%d \n", cache->sectorBuf, BLOCK_SIZE * CACHE_SIZE);

    // added by Hermes
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    cache->sectorSize = bd->sectorSize;
#else
    cache->sectorSize = sectSize;
#endif
    cache->indexLimit = BLOCK_SIZE / cache->sectorSize; // number of sectors per 1 cache slot
#ifdef SCACHE_RECORD_STATS
    cache->cacheAccess = 0;
    cache->cacheHits   = 0;
#endif
    XPRINTF("sectorSize: 0x%x\n", cache->sectorSize);
    initRecords(cache);
    return cache;
}

#ifdef SCACHE_RECORD_STATS
//---------------------------------------------------------------------------
void scache_getStat(cache_set *cache, unsigned int *access, unsigned int *hits)
{
    *access = cache->cacheAccess;
    *hits   = cache->cacheHits;
}
#endif

//---------------------------------------------------------------------------
void scache_kill(cache_set *cache) // dlanor: added for disconnection events (flush impossible)
{
    XPRINTF("cache: kill devId = %i \n", DEVID_ACCESSOR(cache));
    if (cache->sectorBuf != NULL) {
        free(cache->sectorBuf);
        cache->sectorBuf = NULL;
    }
    free(cache);
}
//---------------------------------------------------------------------------
void scache_close(cache_set *cache)
{
    XPRINTF("cache: close devId = %i \n", DEVID_ACCESSOR(cache));
    scache_flushSectors(cache);
    scache_kill(cache);
}
//---------------------------------------------------------------------------
// End of file:  scache.c
//---------------------------------------------------------------------------
