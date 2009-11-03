/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: iopdebug.h $
#
# IOPDEBUG - IOP debugging library.
#
# iopdebug.h - Definitions for the IOPDEBUG library.
#
*/

#ifndef _IOP_DEBUG_H
#define _IOP_DEBUG_H

#include <tamtypes.h>
#include <iop_cop0_defs.h>
#include <ps2_debug.h>

#ifdef __cplusplus
extern "C" {
#endif

#define iopdebug_IMPORTS_start DECLARE_IMPORT_TABLE(iopdebug, 1, 1)
#define iopdebug_IMPORTS_end END_IMPORT_TABLE

typedef int (IOP_ExceptionHandler)(struct st_IOP_RegFrame *);

int iop_dbg_install(void);
#define I_iop_dbg_install DECLARE_IMPORT(5, iop_dbg_install)

int iop_dbg_remove(void);
#define I_iop_dbg_remove DECLARE_IMPORT(6, iop_dbg_remove)

IOP_ExceptionHandler *iop_dbg_get_handler(int cause);
#define I_iop_dbg_get_handler DECLARE_IMPORT(7, iop_dbg_get_handler)

IOP_ExceptionHandler *iop_dbg_set_handler(int cause, IOP_ExceptionHandler *handler);
#define I_iop_dbg_set_handler DECLARE_IMPORT(8, iop_dbg_set_handler)

void iop_dbg_get_reg_frames(IOP_RegFrame **def_frame_ptr, IOP_RegFrame **dbg_frame_ptr);
#define I_iop_dbg_get_reg_frames DECLARE_IMPORT(9, iop_dbg_get_reg_frames)

u32 iop_dbg_get_dcic(void);
#define I_iop_dbg_get_dcic DECLARE_IMPORT(10, iop_dbg_get_dcic)

void iop_dbg_set_dcic(u32 v);
#define I_iop_dbg_set_dcic DECLARE_IMPORT(11, iop_dbg_set_dcic)



u32 iop_dbg_get_bpc(void);
#define I_iop_dbg_get_bpc DECLARE_IMPORT(12, iop_dbg_get_bpc)

void iop_dbg_set_bpc(u32 v);
#define I_iop_dbg_set_bpc DECLARE_IMPORT(13, iop_dbg_set_bpc)



u32 iop_dbg_get_bpcm(void);
#define I_iop_dbg_get_bpcm DECLARE_IMPORT(14, iop_dbg_get_bpcm)

void iop_dbg_set_bpcm(u32 v);
#define I_iop_dbg_set_bpcm DECLARE_IMPORT(15, iop_dbg_set_bpcm)



u32 iop_dbg_get_bda(void);
#define I_iop_dbg_bda DECLARE_IMPORT(16, iop_dbg_bda)

void iop_dbg_set_bda(u32 v);
#define I_iop_dbg_set_bda DECLARE_IMPORT(17, iop_dbg_set_bda)

u32 iop_dbg_get_bdam(void);
#define I_iop_dbg_bdam DECLARE_IMPORT(18, iop_dbg_bdam)

void iop_dbg_set_bdam(u32 v);
#define I_iop_dbg_set_bdam DECLARE_IMPORT(19, iop_dbg_bdam)


void iop_dbg_set_bpr(u32 addr, u32 mask, u32 user_mask);
#define I_iop_dbg_set_bpr DECLARE_IMPORT(20, iop_dbg_set_bpr)

void iop_dbg_set_bpw(u32 addr, u32 mask, u32 user_mask);
#define I_iop_dbg_set_bpw DECLARE_IMPORT(21, iop_dbg_set_bpw)

void iop_dbg_set_bpx(u32 addr, u32 mask, u32 user_mask);
#define I_iop_dbg_set_bpx DECLARE_IMPORT(22, iop_dbg_bpx)


void iop_dbg_clr_bps(void);
#define I_iop_dbg_clr_bps DECLARE_IMPORT(23, iop_dbg_clr_bps)

void iop_dbg_clr_bpda(void);
#define I_iop_dbg_clr_bpda DECLARE_IMPORT(24, iop_dbg_clr_bpda)

void iop_dbg_clr_bpx(void);
#define I_iop_dbg_clr_bpx DECLARE_IMPORT(25, iop_dbg_clr_bpx)

#ifdef __cplusplus
}
#endif

#endif
