/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/** 
 * @file Chain mode related functions for packet2.
 * @defgroup packet2_chain Chain
 * Chain mode related functions of libpacket2.
 * @ingroup packet2
 * @{
 */

#ifndef __PACKET2_CHAIN_H__
#define __PACKET2_CHAIN_H__

#include <packet2.h>
#include <packet2_types.h>
#include <assert.h>

#define PACKET2_COUNT_QWC 1 << 16

#ifdef __cplusplus
extern "C"
{
#endif

    // ---
    // DMA tag management
    // ---

    /** 
     * Set dma tag. 
     * For more details, check description of dma_tag_t. 
     * @param tag Pointer to DMA tag to set. 
     * @param qwc Qword count. 
     * @param pce Priority Control Enable. 0 by default.
     * @param id Tag ID. 
     * @param irq Interrupt Request. False by default.
     * @param addr Address. 
     * @param spr Memory/SPR Selection. False by default.
     */
    static inline void packet2_chain_set_dma_tag(dma_tag_t *tag, u32 qwc, u32 pce, u32 id, u8 irq, const u128 *addr, u8 spr)
    {
        tag->QWC = qwc;
        tag->PCE = pce;
        tag->ID = id;
        tag->IRQ = irq;
        tag->ADDR = (u64)((u32)addr);
        tag->SPR = spr;
    }

    /** 
     * Add dma tag. 
     * For more details, check description of dma_tag_t. 
     * Will move current buffer position by: 
     * DWORD if packet->tte == true, 
     * QWORD if packet->tte == false
     * @param packet2 Pointer to packet. 
     * @param qwc Qword count. If you want to use 
     * packet2_chain_close_tag(), and count qwords 
     * automatically, set value to PACKET2_COUNT_QWC. 
     * @param pce Priority Control Enable. 0 by default.
     * @param id Tag ID. 
     * @param irq Interrupt Request. False by default.
     * @param addr Address. 
     * @param spr Memory/SPR Selection. False by default.
     */
    static inline void packet2_chain_add_dma_tag(packet2_t *packet2, u32 qwc, u32 pce, enum DmaTagType id, u8 irq, const u128 *addr, u8 spr)
    {
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        assert(packet2->tag_opened_at == NULL);  // All previous tags are closed.

        if (qwc == PACKET2_COUNT_QWC)
        {
            packet2_chain_set_dma_tag((dma_tag_t *)packet2->next, 0, pce, id, irq, addr, spr); // placeholder
            packet2->tag_opened_at = (dma_tag_t *)packet2->next;
        }
        else
        {
            packet2_chain_set_dma_tag((dma_tag_t *)packet2->next, qwc, pce, id, irq, addr, spr);
            packet2->tag_opened_at = (dma_tag_t *)NULL;
        }
        if (!packet2->tte)
            packet2_advance_next(packet2, sizeof(dma_tag_t));
        else
            packet2_advance_next(packet2, sizeof(u64));
    }

    /** 
     * Close dma tag. 
     * In reality, get back to pointer "tag_opened_at" and fix qwc value. 
     * @param packet2 Pointer to packet. 
     */
    static inline void packet2_chain_close_tag(packet2_t *packet2)
    {
        assert(packet2->tag_opened_at != NULL);  // There is opened tag.
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        packet2->tag_opened_at->QWC = (((u32)packet2->next - (u32)packet2->tag_opened_at) / 16 - 1);
        packet2->tag_opened_at = (dma_tag_t *)NULL;
    }

    // ---
    // DMA tags
    // ---

    /** 
     * Add CNT tag. 
     * @note packet2_chain_close_tag() required. Qwords 
     * are counted automatically. 
     * For more details, check description of dma_tag_t. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default.
     * @param pce Priority Control Enable. 0 by default.
     * @param spr Memory/SPR Selection. False by default.
     */
    static inline void packet2_chain_open_cnt(packet2_t *packet2, u8 irq, u32 pce, u8 spr)
    {
        packet2_chain_add_dma_tag(packet2, PACKET2_COUNT_QWC, pce, P2_DMA_TAG_CNT, irq, 0, spr);
    }

    /** 
     * Add END tag. 
     * @note packet2_chain_close_tag() required. Qwords 
     * are counted automatically. 
     * For more details, check description of dma_tag_t. 
     * @note Don't forget about close_tag()!
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default.
     * @param pce Priority Control Enable. 0 by default.
     */
    static inline void packet2_chain_open_end(packet2_t *packet2, u8 irq, u32 pce)
    {
        packet2_chain_add_dma_tag(packet2, PACKET2_COUNT_QWC, pce, P2_DMA_TAG_END, irq, 0, 0);
    }

    /** 
     * Add REF tag. 
     * @note Qwords are NOT counted automatically, so 
     * there is NO need to use packet2_chain_close_tag(); 
     * For more details, check description of dma_tag_t. 
     * @param packet2 Pointer to packet. 
     * @param ref_data Pointer to data. 
     * @param qw_length Size of data in qwords. 
     * @param irq Interrupt Request. False by default.
     * @param spr Memory/SPR Selection. False by default.
     * @param pce Priority Control Enable. 0 by default.
     */
    static inline void packet2_chain_ref(packet2_t *packet2, const void *ref_data, u32 qw_length, u8 irq, u8 spr, u32 pce)
    {
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        packet2_chain_add_dma_tag(packet2, qw_length, pce, P2_DMA_TAG_REF, irq, (const u128 *)ref_data, spr);
    }

    /** 
     * Add NEXT tag. 
     * @note packet2_chain_close_tag() required. Qwords 
     * are counted automatically. 
     * For more details, check description of dma_tag_t. 
     * @param next_tag Pointer to next tag. 
     * @param irq Interrupt Request. False by default. 
     * @param spr Memory/SPR Selection. False by default. 
     * @param pce Priority Control Enable. 0 by default. 
     */
    static inline void packet2_chain_next(packet2_t *packet2, const dma_tag_t *next_tag, u8 irq, u8 spr, u32 pce)
    {
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        packet2_chain_add_dma_tag(packet2, PACKET2_COUNT_QWC, pce, P2_DMA_TAG_NEXT, irq, (u128 *)next_tag, spr);
    }

    /** 
     * Add REFS tag.
     * @note Qwords are NOT counted automatically, so 
     * there is NO need to use packet2_chain_close_tag();
     * For more details, check description of dma_tag_t. 
     * @param packet2 Pointer to packet. 
     * @param ref_data Pointer to data. 
     * @param qw_length Size of data in qwords. 
     * @param irq Interrupt Request. False by default.
     * @param spr Memory/SPR Selection. False by default.
     * @param pce Priority Control Enable. 0 by default.
     */
    static inline void packet2_chain_refs(packet2_t *packet2, const void *ref_data, u32 qw_length, u8 irq, u8 spr, u32 pce)
    {
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        packet2_chain_add_dma_tag(packet2, qw_length, pce, P2_DMA_TAG_REFS, irq, (const u128 *)ref_data, spr);
    }

    /** 
     * Add REFE tag.
     * @note Qwords are NOT counted automatically, so 
     * there is NO need to use packet2_chain_close_tag();
     * For more details, check description of dma_tag_t. 
     * @param packet2 Pointer to packet. 
     * @param ref_data Pointer to data. 
     * @param qw_length Size of data in qwords. 
     * @param irq Interrupt Request. False by default.
     * @param spr Memory/SPR Selection. False by default.
     * @param pce Priority Control Enable. 0 by default.
     */
    static inline void packet2_chain_refe(packet2_t *packet2, const void *ref_data, u32 qw_length, u8 irq, u8 spr, u32 pce)
    {
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        packet2_chain_add_dma_tag(packet2, qw_length, pce, P2_DMA_TAG_REFE, irq, (const u128 *)ref_data, spr);
    }

    /** 
     * Add CALL tag. 
     * @note packet2_chain_close_tag() required. Qwords 
     * are counted automatically. 
     * For more details, check description of dma_tag_t. 
     * @param packet2 Pointer to packet. 
     * @param next_tag Pointer to next tag. 
     * @param irq Interrupt Request. False by default.
     * @param spr Memory/SPR Selection. False by default.
     * @param pce Priority Control Enable. 0 by default.
     */
    static inline void packet2_chain_call(packet2_t *packet2, const void *next_tag, u8 irq, u8 spr, u32 pce)
    {
        assert(((u32)packet2->next & 0xF) == 0); // Free space in packet is aligned properly.
        packet2_chain_add_dma_tag(packet2, PACKET2_COUNT_QWC, pce, P2_DMA_TAG_CALL, irq, (u128 *)next_tag, spr);
    }

    /** 
     * Add RET tag. 
     * @note packet2_chain_close_tag() required. Qwords 
     * are counted automatically. 
     * For more details, check description of dma_tag_t. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default.
     * @param pce Priority Control Enable. 0 by default.
     */
    static inline void packet2_chain_ret(packet2_t *packet2, u8 irq, u32 pce)
    {
        packet2_chain_add_dma_tag(packet2, PACKET2_COUNT_QWC, pce, P2_DMA_TAG_RET, irq, 0, 0);
    }

#ifdef __cplusplus
}
#endif

#endif /* __PACKET2_CHAIN_H__ */

/** @} */ // end of packet2_chain subgroup
