/*      
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2001, Gustavo Scotti (gustavo@scotti.com)
  ------------------------------------------------------------------------
  debug.h
			EE Debug prototypes
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

void init_scr(void);
void scr_printf(const char *, ...);
void _putchar(int x, int y, u32 color, u8 ch);

#define DEBUG_BGCOLOR(col) *((u64 *) 0x120000e0) = (u64) (col)

#ifdef __cplusplus
}
#endif

#endif
