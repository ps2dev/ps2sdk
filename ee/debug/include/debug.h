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
# EE Debug prototypes
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

void init_scr(void);
void scr_printf(const char *, ...) __attribute__((format(printf,1,2)));
void _putchar(int x, int y, u32 color, u8 ch);
void ps2GetStackTrace(unsigned int* results,int max);
void _setXY(int x, int y);

#define DEBUG_BGCOLOR(col) *((u64 *) 0x120000e0) = (u64) (col)

#ifdef __cplusplus
}
#endif

#endif
