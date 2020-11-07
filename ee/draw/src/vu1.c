/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <dma.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <kernel.h>
#include <vu1.h>

static VU1BuildList vu1_build_list;
static u8
    vu1_is_double_buffer_set,
    vu1_using_double_buffer,
    vu1_is_initialized,
    vu1_switch_buffer;
static u16
    vu1_double_buffer_base,
    vu1_double_buffer_offset;
static u32 vu1_buffer_size;
static void *vu1_current_buffer;
static char **vu1_dma_buffer __attribute__((aligned(128)));
static char **vu1_dma_buffer_2 __attribute__((aligned(128)));

void vu1_init(u32 t_dma_buffer_size, u8 t_use_double_buffer, u16 t_double_buffer_base, u16 t_double_buffer_offset)
{
    if (vu1_is_initialized)
        vu1_free();
    else
    {
        dma_channel_initialize(DMA_CHANNEL_VIF1, NULL, 0);
        dma_channel_fast_waits(DMA_CHANNEL_VIF1);
    }
    vu1_is_initialized = 1;
    vu1_using_double_buffer = t_use_double_buffer;
    if (vu1_using_double_buffer)
    {
        vu1_double_buffer_base = t_double_buffer_base;
        vu1_double_buffer_base = t_double_buffer_offset;
    }
    vu1_is_double_buffer_set = 0;
    vu1_switch_buffer = 0;
    vu1_buffer_size = t_dma_buffer_size;
    vu1_dma_buffer = (char **)memalign(128, sizeof(char *) * t_dma_buffer_size);
    vu1_dma_buffer_2 = (char **)memalign(128, sizeof(char *) * t_dma_buffer_size);
}

void vu1_free()
{
    if (vu1_is_initialized && vu1_buffer_size)
    {
        free(vu1_dma_buffer);
        free(vu1_dma_buffer_2);
    }
}

void vu1_create_dyn_list()
{
    vu1_switch_buffer = !vu1_switch_buffer;
    memset((char *)&vu1_build_list, 0, sizeof(vu1_build_list));

    if (vu1_switch_buffer)
        vu1_current_buffer = (char *)&vu1_dma_buffer;
    else
        vu1_current_buffer = (char *)&vu1_dma_buffer_2;
    vu1_build_list.kick_buffer = vu1_current_buffer;
}

void vu1_send_single_ref_list(u32 t_dest_address, void *t_data, u32 t_quad_size)
{
    u8 tempBuffer[32] __attribute__((aligned(16)));
    void *chain = (u64 *)&tempBuffer; // uncached

    *((u64 *)chain)++ = DMA_REF_TAG((u32)t_data, t_quad_size);
    *((u32 *)chain)++ = VIF_CODE(VIF_STCYL, 0, 0x0101);
    *((u32 *)chain)++ = VIF_CODE(VIF_UNPACK_V4_32, t_quad_size, t_dest_address);

    *((u64 *)chain)++ = DMA_END_TAG(0);
    *((u32 *)chain)++ = VIF_CODE(VIF_NOP, 0, 0);
    *((u32 *)chain)++ = VIF_CODE(VIF_NOP, 0, 0);

    FlushCache(0);
    dma_channel_send_chain(DMA_CHANNEL_VIF1, tempBuffer, 0, DMA_FLAG_TRANSFERTAG, 0);
    dma_channel_wait(DMA_CHANNEL_VIF1, VU1_DMA_CHAN_TIMEOUT);
}

void vu1_send()
{
    *((u64 *)vu1_current_buffer)++ = DMA_END_TAG(0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_NOP, 0, 0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_NOP, 0, 0);
    FlushCache(0);
    dma_channel_send_chain(DMA_CHANNEL_VIF1, vu1_build_list.kick_buffer, 0, DMA_FLAG_TRANSFERTAG, 0);
    dma_channel_wait(DMA_CHANNEL_VIF1, VU1_DMA_CHAN_TIMEOUT);
}

void vu1_check_list()
{
    if (vu1_build_list.is_building == 0)
        printf("VU1: Please add list beginning before adding data!\n");
    if (vu1_build_list.dma_size_all > vu1_buffer_size)
        printf("VU1: Buffer size exceed!\n");
}

u32 vu1_count_program_size(u32 *t_start, u32 *t_end)
{
    u32 size = (t_end - t_start) / 2;
    if (size & 1)
        size++;
    return size;
}

