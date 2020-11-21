/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "packet2_vu.h"

u32 packet2_vu_count_program_instructions(u32 *start, u32 *end)
{
    u32 size = (end - start) / 2;
    if (size & 1)
        size++;
    return size;
}

void packet2_vu_add_micro_program(packet2_t *packet2, u32 dest, u32 *start, u32 *end)
{
    // get the size of the code as we can only send 256 instructions in each MPGtag
    u32 count = packet2_vu_count_program_instructions(start, end);
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
