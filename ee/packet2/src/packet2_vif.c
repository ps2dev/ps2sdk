/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "packet2_vif.h"
#include "packet2_chain.h"

void packet2_vif_add_micro_program(packet2_t *packet2, u32 dest, u32 *start, u32 *end)
{
    // get the size of the code as we can only send 256 instructions in each MPGtag
    u32 count = (end - start) / 2;
    if (count & 1)
        count++;

    u32 *l_start = start;
    while (count > 0)
    {
        u16 curr_count = count > 256 ? 256 : count;
        packet2_chain_ref(packet2, l_start, curr_count / 2, 0, 0, 0);
        packet2_vif_nop(packet2, 0);
        packet2_vif_mpg(packet2, curr_count & 0xFF, dest, 0);
        l_start += curr_count * 2;
        count -= curr_count;
        dest += curr_count;
    }
}

u32 packet2_vif_close_unpack_auto(packet2_t *packet2, u32 wl, u32 cl)
{
    assert(packet2->vif_code_opened_at != NULL);               // There is open UNPACK/DIRECT.
    assert(((u32)packet2->next & 0x3) == 0);                   // Make sure we're u32 aligned
    assert((packet2->vif_code_opened_at->cmd & 0x60) == 0x60); // It was UNPACK

    u32 vn = (packet2->vif_code_opened_at->cmd & 0xC) >> 2;
    u32 vl = packet2->vif_code_opened_at->cmd & 0x3;

    // "the goal here is to find the num field of the open unpack, which is the number of
    // qwords ACTUALLY WRITTEN to vu memory (it does not count quads that are skipped in
    // "skipping write" mode.)  But first forget about skipping/filling writes and compute the number
    // of qwords that the data in this packet will expand to."
    u32 bytes_count = (u32)packet2->next - (u32)packet2->vif_code_opened_at - 4;
    u32 block_bytes_count = 4 >> vl;
    u32 quad_blocks_count = vn + 1;

    // "make sure that the data length is a multiple of 8, 16, or 32 bits, whichever is appropriate"
    assert((bytes_count & (block_bytes_count - 1)) == 0);
    u32 quads_count = (bytes_count / block_bytes_count) / quad_blocks_count;

    // "We have the number of quads our data will directly expand to, so now we need to account for
    // skipping/filling write modes.""

    // "skipping write is easy -- we are already done"

    // "filling write: now we get ambiguous -- what to do when quads_count == CL in the last
    // block? I will say that in this case the vif should still do the full wl length block, filling
    // with internal registers. If you want different behavior call packet2_vif_close_unpack_manual
    // with a num field you've computed yourself"
    if (cl < wl)
    {
        u32 wl_blocks_count = (quads_count / cl);
        u32 last_block_quads = quads_count - wl_blocks_count * cl;
        if (last_block_quads == cl)
            last_block_quads = wl;
        quads_count = wl_blocks_count * wl + last_block_quads;
    }

    packet2_vif_close_unpack_manual(packet2, quads_count);
    return quads_count;
}
