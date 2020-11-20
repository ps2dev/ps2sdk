/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <dma.h>
#include <packet2.h>
#include "vu.h"

// ----
// Main
// ----

void vu_send_data(u32 t_dest_address, void *t_data, u32 t_quad_size)
{
    packet2_t *packet2 = packet2_create_chain(2, P2_TYPE_NORMAL, 1);
    packet2_chain_ref(packet2, t_data, t_quad_size, 0, 0, 0);
    packet2_vif_stcycl(packet2, 0, 0x0101, 0);
    packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, t_dest_address, 0, 0, 0, 0);
    packet2_vif_close_unpack(packet2, t_quad_size);
    packet2_chain_open_end(packet2, 0, 0);
    packet2_vif_nop(packet2, 0);
    packet2_vif_nop(packet2, 0);
    packet2_chain_close_tag(packet2);
    dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
    dma_channel_wait(DMA_CHANNEL_VIF1, 0);
    packet2_free(packet2);
}

void vu_send_double_buffer_settings(u16 base, u16 offset)
{
    packet2_t *packet2 = packet2_create_chain(1, P2_TYPE_NORMAL, 1);
    packet2_chain_open_end(packet2, 0, 0);
    packet2_vif_base(packet2, base, 0);
    packet2_vif_offset(packet2, offset, 0);
    packet2_chain_close_tag(packet2);
    dma_channel_wait(DMA_CHANNEL_VIF1, 0);
    dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
    dma_channel_wait(DMA_CHANNEL_VIF1, 0);
    packet2_free(packet2);
}

void vu_add_data(packet2_t *packet2, void *t_data, u32 t_size, u8 t_use_top)
{
    packet2_chain_ref(packet2, t_data, t_size, 0, 0, 0);
    packet2_vif_stcycl(packet2, 0, 0x0101, 0);
    packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, packet2->vif_added_bytes >> 4, t_use_top, 0, 1, 0);
    packet2_vif_close_unpack(packet2, t_size);
    packet2->vif_added_bytes += t_size * 8;
}

void vu_add_start_program(packet2_t *packet2)
{
    packet2_chain_open_cnt(packet2, 0, 0, 0);
    packet2_vif_mscal(packet2, 0, 0);
    packet2_vif_flush(packet2, 0);
    packet2_chain_close_tag(packet2);
}

void vu_send_packet(packet2_t *packet2)
{
    packet2_chain_open_end(packet2, 0, 0);
    packet2_vif_nop(packet2, 0);
    packet2_vif_nop(packet2, 0);
    packet2_chain_close_tag(packet2);
    dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
    dma_channel_wait(DMA_CHANNEL_VIF1, 0);
}

// ----
// UNPACK
// ----

void vu_open_unpack(packet2_t *packet2)
{
    packet2->vif_added_bytes = 0;
    packet2_chain_open_cnt(packet2, 0, 0, 0);
    packet2_vif_nop(packet2, 0);
    packet2_vif_flush(packet2, 0);
    packet2_chain_close_tag(packet2);
    packet2_chain_open_cnt(packet2, 0, 0, 0);
    packet2_vif_stcycl(packet2, 0, 0x0101, 0);
    packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, 0, 1, 0, 1, 0);
}

void vu_close_unpack(packet2_t *packet2)
{
    packet2_align_to_qword(packet2);
    packet2_chain_close_tag(packet2);
    packet2_vif_close_unpack(packet2, packet2->vif_added_bytes >> 4);
}

// ----
// Program code
// ----

u32 vu_count_program_size(u32 *t_start, u32 *t_end)
{
    u32 size = (t_end - t_start) / 2;
    if (size & 1)
        size++;
    return size;
}

void vu_upload_program(u32 t_dest, u32 *t_start, u32 *t_end)
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
    dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
    dma_channel_wait(DMA_CHANNEL_VIF1, 0);
}
