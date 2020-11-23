/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/** @file Successor of standard packet library. */

#ifndef __PACKET2_H__
#define __PACKET2_H__

#include "./packet2_types.h"
#include "./packet2_chain.h"
#include "./packet2_vif.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // ---
    // Packet2 management
    // ---

    /** 
     * Allocate new packet2. 
     * @param qwords Maximum data size in qwords (128bit). 
     * @param type Memory mapping type. 
     * @param mode Packet mode. Normal or chain (so also vif/vu). 
     * @param tte Tag transfer enable. 
     * Used only for CHAIN mode! 
     * If >0 transfer tag is set during packet sending and 
     * add_dma_tag() (so also every open_tag()) will move buffer by DWORD,
     * so remember to align memory!
     * @returns Pointer to packet2 on success or NULL if memory allocation fail.
     */
    packet2_t *packet2_create(u16 qwords, enum Packet2Type type, enum Packet2Mode mode, u8 tte);

    /** 
     * Create new packet2 with given data pointer. 
     * @param base Pointer to base (start of buffer). 
     * @param next Pointer to next (current position of buffer). 
     * @param qwords Maximum data size in qwords (128bit). 
     * @param type Memory mapping type. 
     * @param mode Packet mode. Normal or chain (so also vif/vu). 
     * @param tte Tag transfer enable. 
     * Used only for CHAIN mode! 
     * If >0 transfer tag is set during packet sending and 
     * add_dma_tag() (so also every open_tag()) will move buffer by DWORD,
     * so remember to align memory!
     * @returns Pointer to packet2 on success or NULL if memory allocation fail.
     */
    packet2_t *packet2_create_from(qword_t *base, qword_t *next, u16 qwords, enum Packet2Type type, enum Packet2Mode mode, u8 tte);

    /** 
     * Free packet2 memory.
     * @param packet2 Pointer to packet2.
     */
    void packet2_free(packet2_t *packet2);

    /** 
     * Reset packet. 
     * Move next pointer back to base pointer, 
     * Make all "openedAt" vars to NULL 
     * Do memset on data if required.
     * @param packet2 Pointer to packet.
     * @param clear_mem If >0, data is cleared via memset(). SLOW! 
     */
    void packet2_reset(packet2_t *packet2, u8 clear_mem);

    /** 
     * Update current position of packet buffer.
     * Useful with drawlib functions. 
     */
    static inline void packet2_update(packet2_t *packet2, qword_t *qw) { packet2->next = qw; }

    // ---
    // Basic add
    // ---

    /** NOTICE: vif_added_bytes increased by 16. */
    static inline void packet2_add_u128(packet2_t *packet2, const u128 val)
    {
        *((u128 *)packet2->next)++ = val;
        packet2->vif_added_bytes += 16;
    }

    /** NOTICE: vif_added_bytes increased by 16. */
    static inline void packet2_add_s128(packet2_t *packet2, const s128 val)
    {
        *((s128 *)packet2->next)++ = val;
        packet2->vif_added_bytes += 16;
    }

    /** 
     * NOTICE: Alignment alert. Size of dword (1/2) 
     * vif_added_bytes increased by 16. 
     */
    static inline void packet2_add_2x_s64(packet2_t *packet2, const s64 v1, const s64 v2)
    {
        *((s64 *)packet2->next)++ = v1;
        *((s64 *)packet2->next)++ = v2;
        packet2->vif_added_bytes += 16;
    }

    /** 
     * NOTICE: Alignment alert. Size of dword (1/2) 
     * vif_added_bytes increased by 8. 
     */
    static inline void packet2_add_u64(packet2_t *packet2, const u64 val)
    {
        *((u64 *)packet2->next)++ = val;
        packet2->vif_added_bytes += 8;
    }

    /** 
     * NOTICE: Alignment alert. Size of dword (1/2) 
     * vif_added_bytes increased by 8. 
     */
    static inline void packet2_add_s64(packet2_t *packet2, const s64 val)
    {
        *((s64 *)packet2->next)++ = val;
        packet2->vif_added_bytes += 8;
    }

    /** 
     * NOTICE: Alignment alert. Size of word (1/4) 
     * vif_added_bytes increased by 4. 
     */
    static inline void packet2_add_u32(packet2_t *packet2, const u32 val)
    {
        *((u32 *)packet2->next)++ = val;
        packet2->vif_added_bytes += 4;
    }

    /** 
     * NOTICE: Alignment alert. Size of word (1/4) 
     * vif_added_bytes increased by 4. 
     */
    static inline void packet2_add_s32(packet2_t *packet2, const s32 val)
    {
        *((s32 *)packet2->next)++ = val;
        packet2->vif_added_bytes += 4;
    }

    /** 
     * NOTICE: Alignment alert. Size of word (1/4)
     * vif_added_bytes increased by 4. 
     */
    static inline void packet2_add_float(packet2_t *packet2, const float val)
    {
        *((float *)packet2->next)++ = val;
        packet2->vif_added_bytes += 4;
    }

    // ---
    // Other
    // ---

    /** 
     * Print data of packet. 
     * @param packet2 Pointer to packet.
     * @param clear_mem Count of qwords, type 0 for all. 
     */
    void packet2_print(packet2_t *packet2, u32 qw_count);

    /** 
     * Print amount of qwords of packet. 
     * @param packet2 Pointer to packet.
     */
    void packet2_print_qw_count(packet2_t *packet2);

    /** Copy data from b packet into a packet with memcpy() */
    void packet2_add(packet2_t *a, packet2_t *b);

    /** Returns count of added qwords into packet. */
    static inline u32 packet2_get_qw_count(packet2_t *packet2) { return ((u32)packet2->next - (u32)packet2->base) >> 4; }

    /** Returns count of added qwords into VU. */
    static inline u32 packet2_get_vif_added_qws(packet2_t *packet2) { return packet2->vif_added_bytes >> 4; }

    /** True if packet doesnt have even number of quads. */
    static inline u8 packet2_doesnt_have_even_number_of_quads(packet2_t *packet2) { return ((u32)packet2->next & 0xF) != 0; }

    /** True if dma tag is opened. */
    static inline u8 packet2_is_dma_tag_opened(packet2_t *packet2) { return packet2->tag_opened_at != NULL; }

    /** True if DIRECT/UNPACK is opened. */
    static inline u8 packet2_is_vif_code_opened(packet2_t *packet2) { return packet2->vif_code_opened_at != NULL; }

    /** Align packet2 to next qword if we are in the middle of qword. */
    static inline void packet2_align_to_qword(packet2_t *packet2)
    {
        if ((u32)packet2->next % 16)
            *((u32 *)packet2->next)++ = 0;
    }

#ifdef __cplusplus
}
#endif

#endif /* __PACKET2_H__ */
