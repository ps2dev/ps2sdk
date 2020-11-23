/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <malloc.h>
#include <kernel.h>
#include <assert.h>
#include <packet2.h>
#include <string.h>
#include <stdio.h>

#define P2_ALIGNMENT 64
#define P2_MAKE_PTR_NORMAL(PTR) ((u32)(PTR)&0x0FFFFFFF)

// ---
// Packet2 management
// ---

packet2_t *packet2_create(u16 qwords, enum Packet2Type type, enum Packet2Mode mode, u8 tte)
{
    packet2_t *packet2 = (packet2_t *)calloc(1, sizeof(packet2_t));
    if (packet2 == NULL)
        return NULL;

    packet2->max_qwords_count = qwords;
    packet2->type = type;
    packet2->mode = mode;
    packet2->tte = tte;
    packet2->vif_added_bytes = 0;
    packet2->tag_opened_at = NULL;
    packet2->vif_code_opened_at = NULL;

    // Dma buffer size should be a whole number of cache lines (64 bytes = 4 quads)
    assert(!((packet2->type == P2_TYPE_UNCACHED || packet2->type == P2_TYPE_UNCACHED_ACCL) && packet2->max_qwords_count & (4 - 1)));

    u32 byte_size = packet2->max_qwords_count << 4;

    if ((packet2->base = memalign(P2_ALIGNMENT, byte_size)) == NULL)
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

packet2_t *packet2_create_from(qword_t *base, qword_t *next, u16 qwords, enum Packet2Type type, enum Packet2Mode mode, u8 tte)
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
    packet2->tte = tte;
    packet2->mode = mode;
    return packet2;
}

void packet2_free(packet2_t *packet2)
{
    if (packet2->base != NULL)
        free((qword_t *)P2_MAKE_PTR_NORMAL(packet2->base));
    free(packet2);
}

void packet2_reset(packet2_t *packet2, u8 clear_mem)
{
    packet2->next = packet2->base;
    packet2->vif_code_opened_at = NULL;
    packet2->tag_opened_at = NULL;
    packet2->vif_added_bytes = 0;
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
    a->next = a->base + packet2_get_qw_count(b) + 1;
}

void packet2_print(packet2_t *packet2, u32 qw_count)
{
    if (qw_count == 0)
        qw_count = ((u32)packet2->next - (u32)packet2->base) >> 4;
    printf("\n============================\n");
    printf("Packet2: Dumping %d words...\n", ((u32)packet2->next - (u32)packet2->base) >> 2);
    u32 i = 0;
    u32 *nextWord;
    for (nextWord = (u32 *)packet2->base; nextWord != (u32 *)packet2->next; nextWord++, i++)
    {
        if ((i % 4) == 0)
            printf("\n0x%08x:  ", (u32)nextWord);
        printf("0x%08x ", *nextWord);
        if (i / 4 == qw_count)
            break;
    }
    printf("\n============================\n");
}
