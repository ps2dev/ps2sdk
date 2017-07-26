/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOPDEBUG - IOP debugging library.
 * Definitions for the IOPDEBUG library.
 */

#ifndef __IOPDEBUG_H__
#define __IOPDEBUG_H__

#include <types.h>
#include <irx.h>
#include <iop_cop0_defs.h>
#include <ps2_debug.h>

typedef int (IOP_ExceptionHandler)(struct st_IOP_RegFrame *);

int iop_dbg_install(void);
int iop_dbg_remove(void);
IOP_ExceptionHandler *iop_dbg_get_handler(int cause);
IOP_ExceptionHandler *iop_dbg_set_handler(int cause, IOP_ExceptionHandler *handler);
void iop_dbg_get_reg_frames(IOP_RegFrame **def_frame_ptr, IOP_RegFrame **dbg_frame_ptr);
u32 iop_dbg_get_dcic(void);
void iop_dbg_set_dcic(u32 v);

u32 iop_dbg_get_bpc(void);
void iop_dbg_set_bpc(u32 v);

u32 iop_dbg_get_bpcm(void);
void iop_dbg_set_bpcm(u32 v);

u32 iop_dbg_get_bda(void);
void iop_dbg_set_bda(u32 v);
u32 iop_dbg_get_bdam(void);
void iop_dbg_set_bdam(u32 v);

void iop_dbg_set_bpr(u32 addr, u32 mask, u32 user_mask);
void iop_dbg_set_bpw(u32 addr, u32 mask, u32 user_mask);
void iop_dbg_set_bpx(u32 addr, u32 mask, u32 user_mask);

void iop_dbg_clr_bps(void);
void iop_dbg_clr_bpda(void);
void iop_dbg_clr_bpx(void);

#define iopdebug_IMPORTS_start DECLARE_IMPORT_TABLE(iopdebug, 1, 1)
#define iopdebug_IMPORTS_end END_IMPORT_TABLE

#define I_iop_dbg_install DECLARE_IMPORT(5, iop_dbg_install)
#define I_iop_dbg_remove DECLARE_IMPORT(6, iop_dbg_remove)
#define I_iop_dbg_get_handler DECLARE_IMPORT(7, iop_dbg_get_handler)
#define I_iop_dbg_set_handler DECLARE_IMPORT(8, iop_dbg_set_handler)
#define I_iop_dbg_get_reg_frames DECLARE_IMPORT(9, iop_dbg_get_reg_frames)
#define I_iop_dbg_get_dcic DECLARE_IMPORT(10, iop_dbg_get_dcic)
#define I_iop_dbg_set_dcic DECLARE_IMPORT(11, iop_dbg_set_dcic)
#define I_iop_dbg_get_bpc DECLARE_IMPORT(12, iop_dbg_get_bpc)
#define I_iop_dbg_set_bpc DECLARE_IMPORT(13, iop_dbg_set_bpc)
#define I_iop_dbg_get_bpcm DECLARE_IMPORT(14, iop_dbg_get_bpcm)
#define I_iop_dbg_set_bpcm DECLARE_IMPORT(15, iop_dbg_set_bpcm)
#define I_iop_dbg_bda DECLARE_IMPORT(16, iop_dbg_bda)
#define I_iop_dbg_set_bda DECLARE_IMPORT(17, iop_dbg_set_bda)
#define I_iop_dbg_bdam DECLARE_IMPORT(18, iop_dbg_bdam)
#define I_iop_dbg_set_bdam DECLARE_IMPORT(19, iop_dbg_bdam)
#define I_iop_dbg_set_bpr DECLARE_IMPORT(20, iop_dbg_set_bpr)
#define I_iop_dbg_set_bpw DECLARE_IMPORT(21, iop_dbg_set_bpw)
#define I_iop_dbg_set_bpx DECLARE_IMPORT(22, iop_dbg_bpx)
#define I_iop_dbg_clr_bps DECLARE_IMPORT(23, iop_dbg_clr_bps)
#define I_iop_dbg_clr_bpda DECLARE_IMPORT(24, iop_dbg_clr_bpda)
#define I_iop_dbg_clr_bpx DECLARE_IMPORT(25, iop_dbg_clr_bpx)

#endif /* __IOPDEBUG_H__ */
