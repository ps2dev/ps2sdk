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

#ifndef _VU1_
#define _VU1_

#include <tamtypes.h>

typedef struct VU1BuildList
{
    /** Is dynamic list under construction? */
    u8 is_building;
    /** Current offset */
    void *offset;
    /** Beginning of buffer */
    void *kick_buffer;
    /** Size of dynamic list */
    u32 dma_size;
    /** Size of packet dynamic list + other list */
    u32 dma_size_all;
} VU1BuildList;

/**
 * - Initialize VU1 module 
 * - Initialize VU1 DMA channel 
 * - Allocate memory for VU1 DMA two buffers. 
 * Can be used multiple times, DMA buffers will be resized. 
 * @param t_dma_buffer_size Size of buffer 
 * @param t_use_double_buffer When true, VU1 double buffer will be set. VU1 DMA is anyways double buffered. 
 * @param t_double_buffer_base At what qword (offset) of VU1 (not DMA), first buffer start? 
 * @param t_double_buffer_offset Offset to second buffer. Will start at base+offset. 
 */
void vu1_init(u32 t_dma_buffer_size, u8 t_use_double_buffer, u16 t_double_buffer_base, u16 t_double_buffer_offset);

/** Deallocate memory of VU1 DMA two buffers */
void vu1_free();

/** Create dynamic list */
void vu1_create_dyn_list();

/** 
 * Create list, add reference to data and send it. 
 * Reference list is way faster than dynamic list adding. 
 * Not using TOP register (double buffer) 
 * Similar to add_reference_list() 
 * @param t_dest_address At what VU1 mem qword data should be unpacked? 
 * @param t_data Pointer to data. 
 * @param t_quad_size Size of data in quadwords. 
 */
void vu1_send_single_ref_list(u32 t_dest_address, void *t_data, u32 t_quad_size);

/** 
 * Add beginning to dynamic list. 
 * This method is setting double buffer if required. 
 */
void vu1_add_dyn_list_beginning();

/** Add ending to dynamic list. */
void vu1_add_dyn_list_ending();

/** 
 * Add list which will load data from given pointer. 
 * A lot faster than dynamic list. 
 * @param t_offset Offset before data in qwords. 
 * @param t_data Data pointer. 
 * @param t_size Size in quadwords. 
 * @param t_use_top When true, data will be loaded at the beginning of current VU1 buffer. 
 */
void vu1_add_reference_list(u32 t_offset, void *t_data, u32 t_size, u8 t_use_top);

/** Start VU1 micro program. */
void vu1_add_start_program();

/** Continue VU1 program from VU1 "--cont" instruction. */
void vu1_add_continue_program();

/** Add end tag and send packet to VU1. */
void vu1_send();

/** Add end tag and send packet to VU1. */
void vu1_add_flush();

/** NOTICE: can be used only on dynamic list! */
void vu1_dyn_add_128(u64 v1, u64 v2);

/** NOTICE: can be used only on dynamic list! */
void vu1_dyn_add_64(u64 v);

/** NOTICE: can be used only on dynamic list! */
void vu1_dyn_add_32(u32 v);

/** NOTICE: can be used only on dynamic list! */
void vu1_dyn_add_float(float v);

/** Upload micro program to VU1. */
void vu1_upload_program(u32 t_dest, u32 *t_start, u32 *t_end);

/** Used internally for checking data alignment. */
void vu1_check_list();

/** Used internally for checking micro program size. */
u32 vu1_count_program_size(u32 *t_start, u32 *t_end);

/// TAGS

#define DMA_REF_TAG(ADDR, COUNT) ((((unsigned long)ADDR) << 32) | (0x3 << 28) | COUNT)
#define DMA_CNT_TAG(COUNT) (((unsigned long)(0x1) << 28) | COUNT)
#define DMA_END_TAG(COUNT) (((unsigned long)(0x7) << 28) | COUNT)
#define VIF_NOP 0x00
#define VIF_STCYL 0x01
#define VIF_OFFSET 0x02
#define VIF_BASE 0x03
#define VIF_FLUSH 0x11
#define VIF_MSCAL 0x14
#define VIF_MSCNT 0x17
#define VIF_MPG 0x4A
#define V4_32 0xC
#define VIF_UNPACK 0x60
#define U128(n) ((u128)(n))
#define VIF_UNPACK_V4_32 (VIF_UNPACK | V4_32)
#define VIF_EXT_UNPACK(FORMAT, ADDR, NUM, USE_TOPS, NO_SIGN, MASKING)          \
    (s32)((0x60 << 24) + (FORMAT << 24) + (MASKING << 28) + (USE_TOPS << 15) + \
          (NO_SIGN << 14) + (NUM << 16) + ADDR)
#define VIF_CODE(CMD, NUM, IMMEDIATE) ((((unsigned int)(CMD)) << 24) | \
                                       (((unsigned int)(NUM)) << 16) | \
                                       ((unsigned int)(IMMEDIATE)))
#define GS_PRIM(PRIM, IIP, TME, FGE, ABE, AA1, FST, CTXT, FIX) U128((FIX << 10) | (CTXT << 9) | (FST << 8) | (AA1 << 7) | (ABE << 6) | (FGE << 5) | (TME << 4) | (IIP << 3) | (PRIM))
#define GS_GIFTAG(NLOOP, EOP, PRE, PRIM, FLG, NREG) (((u64)(NREG) << 60) | ((u64)(FLG) << 58) | ((u64)(PRIM) << 47) | ((u64)(PRE) << 46) | (EOP << 15) | (NLOOP << 0))
#define VU1_DMA_CHAN_TIMEOUT -1
#define GS_GIFTAG_PACKED 0

#endif
