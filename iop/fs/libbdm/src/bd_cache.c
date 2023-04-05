#include <bd_cache.h>
#include <string.h>
#include <sysmem.h>

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define SECTORS_PER_BLOCK     8 //  8 * 512b =   4KiB
#define BLOCK_COUNT          32 // 32 * 4KiB = 128KiB
#define BLOCK_WEIGHT_FACTOR 256 // Fixed point math (24.8)

struct bd_cache
{
    struct block_device *bd;
    int weight[BLOCK_COUNT];
    u64 sector[BLOCK_COUNT];
    u8 cache[BLOCK_COUNT][SECTORS_PER_BLOCK*512];
#ifdef DEBUG
    u32 sectors_read;
    u32 sectors_cache;
    u32 sectors_dev;
#endif
};

/* cache overlaps with requested area ? */
static int _overlaps(u64 csector, u64 sector, u16 count)
{
    if ((sector < (csector + SECTORS_PER_BLOCK)) && ((sector + count) > csector))
        return 1;
    else
        return 0;
}

/* cache contains requested area ? */
static int _contains(u64 csector, u64 sector, u16 count)
{
    if ((sector >= csector) && ((sector + count) <= (csector + SECTORS_PER_BLOCK)))
        return 1;
    else
        return 0;
}

static void _invalidate(struct bd_cache *c, u64 sector, u16 count)
{
    int blkidx;

    for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
        if (_overlaps(c->sector[blkidx], sector, count)) {
            // Invalidate cache entry
            c->sector[blkidx] = 0xffffffffffffffff;
        }
    }
}

static int _read(struct block_device *bd, u64 sector, void *buffer, u16 count)
{
    struct bd_cache *c = bd->priv;

    //M_DEBUG("%s(%d, %d)\n", __FUNCTION__, sector, count);

    if (count >= SECTORS_PER_BLOCK) {
        // Do a direct read
        return c->bd->read(c->bd, sector, buffer, count);
    }

#ifdef DEBUG
    c->sectors_read += count;
#endif

    // Do a cached read
    int blkidx;
    for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
        if (_contains(c->sector[blkidx], sector, count)) {
#ifdef DEBUG
            c->sectors_cache += count;
            //M_DEBUG("- CACHE HIT[%d] [block %d] [devread %ds, hit-ratio %d%%]\n", sector, blkidx, c->sectors_dev, (c->sectors_cache * 100) / c->sectors_read);
#endif
            // Minimum weight
            if (c->weight[blkidx] < 0)
                c->weight[blkidx] = 0;

            c->weight[blkidx] += count * BLOCK_WEIGHT_FACTOR;

            // Read from cache
            u64 offset = (sector - c->sector[blkidx]) * 512;
            memcpy(buffer, &c->cache[blkidx][offset], count * 512);
            return count;
        }
    }

    // Find block with the lowest weight
    int blkidx_best_weight = 0x7fffffff;
    int blkidx_best = 0;
    M_DEBUG("- list: ");
    for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
#ifdef DEBUG
        printf("%*d ", 3, c->weight[blkidx] / BLOCK_WEIGHT_FACTOR);
#endif

        // Dynamic aging
        c->weight[blkidx] -= (SECTORS_PER_BLOCK * BLOCK_WEIGHT_FACTOR / BLOCK_COUNT) + (c->weight[blkidx] / 32);

        if (c->weight[blkidx] < blkidx_best_weight) {
            // Better block found
            blkidx_best_weight = c->weight[blkidx];
            blkidx_best = blkidx;
        }
    }
#ifdef DEBUG
    printf(" devread: %*d, evict %*d [%*d], add [%*d]\n", 4, c->sectors_dev, 2, blkidx_best, 8, c->sector[blkidx_best], 8, sector);
    c->sectors_dev += SECTORS_PER_BLOCK;
    //M_DEBUG("- CACHE READ[%d] -> [block %d] [devread %ds, hit-ratio %d%%]\n", sector, blkidx_best, c->sectors_dev, (c->sectors_cache * 100) / c->sectors_read);
#endif

    // Fill the block
    c->bd->read(c->bd, sector, c->cache[blkidx_best], SECTORS_PER_BLOCK);
    c->sector[blkidx_best] = sector;

    // Read from cache
    u64 offset = (sector - c->sector[blkidx_best]) * 512;
    c->weight[blkidx_best] = count * BLOCK_WEIGHT_FACTOR;
    memcpy(buffer, &c->cache[blkidx_best][offset], count * 512);
    return count;
}

static int _write(struct block_device *bd, u64 sector, const void *buffer, u16 count)
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
    for (blkidx = 0; blkidx < BLOCK_COUNT; blkidx++) {
        c->weight[blkidx] = 0;
        c->sector[blkidx] = 0xffffffffffffffff;
    }
#ifdef DEBUG
    c->sectors_read = 0;
    c->sectors_cache = 0;
    c->sectors_dev = 0;
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
