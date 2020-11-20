/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/** @file VU library. */

#ifndef __VU_H__
#define __VU_H__

#include <tamtypes.h>
#include <packet2.h>
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
    // Main
    // ----

    /** 
     * Add VU buffers size settings. 
     * @param t_base Base address (qw)
     * @param t_offset Offset address (qw)
     */
    static inline void vu_add_double_buffer_settings(packet2_t *packet2, u16 base, u16 offset)
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
     * @param t_use_top Double buffer? 
     * When true, data will be loaded at t_dest_address + beginning of current VU buffer. 
     */
    static inline void vu_add_unpack_data(packet2_t *packet2, u32 t_dest_address, void *t_data, u32 t_size, u8 t_use_top)
    {
        packet2_chain_ref(packet2, t_data, t_size, 0, 0, 0);
        packet2_vif_stcycl(packet2, 0, 0x0101, 0);
        packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, t_dest_address + (packet2->vif_added_bytes >> 4), t_use_top, 0, 1, 0);
        packet2_vif_close_unpack(packet2, t_size);
        packet2->vif_added_bytes += t_size * 8;
    }

    /** 
     * Start VU micro program. 
     * Adds MSCAL and FLUSH VIF opcodes. 
     */
    static inline void vu_add_start_program(packet2_t *packet2)
    {
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_mscal(packet2, 0, 0);
        packet2_vif_flush(packet2, 0);
        packet2_chain_close_tag(packet2);
    }

    /** Add aligned END tag. */
    static inline void vu_add_end_tag(packet2_t *packet2)
    {
        packet2_chain_open_end(packet2, 0, 0);
        packet2_vif_nop(packet2, 0);
        packet2_vif_nop(packet2, 0);
        packet2_chain_close_tag(packet2);
    }

    /** Add aligned CNT tag with FLUSH opcode. */
    static inline void vu_add_flush(packet2_t *packet2)
    {
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_nop(packet2, 0);
        packet2_vif_flush(packet2, 0);
        packet2_chain_close_tag(packet2);
    }

    // ----
    // UNPACK
    // ----

    /** Open VU unpack. */
    static inline void vu_open_unpack(packet2_t *packet2)
    {
        packet2->vif_added_bytes = 0;
        packet2_chain_open_cnt(packet2, 0, 0, 0);
        packet2_vif_stcycl(packet2, 0, 0x0101, 0);
        packet2_vif_open_unpack(packet2, P2_UNPACK_V4_32, 0, 1, 0, 1, 0);
    }

    /** Close VU unpack. */
    static inline void vu_close_unpack(packet2_t *packet2)
    {
        packet2_align_to_qword(packet2);
        packet2_chain_close_tag(packet2);
        packet2_vif_close_unpack(packet2, packet2->vif_added_bytes >> 4);
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_u128(packet2_t *packet2, u128 v)
    {
        packet2_add_u128(packet2, v);
        packet2->vif_added_bytes += 16;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_s128(packet2_t *packet2, u128 v)
    {
        packet2_add_s128(packet2, v);
        packet2->vif_added_bytes += 16;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_u64(packet2_t *packet2, u64 v)
    {
        packet2_add_u64(packet2, v);
        packet2->vif_added_bytes += 8;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_2x_s64(packet2_t *packet2, s64 v1, s64 v2)
    {
        packet2_add_s64(packet2, v1);
        packet2_add_s64(packet2, v2);
        packet2->vif_added_bytes += 16;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_s64(packet2_t *packet2, u64 v)
    {
        packet2_add_s64(packet2, v);
        packet2->vif_added_bytes += 8;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_u32(packet2_t *packet2, u32 v)
    {
        packet2_add_u32(packet2, v);
        packet2->vif_added_bytes += 4;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_s32(packet2_t *packet2, u32 v)
    {
        packet2_add_s32(packet2, v);
        packet2->vif_added_bytes += 4;
    }

    /** NOTICE: can be used only with open unpack! */
    static inline void vu_unpack_add_float(packet2_t *packet2, float v)
    {
        packet2_add_float(packet2, v);
        packet2->vif_added_bytes += 4;
    }

    // ----
    // UNPACK -- GS
    // ----

    /** 
     * Add set GIFTag which will be 
     * sent from VU1 via XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param loops_count How many GIF tags there will be? 
     */
    static inline void vu_unpack_add_set(packet2_t *packet2, u32 loops_count)
    {
        vu_unpack_add_2x_s64(packet2, GIF_SET_TAG(loops_count, 0, 0, 0, GIF_FLG_PACKED, 1), GIF_REG_AD);
    }

    /** 
     * Add lod GIFTag which will be 
     * sent from VU1 via XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param lod Pointer to lod settings. 
     */
    static inline void vu_unpack_add_lod(packet2_t *packet2, lod_t *lod)
    {
        vu_unpack_add_2x_s64(
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
    static inline void vu_unpack_add_texbuff_clut(packet2_t *packet2, texbuffer_t *texbuff, clutbuffer_t *clut)
    {
        vu_unpack_add_2x_s64(
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
     * Add drawing GIFTag which will be sent from VU1 via 
     * XGKICK instruction. 
     * @param packet2 Pointer to packet2. 
     * @param prim Pointer to prim settings. 
     * @param loops_count GIF tag loops count. 
     * @param nreg What types of data we will use? 
     * @param nreg_count How many types there are in nreg? 
     * @param context Drawing context 
     */
    static inline void vu_unpack_add_giftag(packet2_t *packet2, prim_t *prim, u32 loops_count, u32 nreg, u8 nreg_count, u8 context)
    {
        vu_unpack_add_2x_s64(
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

    // ----
    // Program code
    // ----

    /** Upload micro program to VU. */
    void vu_upload_program(u32 t_dest, u32 *t_start, u32 *t_end);

    /** Used internally for checking micro program size. */
    u32 vu_count_program_size(u32 *t_start, u32 *t_end);

#ifdef __cplusplus
}
#endif

#endif /* __PACKET2_H__ */