void vu1_upload_program(u32 t_dest, u32 *t_start, u32 *t_end)
{
    u32 count = 0;
    u8 tempBuffer[512] __attribute__((aligned(16)));
    void *chain = (u64 *)&tempBuffer; // uncached

    // get the size of the code as we can only send 256 instructions in each MPGtag
    count = vu1_count_program_size(t_start, t_end);
    while (count > 0)
    {
        u32 currentCount = count > 256 ? 256 : count;

        *((u64 *)chain)++ = DMA_REF_TAG((u32)t_start, currentCount / 2);
        *((u32 *)chain)++ = VIF_CODE(VIF_NOP, 0, 0);
        *((u32 *)chain)++ = VIF_CODE(VIF_MPG, currentCount & 0xFF, t_dest);

        t_start += currentCount * 2;
        count -= currentCount;
        t_dest += currentCount;
    }

    *((u64 *)chain)++ = DMA_END_TAG(0);
    *((u32 *)chain)++ = VIF_CODE(VIF_NOP, 0, 0);
    *((u32 *)chain)++ = VIF_CODE(VIF_NOP, 0, 0);

    // Send it to vif1
    FlushCache(0);
    dma_channel_send_chain(DMA_CHANNEL_VIF1, tempBuffer, 0, DMA_FLAG_TRANSFERTAG, 0);
    dma_channel_wait(DMA_CHANNEL_VIF1, VU1_DMA_CHAN_TIMEOUT); // synchronize immediately.
}

// ----
// List adding
// ----

void vu1_add_dyn_list_beginning()
{
    if (vu1_build_list.is_building == 1)
    {
        printf("VU1: Please end current list before adding new one!\n");
        return;
    }

    if (vu1_using_double_buffer && !vu1_is_double_buffer_set)
    {
        *((u64 *)vu1_current_buffer)++ = DMA_CNT_TAG(0);
        *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_BASE, 0, vu1_double_buffer_base);
        *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_OFFSET, 0, vu1_double_buffer_offset);
        vu1_is_double_buffer_set = 1;
    }
    else
        vu1_add_flush();

    vu1_build_list.dma_size_all += vu1_build_list.dma_size;
    vu1_build_list.dma_size = 0;
    vu1_build_list.offset = vu1_current_buffer;
    *((u64 *)vu1_current_buffer)++ = DMA_CNT_TAG(0);                   // placeholder
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_STCYL, 0, 0x0101);   // placeholder
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_UNPACK_V4_32, 0, 0); // placeholder
    vu1_build_list.is_building = 1;
}

void vu1_add_flush()
{
    *((u64 *)vu1_current_buffer)++ = DMA_CNT_TAG(0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_NOP, 0, 0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_FLUSH, 0, 0);
}

void vu1_add_dyn_list_ending()
{
    if (!vu1_build_list.is_building)
    {
        printf("VU1: Please add list beginning first. Nothing to end!\n");
        return;
    }

    while ((vu1_build_list.dma_size & 0xF))
    {
        *((u32 *)vu1_current_buffer)++ = 0;
        vu1_build_list.dma_size += 4;
    }

    *((u64 *)vu1_build_list.offset)++ = DMA_CNT_TAG(vu1_build_list.dma_size >> 4);
    *((u32 *)vu1_build_list.offset)++ = VIF_CODE(VIF_STCYL, 0, 0x0101);
    *((u32 *)vu1_build_list.offset)++ = VIF_EXT_UNPACK(V4_32, 0, (vu1_build_list.dma_size >> 4), 1, 1, 0);

    vu1_build_list.is_building = 0;
}

void vu1_add_reference_list(u32 t_offset, void *t_data, u32 t_size, u8 t_use_top)
{
    if (vu1_build_list.is_building)
    {
        printf("VU1: Please end current list before adding new one!\n");
        return;
    }
    *((u64 *)vu1_current_buffer)++ = DMA_REF_TAG((u32)t_data, t_size);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_STCYL, 0, 0x0101);
    *((u32 *)vu1_current_buffer)++ =
        VIF_EXT_UNPACK(V4_32, (t_use_top == 1 ? vu1_build_list.dma_size >> 4 : t_offset >> 4), t_size, t_use_top, 1, 0);
    vu1_build_list.dma_size += t_size * 8; // 1 qw = 8
    vu1_build_list.dma_size_all += t_size * 8;
}

void vu1_add_start_program()
{
    *((u64 *)vu1_current_buffer)++ = DMA_CNT_TAG(0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_MSCAL, 0, 0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_FLUSH, 0, 0);
}

void vu1_add_continue_program()
{
    *((u64 *)vu1_current_buffer)++ = DMA_CNT_TAG(0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_MSCNT, 0, 0);
    *((u32 *)vu1_current_buffer)++ = VIF_CODE(VIF_FLUSH, 0, 0);
}

void vu1_dyn_add_128(u64 v1, u64 v2)
{
    vu1_check_list();
    *((u64 *)vu1_current_buffer)++ = v1;
    *((u64 *)vu1_current_buffer)++ = v2;
    vu1_build_list.dma_size += 16;
}

void vu1_dyn_add_64(u64 v)
{
    vu1_check_list();
    *((u64 *)vu1_current_buffer)++ = v;
    vu1_build_list.dma_size += 8;
}

void vu1_dyn_add_32(u32 v)
{
    vu1_check_list();
    *((u32 *)vu1_current_buffer)++ = v;
    vu1_build_list.dma_size += 4;
}

void vu1_dyn_add_float(float v)
{
    vu1_check_list();
    *((float *)vu1_current_buffer)++ = v;
    vu1_build_list.dma_size += 4;
}
