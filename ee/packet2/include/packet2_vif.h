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
 * @file VIF related functions for packet2.
 * @defgroup packet2_vif VIF
 * VIF related functions of libpacket2.
 * @ingroup packet2
 * @{
 */

#ifndef __PACKET2_VIF_H__
#define __PACKET2_VIF_H__

#include <packet2.h>
#include <packet2_types.h>
#include <assert.h>

#define MAKE_VIF_CODE(_immediate, _num, _cmd, _irq) ((u32)(_immediate) | ((u32)(_num) << 16) | ((u32)(_cmd) << 24) | ((u32)(_irq) << 31))

#ifdef __cplusplus
extern "C"
{
#endif

    /** 
     * Add UNPACK VIF opcode. 
     * @note packet2_vif_close_unpack() required. Qwords 
     * are counted automatically. 
     * For more details, check description of vif_code_t. 
     * @param packet2 Pointer to packet. 
     * @param mode Unpack mode
     * @param vuAddr Memory address (divided by 16). 
     * @param dblBuffered (VIF1 only) 
     * 1 - Adds VIF1_TOPS register to ADDR 
     * 0 - Does not use VIF1_TOPS register. 
     * @param masked Is masked
     * @param usigned 
     * 1 - Unsigned - Decompress by padding 0 to the upper field 
     * 0 - Signed - Decompress by sign extension 
     * @param irq Interrupt Request. False by default.
     */
    static inline void packet2_vif_open_unpack(packet2_t *packet2, enum UnpackMode mode, u32 vuAddr, u8 dblBuffered, u8 masked, u8 usigned, u8 irq)
    {
        assert(packet2->vif_code_opened_at == NULL); // All previous UNPACK/DIRECT are closed.
        packet2->vif_code_opened_at = (vif_code_t *)packet2->next;
        packet2_add_u32(packet2,
                        MAKE_VIF_CODE(vuAddr | ((u32)usigned << 14) | ((u32)dblBuffered << 15),
                                      0,
                                      mode | ((u32)masked << 4) | 0x60, irq));
    }

    /** 
     * Close UNPACK manually. 
     * In reality, get back to pointer "vif_code_opened_at" and 
     * fix num value with qwords counted from last packet2_vif_open_unpack().
     * @param packet2 Pointer to packet.
     * @param unpack_num Amount of data written to the VU Mem (qwords) or MicroMem (dwords). 
     * 256 is max value! 
     */
    static inline void packet2_vif_close_unpack_manual(packet2_t *packet2, u32 unpack_num)
    {
        assert(packet2->vif_code_opened_at != NULL);               // There is open UNPACK/DIRECT.
        assert(((u32)packet2->next & 0x3) == 0);                   // Make sure we're u32 aligned
        assert((packet2->vif_code_opened_at->cmd & 0x60) == 0x60); // It was UNPACK
        assert(unpack_num <= 256);
        packet2->vif_code_opened_at->num = (unpack_num == 256) ? 0 : unpack_num;
        packet2->vif_code_opened_at = (vif_code_t *)NULL;
    }

    /** 
     * Close UNPACK automatically. 
     * In reality, get back to pointer "vif_code_opened_at" and 
     * fix num value with qwords counted from last packet2_vif_open_unpack().
     * @param packet2 Pointer to packet.
     * @param wl WL Value (look at STCYCL)
     * @param cl CL Value (look at STCYCL)
     * @returns Unpacked qwords count 
     */
    u32 packet2_vif_close_unpack_auto(packet2_t *packet2, u32 wl, u32 cl);

    /** 
     * Add DIRECT VIF opcode. 
     * @note Direct must be closed via 
     * packet2_vif_close_direct_manual() or
     * packet2_vif_close   : "memory"
     */
    static inline void packet2_vif_open_direct(packet2_t *packet2, u8 irq)
    {
        assert(packet2->vif_code_opened_at == NULL); // All previous UNPACK/DIRECT are closed.
        packet2->vif_code_opened_at = (vif_code_t *)packet2->next;
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_DIRECT, irq));
    }

    /** 
     * Close DIRECT manually. 
     * In reality, get back to pointer "vif_code_opened_at" and 
     * fix immediate value with given value. 
     * @param packet2 Pointer to packet. 
     * @param qwords Qwords count. 
     */
    static inline void packet2_vif_close_direct_manual(packet2_t *packet2, u32 qwords)
    {
        assert(packet2->vif_code_opened_at != NULL); // There is open UNPACK/DIRECT.
        assert((((u32)packet2->next - ((u32)packet2->vif_code_opened_at + 4)) & 0xF) == 0);
        packet2->vif_code_opened_at->immediate = qwords;
        packet2->vif_code_opened_at = (vif_code_t *)NULL;
    }

    /** 
     * Close DIRECT automatically. 
     * In reality, get back to pointer "vif_code_opened_at" and 
     * fix immediate value with qwords counted from last packet2_vif_open_direct(). 
     * @param packet2 Pointer to packet. 
     */
    static inline void packet2_vif_close_direct_auto(packet2_t *packet2)
    {
        return packet2_vif_close_direct_manual(packet2, ((u32)packet2->next - ((u32)packet2->vif_code_opened_at + 4)) >> 4);
    }

    /** 
     * Add NOP VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_nop(packet2_t *packet2, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_NOP, irq));
    }

    /**
     * Fill with NOP VIF opcode to align to 96bits
     * @param packet2 Pointer to packet.
     */
    static inline void packet2_vif_pad96(packet2_t *packet2)
    {
        packet2_pad96(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_NOP, 0));
    }

    /**
     * Fill with NOP VIF opcode to align to 128bits
     * @param packet2 Pointer to packet.
     */
    static inline void packet2_vif_pad128(packet2_t *packet2)
    {
        packet2_pad128(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_NOP, 0));
    }

    /** 
     * Add MPG VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param num Number of 64-bit data. 
     * @param addr Address. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_mpg(packet2_t *packet2, u32 num, u32 addr, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(addr, num, P2_VIF_MPG, irq));
    }

    /** 
     * Add STCYCL VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param wl WL field. 
     * @param cl CL field. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_stcycl(packet2_t *packet2, u32 wl, u32 cl, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(cl | (wl << 8), 0, P2_VIF_STCYCL, irq));
    }

    /** 
     * Add OFFSET VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param offset Offset. BASE+OFFSET will be the address of second buffer. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_offset(packet2_t *packet2, u32 offset, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(offset, 0, P2_VIF_OFFSET, irq));
    }

    /** 
     * Add BASE VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param base Base address of double buffer. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_base(packet2_t *packet2, u32 base, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(base, 0, P2_VIF_BASE, irq));
    }

    /** 
     * Add FLUSH VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_flush(packet2_t *packet2, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_FLUSH, irq));
    }

    /** 
     * Add MSCAL VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param addr Starting address. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_mscal(packet2_t *packet2, u32 addr, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(addr, 0, P2_VIF_MSCAL, irq));
    }

    /** 
     * Add MSCNT VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_mscnt(packet2_t *packet2, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_MSCNT, irq));
    }

    /** 
     * Add ITOP VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param itops Value for VU XITOP instruction. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_itop(packet2_t *packet2, u32 itops, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(itops, 0, P2_VIF_ITOP, irq));
    }

    /** 
     * Add MSKPATH3 VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param mode Decompression mode. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_stmod(packet2_t *packet2, u32 mode, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(mode, 0, P2_VIF_STMOD, irq));
    }

    /** 
     * Add MSKPATH3 VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param mask Mask. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_mskpath3(packet2_t *packet2, u32 mask, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(mask, 0, P2_VIF_MSKPATH3, irq));
    }

    /** 
     * Add MARK VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param value Value. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_mark(packet2_t *packet2, u32 value, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(value, 0, P2_VIF_MARK, irq));
    }

    /** 
     * Add FLUSHE VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_flushe(packet2_t *packet2, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_FLUSHE, irq));
    }

    /** 
     * Add FLUSHA VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_flusha(packet2_t *packet2, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_FLUSHA, irq));
    }

    /** 
     * Add MSCALF VIF opcode. 
     * @note Alignment alert. Size of word (1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param addr Address. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_mscalf(packet2_t *packet2, u32 addr, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(addr, 0, P2_VIF_MSCALF, irq));
    }

    /** 
     * Add STMASK VIF opcode. 
     * @note Alignment alert. Size of dword (1/2). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param mask Mask. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_stmask(packet2_t *packet2, Mask mask, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_STMASK, irq));
        packet2_add_u32(packet2, mask.m);
    }

    /** 
     * Add STROW VIF opcode. 
     * @note Alignment alert. Size of 5 x word (1+1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param row_arr Row array. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_strow(packet2_t *packet2, const u32 *row_arr, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_STROW, irq));
        packet2_add_u32(packet2, row_arr[0]);
        packet2_add_u32(packet2, row_arr[1]);
        packet2_add_u32(packet2, row_arr[2]);
        packet2_add_u32(packet2, row_arr[3]);
    }

    /** 
     * Add STCOL VIF opcode. 
     * @note Alignment alert. Size of 5 x word (1+1/4). 
     * For more details, check description of VIFOpcode. 
     * @param packet2 Pointer to packet. 
     * @param col_arr Column array. 
     * @param irq Interrupt Request. False by default. 
     */
    static inline void packet2_vif_stcol(packet2_t *packet2, const u32 *col_arr, u8 irq)
    {
        packet2_add_u32(packet2, MAKE_VIF_CODE(0, 0, P2_VIF_STCOL, irq));
        packet2_add_u32(packet2, col_arr[0]);
        packet2_add_u32(packet2, col_arr[1]);
        packet2_add_u32(packet2, col_arr[2]);
        packet2_add_u32(packet2, col_arr[3]);
    }

    /** 
     * Add VU micro program into packet2. 
     * Packet2 MODE for micro program upload: Chain
     * @param dest VU destination address (divided by 16). 
     * @param start Start address. 
     * @param end End address. 
     */
    void packet2_vif_add_micro_program(packet2_t *packet2, u32 dest, u32 *start, u32 *end);

#ifdef __cplusplus
}
#endif

#endif /* __PACKET2_VIF_H__ */

/** @} */ // end of packet2_vif subgroup
