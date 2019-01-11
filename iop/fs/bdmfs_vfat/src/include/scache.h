#ifndef _SCACHE_H
#define _SCACHE_H

#include <bdm.h>

//number of cache slots (1 slot = block)
#define CACHE_SIZE 32

typedef struct _cache_record {
    unsigned int sector;
    int tax;
    char writeDirty;
} cache_record;

typedef struct _cache_set {
    struct block_device* bd;
    unsigned int sectorSize;
    unsigned int indexLimit;
    unsigned char* sectorBuf;     // = NULL;		//sector content - the cache buffer
    cache_record rec[CACHE_SIZE]; //cache info record

#ifdef SCACHE_RECORD_STATS
    //statistical information
    unsigned int cacheAccess;
    unsigned int cacheHits;
#endif
    unsigned int writeFlag;
} cache_set;

cache_set* scache_init(struct block_device* bd);
void scache_close(cache_set* cache);
void scache_kill(cache_set* cache); //dlanor: added for disconnection events (flush impossible)
//int scache_allocSector(cache_set* cache, unsigned int sector, void** buf);
int scache_readSector(cache_set* cache, unsigned int sector, void** buf);
int scache_writeSector(cache_set* cache, unsigned int sector);
int scache_flushSectors(cache_set* cache);

void scache_getStat(cache_set* cache, unsigned int* access, unsigned int* hits);

#endif
