/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Timer prototypes
*/

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

// EE Timers
#define T0_COUNT   ((volatile unsigned long*)0x10000000) 
#define T0_MODE      ((volatile unsigned long*)0x10000010) 
#define T0_COMP      ((volatile unsigned long*)0x10000020) 
#define T0_HOLD      ((volatile unsigned long*)0x10000030) 

#define T1_COUNT   ((volatile unsigned long*)0x10000800) 
#define T1_MODE      ((volatile unsigned long*)0x10000810) 
#define T1_COMP      ((volatile unsigned long*)0x10000820) 
#define T1_HOLD      ((volatile unsigned long*)0x10000830) 

// Note! T2 and T3 don't have a Tn_HOLD register! 
// ---------------------------------------------- 
#define T2_COUNT   ((volatile unsigned long*)0x10001000) 
#define T2_MODE      ((volatile unsigned long*)0x10001010) 
#define T2_COMP      ((volatile unsigned long*)0x10001020) 

#define T3_COUNT   ((volatile unsigned long*)0x10001800) 
#define T3_MODE      ((volatile unsigned long*)0x10001810) 
#define T3_COMP      ((volatile unsigned long*)0x10001820) 

#define Tn_MODE(CLKS,GATE,GATS,GATM,ZRET,CUE,CMPE,OVFE,EQUF,OVFF) \
    (u32)((u32)(CLKS) | ((u32)(GATE) << 2) | \
   ((u32)(GATS) << 3) | ((u32)(GATM) << 4) | \
   ((u32)(ZRET) << 6) | ((u32)(CUE) << 7) | \
   ((u32)(CMPE) << 8) | ((u32)(OVFE) << 9) | \
   ((u32)(EQUF) << 10) | ((u32)(OVFF) << 11))

// Available rates
#define kBUSCLK             (147456000) 
#define kBUSCLKBY16         (kBUSCLK / 16) 
#define kBUSCLKBY256        (kBUSCLK / 256) 
#define kHBLNK_NTSC         (15734) 
#define kHBLNK_PAL          (15625) 
#define kHBLNK_DTV480p      (31469) 
#define kHBLNK_DTV1080i     (33750) 

u32 cpu_ticks(void);

#ifdef __cplusplus
}
#endif

#endif
