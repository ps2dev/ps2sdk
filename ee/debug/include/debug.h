/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE Debug prototypes
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <tamtypes.h>
#include <stdarg.h>

#define DEBUG_BGCOLOR(col) *((u64 *) 0x120000e0) = (u64) (col)

#ifdef __cplusplus
extern "C" {
#endif

void init_scr(void);
void scr_printf(const char *, ...) __attribute__((format(printf,1,2)));
void scr_vprintf(const char *format, va_list opt);
void scr_putchar(int x, int y, u32 color, int ch);
void ps2GetStackTrace(unsigned int* results,int max);
void scr_setXY(int x, int y);
int scr_getX(void);
int scr_getY(void);
void scr_clear(void);
void scr_clearline(int Y);
void scr_clearchar(int X, int Y);
void scr_setbgcolor(u32 color);
void scr_setfontcolor(u32 color);
void scr_setcursorcolor(u32 color);
void scr_setCursor(int enable);
int scr_getCursor(void);
#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_H__ */
