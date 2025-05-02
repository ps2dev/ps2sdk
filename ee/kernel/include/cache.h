#ifndef CACHE_H_
#define CACHE_H_

#define CACHE_LINE_SIZE 0x40
#define CACHE_LINE_MASK (~(CACHE_LINE_SIZE - 1))

#define OP_IHIN   0x0b /* Instruction cache: Hit INvalidate.  */
#define OP_DXWBIN 0x14 /* Data cache: indeX WriteBack INvalidate.  */
#define OP_DXIN   0x16 /* Data cache: indeX INvalidate.  */
#define OP_DHWBIN 0x18 /* Data cache: Hit WriteBack INvalidate.  */
#define OP_DHIN   0x1a /* Data cache: Hit INvalidate.  */

#define DCACHE_OP_LINE(op, line) \
    __asm__ volatile(            \
        ".set push          \n"  \
        ".set noreorder     \n"  \
        "sync.l             \n"  \
        "cache %0, 0(%1)    \n"  \
        "sync.l             \n"  \
        ".set pop           \n"  \
        :                        \
        : "i"(op), "r"(line));

#define ICACHE_OP_LINE(op, line) \
    __asm__ volatile(            \
        ".set push          \n"  \
        ".set noreorder     \n"  \
        "sync.p             \n"  \
        "cache %0, 0(%1)    \n"  \
        "sync.p             \n"  \
        ".set pop           \n"  \
        :                        \
        : "i"(op), "r"(line));

/* Single line operations */
static inline void dcache_writeback_line(unsigned addr)
{
    DCACHE_OP_LINE(OP_DHWBIN, addr);
}

static inline void dcache_invalid_line(unsigned addr)
{
    DCACHE_OP_LINE(OP_DHIN, addr);
}

static inline void icache_invalid_line(unsigned addr)
{
    ICACHE_OP_LINE(OP_IHIN, addr);
}

/*
 * The standard SyncDCache/InvalidDCache functions have a slightly awkward
 * API in that the range is inclusive of both the start and end, requiring
 * you to remember to substract from the end to avoid affecting unrelated
 * cache lines.
 *
 *
 * These functions are exclusive of the end and can therefore be used
 * with sizeof in less error-prone way.
 * e.g. dcache_writeback_range(&mystruct, &mystruct + sizeof(mystruct));
 */

/*
 * Write back data cache lines corresponding to range [start, end)
 */
static inline void dcache_writeback_range(unsigned start, unsigned end)
{
    start = start & CACHE_LINE_MASK;
    end   = (end - 1) & CACHE_LINE_MASK;

    while (1) {
        dcache_writeback_line(start);
        if (start == end) {
            break;
        }
        start += CACHE_LINE_SIZE;
    }
}

/*
 * Invalidate data cache lines corresponding to range [start, end)
 */
static inline void dcache_invalid_range(unsigned start, unsigned end)
{
    start = start & CACHE_LINE_MASK;
    end   = (end - 1) & CACHE_LINE_MASK;

    while (1) {
        dcache_invalid_line(start);
        if (start == end) {
            break;
        }
        start += CACHE_LINE_SIZE;
    }
}

/*
 * Invalidate instruction cache lines corresponding to range [start, end)
 * This also automatically invalidates the relevant BTAC entries.
 */
static inline void icache_invalid_range(unsigned start, unsigned end)
{
    start = start & CACHE_LINE_MASK;
    end   = (end - 1) & CACHE_LINE_MASK;

    while (1) {
        icache_invalid_line(start);
        if (start == end) {
            break;
        }
        start += CACHE_LINE_SIZE;
    }
}


#endif // CACHE_H_
