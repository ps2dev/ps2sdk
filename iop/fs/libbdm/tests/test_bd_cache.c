#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <bd_cache.h>

static int calls;
static int mode;

static int read_device(struct block_device *bd, u64 sector, void *buffer, u16 count)
{
    unsigned int i;

    (void)bd;
    ++calls;

    /* Poison failed transfers to catch accidentally caching incomplete data. */
    memset(buffer, 0xee, count * 512);

    if (mode == 1)
        return -7;
    if (mode == 2)
        return count - 1;
    if (sector + count > 100)
        return -7;

    for (i = 0; i < count; ++i)
        memset((u8 *)buffer + i * 512, (u8)(sector + i), 512);

    return count;
}

static int write_device(struct block_device *bd, u64 sector, const void *buffer, u16 count)
{
    (void)bd;
    (void)sector;
    (void)buffer;
    return count;
}

int main(void)
{
    struct block_device raw = {
        .sectorSize = 512,
        .sectorCount = 100,
        .read = read_device,
        .write = write_device,
    };
    struct block_device *cache = bd_cache_create(&raw);
    u8 data[4096];

    assert(cache != NULL);

    assert(cache->read(cache, 10, data, 1) == 1 && calls == 1);
    assert(cache->read(cache, 11, data, 1) == 1 && calls == 1 && data[0] == 11);

    /* Read-ahead past the end of media must fall back to the requested range. */
    assert(cache->read(cache, 99, data, 1) == 1 && calls == 3 && data[0] == 99);
    assert(cache->read(cache, 99, data, 1) == 1 && calls == 3);
    assert(cache->read(cache, 100, data, 1) < 0 && calls == 5);

    /* Failed and short fills must neither reach the caller nor become hits. */
    mode = 1;
    assert(cache->read(cache, 30, data, 1) == -7 && calls == 7);
    mode = 2;
    assert(cache->read(cache, 30, data, 1) < 0 && calls == 9);
    mode = 0;
    assert(cache->read(cache, 30, data, 1) == 1 && calls == 10 && data[0] == 30);

    /* Writes invalidate overlapping cached data. */
    assert(cache->write(cache, 30, data, 1) == 1);
    assert(cache->read(cache, 30, data, 1) == 1 && calls == 11);

    /* Full cache-block requests bypass the cache. */
    assert(cache->read(cache, 40, data, 8) == 8 && calls == 12 && data[4095] == 47);

    bd_cache_destroy(cache);

    puts("PASS: hits, end-of-media fallback, errors, short fills, invalidation, direct reads");
    return 0;
}
