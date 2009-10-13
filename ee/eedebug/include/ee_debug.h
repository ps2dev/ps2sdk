/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: ee_debug.h $
#
# EEDEBUG - EE debugging library.
#
# ee_debug.h - Definitions for the EEDEBUG library.
#
*/

#ifndef _EE_DEBUG_H
#define _EE_DEBUG_H

#include <tamtypes.h>
#include <ee_cop0_defs.h>
#include <ps2_debug.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (EE_ExceptionHandler)(struct st_EE_RegFrame *);

int ee_dbg_install(int levels);
int ee_dbg_remove(int levels);

EE_ExceptionHandler *ee_dbg_get_level1_handler(int cause);
EE_ExceptionHandler *ee_dbg_set_level1_handler(int cause, EE_ExceptionHandler *handler);

EE_ExceptionHandler *ee_dbg_get_level2_handler(int cause);
EE_ExceptionHandler *ee_dbg_set_level2_handler(int cause, EE_ExceptionHandler *handler);

u32 ee_dbg_get_bpc(void);
void ee_dbg_set_bpc(u32);

u32 ee_dbg_get_iab(void);
void ee_dbg_set_iab(u32);

u32 ee_dbg_get_iabm(void);
void ee_dbg_set_iabm(u32);

u32 ee_dbg_get_dab(void);
void ee_dbg_set_dab(u32);

u32 ee_dbg_get_dabm(void);
void ee_dbg_set_dabm(u32);

u32 ee_dbg_get_dvb(void);
void ee_dbg_set_dvb(u32);

u32 ee_dbg_get_dvbm(void);
void ee_dbg_set_dvbm(u32);

void ee_dbg_set_bpr(u32 addr, u32 mask, u32 opmode_mask);
void ee_dbg_set_bpw(u32 addr, u32 mask, u32 opmode_mask);
void ee_dbg_set_bpv(u32 value, u32 mask, u32 opmode_mask);
void ee_dbg_set_bpx(u32 addr, u32 mask, u32 opmode_mask);

void ee_dbg_clr_bps(void);
void ee_dbg_clr_bpda(void);
void ee_dbg_clr_bpdv(void);
void ee_dbg_clr_bpx(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _EE_DEBUG_H

