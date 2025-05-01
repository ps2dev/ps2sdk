#include "kernel.h"

#define LINE_SIZE 0x40
#define LINE_MASK (~(LINE_SIZE - 1))

#define DXWBIN 0x14 /* Data cache: indeX WriteBack INvalidate.  */
#define DXIN   0x16 /* Data cache: indeX INvalidate.  */
#define DHWBIN 0x18 /* Data cache: Hit WriteBack INvalidate.  */
#define DHIN   0x1a /* Data cache: Hit INvalidate.  */

#define DCACHE_OP_LINE(op, line) \
    __asm__ volatile(            \
        ".set push          \n"  \
        ".set noreorder     \n"  \
        "sync.l             \n"  \
        "cache %0, 0(%1)    \n"  \
        "sync.l             \n"  \
        ".set pop           \n"  \
        :                        \
        : "i"(op), "r"(start));


static inline void _SyncDCache(u32 start, u32 end)
{
    while (1) {
        DCACHE_OP_LINE(DHWBIN, start);
        if (start == end) {
            break;
        }
        start += LINE_SIZE;
    }
}

static inline void _InvalidDCache(u32 start, u32 end)
{
    while (1) {
        DCACHE_OP_LINE(DHIN, start);
        if (start == end) {
            break;
        }
        start += LINE_SIZE;
    }
}

#ifdef F_SyncDCache
void SyncDCache(void *start, void *end)
{
    int oldintr;

    oldintr = DIntr();

    _SyncDCache((u32)start & LINE_MASK, (u32)end & LINE_MASK);

    if (oldintr) {
        EIntr();
    }
}
#endif

#ifdef F_iSyncDCache
void iSyncDCache(void *start, void *end)
{
    _SyncDCache((u32)start & LINE_MASK, (u32)end & LINE_MASK);
}
#endif

#ifdef F_InvalidDCache
void InvalidDCache(void *start, void *end)
{
    int oldintr;

    oldintr = DIntr();

    _InvalidDCache((u32)start & LINE_MASK, (u32)end & LINE_MASK);

    if (oldintr) {
        EIntr();
    }
}
#endif

#ifdef F_iInvalidDCache
void iInvalidDCache(void *start, void *end)
{
    _InvalidDCache((u32)start & LINE_MASK, (u32)end & LINE_MASK);
}
#endif
