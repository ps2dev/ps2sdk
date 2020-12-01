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
 * @file Helper functions for packet2.
 * @defgroup packet2_utils Utils
 * Useful functions and examples for libpacket2.
 * @ingroup packet2
 * @{
 */

#ifndef __PACKET2_UTILS_H__
#define __PACKET2_UTILS_H__

#include "packet2.h"
#include "packet2_chain.h"
#include "packet2_vif.h"

#include <tamtypes.h>
#include <draw3d.h>
#include <draw_buffers.h>
#include <draw_sampling.h>
#include <gs_gp.h>

#define VU_GS_PRIM(PRIM, IIP, TME, FGE, ABE, AA1, FST, CTXT, FIX) (u128)(((FIX << 10) | (CTXT << 9) | (FST << 8) | (AA1 << 7) | (ABE << 6) | (FGE << 5) | (TME << 4) | (IIP << 3) | (PRIM)))
#define VU_GS_GIFTAG(NLOOP, EOP, PRE, PRIM, FLG, NREG) (((u64)(NREG) << 60) | ((u64)(FLG) << 58) | ((u64)(PRIM) << 47) | ((u64)(PRE) << 46) | (EOP << 15) | (NLOOP << 0))

#ifdef __cplusplus
extern "C"
{
#endif

    // ----
    // VU
    // ----

    /** 
     * Add VU buffers size settings. 
     * @param t_base Base address (qw)
     * @param t_offset Offset address (qw)
     */
    static inline void packet2_utils_vu_add_double_buffer(packet2_t *packet2, u16 base, u16 offset)
    {
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_base(packet2, base, 0);
        packet2_vif_offset(packet2, offset, 0);
        packet2_chain_close_tag(packet2);
    }

    /** 
     * Add data via REF tag and UNPACK. 
     * Please be sure that data is aligned, 
     * and there is no any open UNPACK/DIRECT. 
     * @param packet2 Pointer to packet2. 
     * @param t_dest_address Destination address. 
     * @param t_data Data pointer. 
     * @param t_size Size in quadwords. 
     * @param t_use_top Unpack to current double buffer? 
     */
    static inline void packet2_utils_vu_add_unpack_data(packet2_t *packet2, u32 t_dest_address, void *t_data, u32 t_size, u8 t_use_top)
    {
        packet2_chain_ref(packet2, t_data, t_size, 0, 0, 0);
        packet2_vif_stcycl(packet2, 0, 0x0101, 0);
        packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, t_dest_address, t_use_top, 0, 1, 0);
        packet2_vif_close_unpack_manual(packet2, t_size);
    }

    /** 
     * Open CNT tag + VU unpack. 
     * @param packet2 Pointer to packet2. 
     * @param t_dest_address Destination address. 
     * @param t_use_top Unpack to current double buffer? 
     */
    static inline void packet2_utils_vu_open_unpack(packet2_t *packet2, u32 t_dest_address, u8 t_use_top)
    {
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_stcycl(packet2, 0, 0x0101, 0);
        packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, t_dest_address, t_use_top, 0, 1, 0);
    }

    /** 
     * Close CNT tag + VU unpack. 
     * @returns Unpacked qwords count
     */
    static inline u32 packet2_utils_vu_close_unpack(packet2_t *packet2)
    {
        packet2_vif_pad128(packet2);
        packet2_chain_close_tag(packet2);
        return packet2_vif_close_unpack_auto(packet2, 0, 0x0101);
    }

    /** 
     * Continue VU micro program (from --cont line). 
     * Adds FLUSH and MSCNT VIF opcodes. 
     */
    static inline void packet2_utils_vu_add_continue_program(packet2_t *packet2)
    {
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_flush(packet2, 0);
        packet2_vif_mscnt(packet2, 0);
        packet2_chain_close_tag(packet2);
    }

    /** 
     * Start VU micro program. 
     * Adds FLUSH and MSCAL VIF opcodes. 
     */
    static inline void packet2_utils_vu_add_start_program(packet2_t *packet2, u32 addr)
    {
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_flush(packet2, 0);
        packet2_vif_mscal(packet2, addr, 0);
        packet2_chain_close_tag(packet2);
    }

    /** Add aligned END tag (with nops). */
    static inline void packet2_utils_vu_add_end_tag(packet2_t *packet2)
    {
        packet2_chain_open_end(packet2, 0, 0);
        packet2_vif_nop(packet2, 0);
        packet2_vif_nop(packet2, 0);
        packet2_chain_close_tag(packet2);
    }

    /** 
     * Get program size in qws 
     * @param start Start address. 
     * @param end End address. 
     * @returns Packet size in qws for specified program (instructions/256 + 1qw for tolerance).
     */
    static inline u32 packet2_utils_get_packet_size_for_program(u32 *start, u32 *end)
    {
        // Count instructions
        u32 count = (end - start) / 2;
        if (count & 1)
            count++;
        return (count >> 8) + 1;
    }

    // ----
    // GIF
    // ----

    /** 
     * Add set GIFTag which will be 
     * sent from VU1 via XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param loops_count How many GIF tags there will be? 
     */
    static inline void packet2_utils_gif_add_set(packet2_t *packet2, u32 loops_count)
    {
        packet2_add_2x_s64(packet2, GIF_SET_TAG(loops_count, 0, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
    }

    // ----
    // GS
    // ----

    /** 
     * Add lod GIFTag which will be 
     * sent from VU1 via XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param lod Pointer to lod settings. 
     */
    static inline void packet2_utils_gs_add_lod(packet2_t *packet2, lod_t *lod)
    {
        packet2_add_2x_s64(
            packet2,
            GS_SET_TEX1(
                lod->calculation,
                lod->max_level,
                lod->mag_filter,
                lod->min_filter,
                lod->mipmap_select,
                lod->l,
                (int)(lod->k * 16.0F)),
            GS_REG_TEX1);
    }

    /** 
     * Add texture buffer / clut GIFTag which will be 
     * sent from VU1 via XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param texbuff Pointer to texture buffer. 
     * @param clut Pointer to clut buffer. 
     */
    static inline void packet2_utils_gs_add_texbuff_clut(packet2_t *packet2, texbuffer_t *texbuff, clutbuffer_t *clut)
    {
        packet2_add_2x_s64(
            packet2,
            GS_SET_TEX0(
                texbuff->address >> 6,
                texbuff->width >> 6,
                texbuff->psm,
                texbuff->info.width,
                texbuff->info.height,
                texbuff->info.components,
                texbuff->info.function,
                clut->address >> 6,
                clut->psm,
                clut->storage_mode,
                clut->start,
                clut->load_method),
            GS_REG_TEX0);
    }

    /** 
     * Add draw finish event GIFTag which will be sent from VU1 via 
     * XGKICK instruction. 
     * Used for synchronization via draw_wait_finish() 
     * @param packet2 Pointer to packet2. 
     */
    static inline void packet2_utils_gs_add_draw_finish_giftag(packet2_t *packet2)
    {
        packet2_add_2x_s64(packet2, 1, GS_REG_FINISH);
    }

    /** 
     * Add drawing (prim) GIFTag which will be sent from VU1 via 
     * XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param prim Pointer to prim settings. 
     * @param loops_count GIF tag loops count. 
     * @param nreg What types of data we will use? 
     * @param nreg_count How many types there are in nreg? 
     * @param context Drawing context 
     */
    static inline void packet2_utils_gs_add_prim_giftag(packet2_t *packet2, prim_t *prim, u32 loops_count, u32 nreg, u8 nreg_count, u8 context)
    {
        packet2_add_2x_s64(
            packet2,
            VU_GS_GIFTAG(
                loops_count, // Information for GS. Amount of loops
                1,
                1,
                VU_GS_PRIM(
                    prim->type,
                    prim->shading,
                    prim->mapping,
                    prim->fogging,
                    prim->blending,
                    prim->antialiasing,
                    prim->mapping_type,
                    context, // context
                    prim->colorfix),
                0,
                nreg_count),
            nreg);
    }

#ifdef __cplusplus
}
#endif

#endif /* __PACKET2_UTILS_H__ */

/** @} */ // end of packet2_utils subgroup
