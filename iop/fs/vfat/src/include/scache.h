#ifndef _SCACHE_H
#define _SCACHE_H 1

#ifdef BUILDING_IEEE1394_DISK
#define BLOCK_SIZE 16384
#else
#define BLOCK_SIZE (4 * 1024)
#endif

#ifndef BUILDING_USBHDFSD
// number of cache slots (1 slot = block)
#define CACHE_SIZE 32

// when the flushCounter reaches FLUSH_TRIGGER then flushSectors is called
// #define FLUSH_TRIGGER 16

typedef struct _cache_record
{
    unsigned int sector;
    int tax;
    char writeDirty;
} cache_record;

typedef struct _cache_set
{
#if !defined(BUILDING_IEEE1394_DISK) && !defined(BUILDING_USBHDFSD)
    struct block_device *bd;
#else
    struct SBP2Device *dev;
#endif
    unsigned int sectorSize;
    unsigned int indexLimit;
    unsigned char *sectorBuf;     // = NULL; // sector content - the cache buffer
    cache_record rec[CACHE_SIZE]; // cache info record

    // statistical infos
    unsigned int cacheAccess;
    unsigned int cacheHits;
    unsigned int writeFlag;
    // unsigned int flushCounter;

    // unsigned int cacheDumpCounter = 0;
} cache_set;
#endif /* BUILDING_USBHDFSD */
#if defined(BUILDING_USBHDFSD)
cache_set *scache_init(mass_dev *dev, int sectorSize);
#elif defined(BUILDING_IEEE1394_DISK)
cache_set *scache_init(struct SBP2Device *dev, int sectorSize);
#else
cache_set *scache_init(struct block_device *bd);
#endif

void scache_close(cache_set *cache);
void scache_kill(cache_set *cache); // dlanor: added for disconnection events (flush impossible)
#if 0
int scache_allocSector(cache_set *cache, unsigned int sector, void **buf);
#endif
int scache_readSector(cache_set *cache, unsigned int sector, void **buf);
int scache_writeSector(cache_set *cache, unsigned int sector);
int scache_flushSectors(cache_set *cache);
#ifdef BUILDING_USBHDFSD
void scache_invalidate(cache_set *cache, unsigned int sector, int count);
#endif /* BUILDING_USBHDFSD */

void scache_getStat(cache_set *cache, unsigned int *access, unsigned int *hits);

#endif
