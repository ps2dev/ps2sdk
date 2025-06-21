#include "cache.h"

#include "kernel.h"

/*
 * Write back data cache lines corresponding to range [start, end)
 */
#ifdef F_sceSifWriteBackDCache
void sceSifWriteBackDCache(void *ptr, int size)
{
    dcache_writeback_range((u32)ptr, (u32)ptr + size);
}
#endif

/*
 * These functions affect range [start, end]
 * (Inclusive on both sides)
 *
 * e.g. SyncDCache(0x0, 0x40) would affect two cache lines.
 */

#ifdef F_SyncDCache
void SyncDCache(void *start, void *end)
{
    dcache_writeback_range((u32)start, (u32)end + 1);
}
#endif

#ifdef F_iSyncDCache
void iSyncDCache(void *start, void *end)
{
    dcache_writeback_range((u32)start, (u32)end + 1);
}
#endif

#ifdef F_InvalidDCache
void InvalidDCache(void *start, void *end)
{
    dcache_invalid_range((u32)start, (u32)end + 1);
}
#endif

#ifdef F_iInvalidDCache
void iInvalidDCache(void *start, void *end)
{
    dcache_invalid_range((u32)start, (u32)end + 1);
}
#endif
