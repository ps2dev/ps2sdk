/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <dma.h>
#include <packet2.h>
#include "vu.h"

u32 vu_count_program_size(u32 *t_start, u32 *t_end)
{
    u32 size = (t_end - t_start) / 2;
    if (size & 1)
        size++;
    return size;
}

void vu_upload_program(u32 t_dest, u32 *t_start, u32 *t_end, int dma_channel)
{
    // get the size of the code as we can only send 256 instructions in each MPGtag
    int count = vu_count_program_size(t_start, t_end);
    // Size of: count/256 + 1qw for tolerance + 1qw for end tag
    packet2_t *packet2 = packet2_create_chain((count >> 8) + 2, P2_TYPE_NORMAL, 1);

    while (count > 0)
    {
        u32 currentCount = count > 256 ? 256 : count;
        packet2_chain_ref(packet2, t_start, currentCount / 2, 0, 0, 0);
        packet2_vif_nop(packet2, 0);
        packet2_vif_mpg(packet2, currentCount & 0xFF, t_dest, 0);
        t_start += currentCount * 2;
        count -= currentCount;
        t_dest += currentCount;
    }
    packet2_chain_open_end(packet2, 0, 0);
    packet2_vif_nop(packet2, 0);
    packet2_vif_nop(packet2, 0);
    packet2_chain_close_tag(packet2);
    dma_channel_send_packet2(packet2, dma_channel, 1);
    dma_channel_wait(dma_channel, 0);
}
