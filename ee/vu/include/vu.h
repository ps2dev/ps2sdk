/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/** @file VU library. */

#ifndef __VU_H__
#define __VU_H__

#include <tamtypes.h>
#include <packet2.h>

#define GS_PRIM(PRIM, IIP, TME, FGE, ABE, AA1, FST, CTXT, FIX) (u128)(((FIX << 10) | (CTXT << 9) | (FST << 8) | (AA1 << 7) | (ABE << 6) | (FGE << 5) | (TME << 4) | (IIP << 3) | (PRIM)))
#define GS_GIFTAG(NLOOP, EOP, PRE, PRIM, FLG, NREG) (((u64)(NREG) << 60) | ((u64)(FLG) << 58) | ((u64)(PRIM) << 47) | ((u64)(PRE) << 46) | (EOP << 15) | (NLOOP << 0))

#ifdef __cplusplus
extern "C"
{
#endif

    // ----
    // Main
    // ----

    /** 
 * Send data to VU mem. 
 * Not using TOP register (double buffer). 
 * Please be sure that data is aligned. 
 * @param t_dest_address At what VU mem qword data should be unpacked? 
 * @param t_data Pointer to data. 
 * @param t_quad_size Size of data in quadwords. 
 */
    void vu_send_data(u32 t_dest_address, void *t_data, u32 t_quad_size);

    /** 
 * Send VU buffers size to VU. 
 * @param t_base Base address (qw)
 * @param t_offset Offset address (qw)
 */
    void vu_send_double_buffer_settings(u16 t_base, u16 t_offset);

    /** 
 * Add data via pointer. 
 * Please be sure that data is aligned. 
 * @param t_offset Offset before data in qwords. 
 * @param t_data Data pointer. 
 * @param t_size Size in quadwords. 
 * @param t_use_top When true, data will be loaded at the beginning of current VU buffer. 
 */
    void vu_add_data(packet2_t *packet2, void *t_data, u32 t_size, u8 t_use_top);

    /** Start VU micro program. */
    void vu_add_start_program(packet2_t *packet2);

    /** Add end tag and send packet to VU. */
    void vu_send_packet(packet2_t *packet2);

    // ----
    // UNPACK
    // ----

    /** Add flush and unpack. */
    void vu_open_unpack(packet2_t *packet2);

    /** Close VU unpack. */
    void vu_close_unpack(packet2_t *packet2);

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
