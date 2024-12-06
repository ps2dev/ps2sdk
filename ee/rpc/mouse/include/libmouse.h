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
 * USB Mouse Driver for PS2
 */

#ifndef __LIBMOUSE_H__
#define __LIBMOUSE_H__

#include <ps2mouse.h>

typedef mouse_data PS2MouseData;

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the RPC library */
extern int PS2MouseInit(void);
/** Reads the current mouse data (depending on read mode) */
extern int PS2MouseRead(PS2MouseData *);
/** Sets the read mode */
extern int PS2MouseSetReadMode(u32 readMode);
/** Gets the current read mode */
extern u32 PS2MouseGetReadMode();

extern int PS2MouseSetThres(u32 thres);
extern u32 PS2MouseGetThres();
extern int PS2MouseSetAccel(float accel);
extern float PS2MouseGetAccel();
extern int PS2MouseSetBoundary(int minx, int maxx, int miny, int maxy);
extern int PS2MouseGetBoundary(int *minx, int *maxx, int *miny, int *maxy);
extern int PS2MouseSetPosition(int x, int y);
extern int PS2MouseReset();
extern u32 PS2MouseEnum();
extern int PS2MouseSetDblClickTime(u32 msec);
extern u32 PS2MouseGetDblClickTIme();
extern u32 PS2MouseGetVersion();

#ifdef __cplusplus
}
#endif

#endif /* __LIBMOUSE_H__ */
