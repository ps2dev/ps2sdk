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

#define GS_PRIM(PRIM, IIP, TME, FGE, ABE, AA1, FST, CTXT, FIX) (u128)(((FIX << 10) | (CTXT << 9) | (FST << 8) | (AA1 << 7) | (ABE << 6) | (FGE << 5) | (TME << 4) | (IIP << 3) | (PRIM)))
#define GS_GIFTAG(NLOOP, EOP, PRE, PRIM, FLG, NREG) (((u64)(NREG) << 60) | ((u64)(FLG) << 58) | ((u64)(PRIM) << 47) | ((u64)(PRE) << 46) | (EOP << 15) | (NLOOP << 0))

// ----
// Main
// ----

/** 
 * Send data to VU1 mem. 
 * Not using TOP register (double buffer). 
 * Please be sure that data is aligned. 
 * @param t_dest_address At what VU1 mem qword data should be unpacked? 
 * @param t_data Pointer to data. 
 * @param t_quad_size Size of data in quadwords. 
 */
void vu1_send_data(u32 t_dest_address, void *t_data, u32 t_quad_size);

/** 
 * Send VU1 buffers size to VU1. 
 * @param t_base Base address (qw)
 * @param t_offset Offset address (qw)
 */
void vu1_set_double_buffer(u16 t_base, u16 t_offset);

/** 
 * Add data via pointer. 
 * Please be sure that data is aligned. 
 * @param t_offset Offset before data in qwords. 
 * @param t_data Data pointer. 
 * @param t_size Size in quadwords. 
 * @param t_use_top When true, data will be loaded at the beginning of current VU1 buffer. 
 */
void vu1_add_data(packet2_t *packet2, void *t_data, u32 t_size, u8 t_use_top);

/** Start VU1 micro program. */
void vu1_add_start_program(packet2_t *packet2);

/** Add end tag and send packet to VU1. */
void vu1_send_packet(packet2_t *packet2);

// ----
// UNPACK
// ----

/** Add flush and unpack. */
void vu1_open_unpack(packet2_t *packet2);

/** Close VU unpack. */
void vu1_close_unpack(packet2_t *packet2);

/** NOTICE: can be used only with open unpack! */
void vu1_unpack_add_128(packet2_t *packet2, u64 v1, u64 v2);

/** NOTICE: can be used only with open unpack! */
void vu1_unpack_add_64(packet2_t *packet2, u64 v);

/** NOTICE: can be used only with open unpack! */
void vu1_unpack_add_32(packet2_t *packet2, u32 v);

/** NOTICE: can be used only with open unpack! */
void vu1_unpack_add_float(packet2_t *packet2, float v);

// ----
// Program code
// ----

/** Upload micro program to VU1. */
void vu1_upload_program(u32 t_dest, u32 *t_start, u32 *t_end);

/** Used internally for checking micro program size. */
u32 vu1_count_program_size(u32 *t_start, u32 *t_end);

#endif
