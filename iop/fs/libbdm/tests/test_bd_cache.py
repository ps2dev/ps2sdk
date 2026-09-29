#!/usr/bin/env python3
"""Compile the actual BDM cache with a fault-injecting host block device."""
from pathlib import Path
import subprocess
import tempfile

source = Path(__file__).resolve().parents[1] / 'src/bd_cache.c'
with tempfile.TemporaryDirectory(prefix='bd-cache-test-') as directory:
    root = Path(directory)
    (root / 'bd_cache.h').write_text('''
#include <stdint.h>
typedef uint64_t u64;
typedef uint16_t u16;
typedef uint8_t u8;
struct block_device {
 void *priv; char *name; char *path;
 unsigned devNr, parNr, parId, sectorSize;
 u64 sectorOffset, sectorCount;
 int (*read)(struct block_device *, u64, void *, u16);
 int (*write)(struct block_device *, u64, const void *, u16);
 void (*flush)(struct block_device *);
 int (*stop)(struct block_device *);
};
''')
    (root / 'sysmem.h').write_text('''
#include <stdlib.h>
#define ALLOC_FIRST 0
#define AllocSysMemory(mode, size, address) malloc(size)
#define FreeSysMemory(ptr) free(ptr)
''')
    (root / 'module_debug.h').write_text('''
#define DEBUG_U64_2XU32(x)
#define M_DEBUG(...)
''')
    (root / 'test.c').write_text('#include "' + str(source) + '"\n' + r'''
#include <assert.h>
#include <stdio.h>
static int calls, mode;
static int read_device(struct block_device *bd, u64 sector, void *buffer, u16 count)
{
    (void)bd;
    ++calls;
    /* Poison failed transfers to catch accidentally caching incomplete data. */
    memset(buffer, 0xee, count * 512);
    if (mode == 1) return -7;
    if (mode == 2) return count - 1;
    if (sector + count > 100) return -7;
    for (unsigned i = 0; i < count; ++i)
        memset((u8 *)buffer + i * 512, (u8)(sector + i), 512);
    return count;
}
static int write_device(struct block_device *bd, u64 sector, const void *buf, u16 count)
{ (void)bd; (void)sector; (void)buf; return count; }
int main(void)
{
    struct block_device raw = { .sectorSize = 512, .sectorCount = 100,
        .read = read_device, .write = write_device };
    struct block_device *cache = bd_cache_create(&raw);
    u8 data[4096];
    assert(cache->read(cache, 10, data, 1) == 1 && calls == 1);
    assert(cache->read(cache, 11, data, 1) == 1 && calls == 1 && data[0] == 11);
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
    assert(cache->write(cache, 30, data, 1) == 1);
    assert(cache->read(cache, 30, data, 1) == 1 && calls == 11);
    assert(cache->read(cache, 40, data, 8) == 8 && calls == 12 && data[4095] == 47);
    bd_cache_destroy(cache);
    puts("PASS: hits, end-of-media fallback, errors, short fills, invalidation, direct reads");
}
''')
    subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-Werror',
                    '-I', str(root),
                    str(root / 'test.c'), '-o', str(root / 'test')], check=True)
    subprocess.run([str(root / 'test')], check=True, timeout=30)
