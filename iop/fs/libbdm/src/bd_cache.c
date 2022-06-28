#include <bd_cache.h>
#include <string.h>
#include <sysmem.h>

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"


#define SECTORS_PER_BLOCK 8 // 8 * 512b =  4KiB
#define BLOCK_COUNT       8 // 8 * 4KiB = 32KiB


struct bd_cache
{
    struct block_device *bd;
    u32 lru_current;
    u32 lru[BLOCK_COUNT];
    u32 sector[BLOCK_COUNT];
    u8 cache[BLOCK_COUNT][SECTORS_PER_BLOCK*512];
#ifdef DEBUG
    u32 sectors_read;
    u32 sectors_cache;
#endif
};

/* cache overlaps with requested area ? */
static int _overlaps(u32 csector, u32 sector, u16 count)
{
    if ((sector < (csector + SECTORS_PER_BLOCK)) && ((sector + count) > csector))
        return 1;
    else
        return 0;
}

/* cache contains requested area ? */
static int _contains(u32 csector, u32 sector, u16 count)
{
    if ((sector >= csector) && ((sector + count) <= (csector + SECTORS_PER_BLOCK)))
        return 1;
    else
        return 0;
}

static void _invalidate(struct bd_cache *c, u32 sector, u16 count)
{
    int blkidx;

    for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
        if (_overlaps(c->sector[blkidx], sector, count)) {
            // Invalidate cache entry
            c->sector[blkidx] = 0xffffffff;
        }
    }
}

static int _read(struct block_device *bd, u32 sector, void *buffer, u16 count)
{
    struct bd_cache *c = bd->priv;

    M_DEBUG("%s(%d, %d)\n", __FUNCTION__, sector, count);

    if (count >= SECTORS_PER_BLOCK) {
        // Do a direct read
        return c->bd->read(c->bd, sector, buffer, count);
    }
    else {
        // Do a cached read
        int blkidx;
        for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
            if (_contains(c->sector[blkidx], sector, count)) {
#ifdef DEBUG
                c->sectors_cache += 1;
                M_DEBUG("- CACHE HIT [block %d] [stats: read %ds, cache %ds]\n", blkidx, c->sectors_read, c->sectors_cache);
#endif
                // Read from cache
                u32 offset = (sector - c->sector[blkidx]) * 512;
                c->lru[blkidx] = c->lru_current++;
                memcpy(buffer, &c->cache[blkidx][offset], count * 512);
                return count;
            }
        }

        // Find the LRU block
        u32 blkidx_best_lru = 0xffffffff;
        int blkidx_best = 0;
        for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
            if (c->lru[blkidx] < blkidx_best_lru) {
                // Better block found
                blkidx_best_lru = c->lru[blkidx];
                blkidx_best = blkidx;
            }
        }

#ifdef DEBUG
        c->sectors_read  += 1; // number of reads from device
        c->sectors_cache += 1; // number of reads from cache
        M_DEBUG("- CACHE READ -> [block %d] [stats: read %d, cache %d]\n", blkidx_best, c->sectors_read, c->sectors_cache);
#endif

        // Fill the block
        c->bd->read(c->bd, sector, c->cache[blkidx_best], SECTORS_PER_BLOCK);
        c->sector[blkidx_best] = sector;

        // Read from cache
        u32 offset = (sector - c->sector[blkidx_best]) * 512;
        c->lru[blkidx_best] = c->lru_current++;
        memcpy(buffer, &c->cache[blkidx_best][offset], count * 512);
        return count;
    }
}

static int _write(struct block_device *bd, u32 sector, const void *buffer, u16 count)
{
    struct bd_cache *c = bd->priv;

    M_DEBUG("%s(%d, %d)\n", __FUNCTION__, sector, count);

    _invalidate(c, sector, count);

    return c->bd->write(c->bd, sector, buffer, count);
}

static void _flush(struct block_device *bd)
{
    struct bd_cache *c = bd->priv;

    M_DEBUG("%s\n", __FUNCTION__);

    return c->bd->flush(c->bd);
}

static int _stop(struct block_device *bd)
{
    struct bd_cache *c = bd->priv;

    M_DEBUG("%s\n", __FUNCTION__);

    return c->bd->stop(c->bd);
}

struct block_device *bd_cache_create(struct block_device *bd)
{
    int blkidx;

    // Create new block device
    struct block_device *cbd = AllocSysMemory(ALLOC_FIRST, sizeof(struct block_device), NULL);
    // Create new private data
    struct bd_cache *c = AllocSysMemory(ALLOC_FIRST, sizeof(struct bd_cache), NULL);

    M_DEBUG("%s\n", __FUNCTION__);

    c->bd = bd;
    c->lru_current = 1;
    for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
        c->lru[blkidx] = 0;
        c->sector[blkidx] = 0xffffffff;
    }
#ifdef DEBUG
    c->sectors_read = 0;
    c->sectors_cache = 0;
#endif

    // copy all parameters becouse we are the same blocks device
    // only difference is we are cached.
    cbd->priv         = c;
    cbd->name         = bd->name;
    cbd->devNr        = bd->devNr;
    cbd->parNr        = bd->parNr;
    cbd->parId        = bd->parId;
    cbd->sectorSize   = bd->sectorSize;
    cbd->sectorOffset = bd->sectorOffset;
    cbd->sectorCount  = bd->sectorCount;

    cbd->read = _read;
    cbd->write = _write;
    cbd->flush = _flush;
    cbd->stop = _stop;

    return cbd;
}

void bd_cache_destroy(struct block_device *cbd)
{
    M_DEBUG("%s\n", __FUNCTION__);

    FreeSysMemory(cbd->priv);
    FreeSysMemory(cbd);
}
