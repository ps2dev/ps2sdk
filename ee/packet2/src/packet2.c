#include <malloc.h>
#include <kernel.h>
#include <assert.h>
#include <packet2.h>
#include <string.h>

#define PACKET2_ALIGNMENT 64
#define MAKE_PTR_NORMAL(PTR) ((u32)(PTR)&0x0FFFFFFF)

// ---
// Packet2 management
// ---

packet2_t *packet2_create_normal(u16 qwords, enum Packet2Type type)
{
    packet2_t *packet2 = (packet2_t *)calloc(1, sizeof(packet2_t));
    if (packet2 == NULL)
        return NULL;

    packet2->max_qwords_count = qwords;
    packet2->type = type;
    packet2->mode = P2_MODE_NORMAL;

    // Dma buffer size should be a whole number of cache lines (64 bytes = 4 quads)
    assert(!((packet2->type == P2_TYPE_UNCACHED || packet2->type == P2_TYPE_UNCACHED_ACCL) && packet2->max_qwords_count & (4 - 1)));

    u32 byte_size = packet2->max_qwords_count << 4;

    if ((packet2->base = memalign(PACKET2_ALIGNMENT, byte_size)) == NULL)
    {
        free(packet2);
        return NULL;
    }

    packet2->base = packet2->next = (qword_t *)((u32)packet2->base | packet2->type);

    memset(packet2->base, 0, byte_size);

    // "I hate to do this, but I've wasted FAR too much time hunting down cache incoherency"
    if (packet2->type == P2_TYPE_UNCACHED || packet2->type == P2_TYPE_UNCACHED_ACCL)
        FlushCache(0);

    return packet2;
}

packet2_t *packet2_create_chain(u16 qwords, enum Packet2Type type, u8 tte)
{
    packet2_t *packet2 = packet2_create_normal(qwords, type);
    packet2->mode = P2_MODE_CHAIN;
    packet2->tte = tte;
    return packet2;
}

packet2_t *packet2_create_from(qword_t *base, qword_t *next, u16 qwords, enum Packet2Type type, enum Packet2Mode mode)
{
    // Dma buffer size should be a whole number of cache lines (64 bytes = 4 quads)
    assert(!((type == P2_TYPE_UNCACHED || type == P2_TYPE_UNCACHED_ACCL) && qwords & (4 - 1)));

    // Dma buffer should be aligned on a cache line (64-byte boundary)
    assert(!((type == P2_TYPE_UNCACHED || type == P2_TYPE_UNCACHED_ACCL) && (u32)base & (64 - 1)));

    packet2_t *packet2 = (packet2_t *)calloc(1, sizeof(packet2_t));
    if (packet2 == NULL)
        return NULL;

    packet2->base = base;
    packet2->next = next;
    packet2->max_qwords_count = qwords;
    packet2->type = type;
    packet2->mode = mode;
    return packet2;
}

void packet2_free(packet2_t *packet2)
{
    if (packet2->base != NULL)
        free((qword_t *)MAKE_PTR_NORMAL(packet2->base));
    free(packet2);
}

void packet2_reset(packet2_t *packet2, u8 clear_mem)
{
    packet2->next = packet2->base;
    packet2->vif_code_opened_at = NULL;
    packet2->tag_opened_at = NULL;
    if (clear_mem)
        memset(packet2->base, 0, packet2->max_qwords_count << 4);
}

// ---
// Utils
// ---

void packet2_add(packet2_t *a, packet2_t *b)
{
    assert(packet2_get_qw_count(a) + packet2_get_qw_count(b) <= a->max_qwords_count);
    memcpy(a->next, b->base, (u32)b->next - (u32)b->base);
    a->next = a->base + packet2_get_qw_count(b);
}
