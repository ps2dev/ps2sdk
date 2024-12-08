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
 * EEDEBUG - EE debugging library.
 * Definitions for the EEDEBUG library.
 */

#ifndef __EE_DEBUG_H__
#define __EE_DEBUG_H__

#include <tamtypes.h>
#include <ee_cop0_defs.h>
#include <ps2_debug.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (EE_ExceptionHandler)(struct st_EE_RegFrame *);

extern int ee_dbg_install(int levels);
extern int ee_dbg_remove(int levels);

extern EE_ExceptionHandler *ee_dbg_get_level1_handler(int cause);
extern EE_ExceptionHandler *ee_dbg_set_level1_handler(int cause, EE_ExceptionHandler *handler);

extern EE_ExceptionHandler *ee_dbg_get_level2_handler(int cause);
extern EE_ExceptionHandler *ee_dbg_set_level2_handler(int cause, EE_ExceptionHandler *handler);

extern u32 ee_dbg_get_bpc(void);
extern void ee_dbg_set_bpc(u32);

extern u32 ee_dbg_get_iab(void);
extern void ee_dbg_set_iab(u32);

extern u32 ee_dbg_get_iabm(void);
extern void ee_dbg_set_iabm(u32);

extern u32 ee_dbg_get_dab(void);
extern void ee_dbg_set_dab(u32);

extern u32 ee_dbg_get_dabm(void);
extern void ee_dbg_set_dabm(u32);

extern u32 ee_dbg_get_dvb(void);
extern void ee_dbg_set_dvb(u32);

extern u32 ee_dbg_get_dvbm(void);
extern void ee_dbg_set_dvbm(u32);

extern void ee_dbg_set_bpr(u32 addr, u32 mask, u32 opmode_mask);
extern void ee_dbg_set_bpw(u32 addr, u32 mask, u32 opmode_mask);
extern void ee_dbg_set_bpv(u32 value, u32 mask, u32 opmode_mask);
extern void ee_dbg_set_bpx(u32 addr, u32 mask, u32 opmode_mask);

extern void ee_dbg_clr_bps(void);
extern void ee_dbg_clr_bpda(void);
extern void ee_dbg_clr_bpdv(void);
extern void ee_dbg_clr_bpx(void);

#ifdef __cplusplus
}
#endif

#endif /* __EE_DEBUG_H__ */
