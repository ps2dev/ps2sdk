#ifndef _SCACHE_H
#define _SCACHE_H 1

#define BLOCK_SIZE 16384

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

struct _cache_set
{
    struct SBP2Device *dev;
    int sectorSize;
    int indexLimit;
    unsigned char *sectorBuf;     // = NULL; // sector content - the cache buffer
    cache_record rec[CACHE_SIZE]; // cache info record

    // statistical infos
    unsigned int cacheAccess;
    unsigned int cacheHits;
    unsigned int writeFlag;
    // unsigned int flushCounter;

    // unsigned int cacheDumpCounter = 0;
};

cache_set *scache_init(struct SBP2Device *dev, int sectorSize);
void scache_close(cache_set *cache);
void scache_kill(cache_set *cache); // dlanor: added for disconnection events (flush impossible)
int scache_allocSector(cache_set *cache, unsigned int sector, void **buf);
int scache_readSector(cache_set *cache, unsigned int sector, void **buf);
int scache_writeSector(cache_set *cache, unsigned int sector);
int scache_flushSectors(cache_set *cache);

void scache_getStat(cache_set *cache, unsigned int *access, unsigned int *hits);

#endif
