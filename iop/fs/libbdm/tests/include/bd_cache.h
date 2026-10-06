#ifndef __BDM_CACHE_H__
#define __BDM_CACHE_H__

#include <stdint.h>

typedef uint64_t u64;
typedef uint16_t u16;
typedef uint8_t u8;

struct block_device
{
    void *priv;
    char *name;
    char *path;
    unsigned int devNr;
    unsigned int parNr;
    unsigned int parId;
    unsigned int sectorSize;
    u64 sectorOffset;
    u64 sectorCount;
    int (*read)(struct block_device *, u64, void *, u16);
    int (*write)(struct block_device *, u64, const void *, u16);
    void (*flush)(struct block_device *);
    int (*stop)(struct block_device *);
};

struct block_device *bd_cache_create(struct block_device *bd);
void bd_cache_destroy(struct block_device *cbd);

#endif
