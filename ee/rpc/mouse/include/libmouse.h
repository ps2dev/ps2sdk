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
# USB Mouse Driver for PS2
*/

#ifndef __LIBMOUSE_H__
#define __LIBMOUSE_H__

#include "ps2mouse.h"

typedef mouse_data PS2MouseData;

#ifdef __cplusplus
extern "C" {
#endif

/* Initialise the RPC library */
int PS2MouseInit(void);
/* Reads the current mouse data (depending on read mode) */
int PS2MouseRead(PS2MouseData *);
/* Sets the read mode */
int PS2MouseSetReadMode(u32 readMode);
/* Gets the current read mode */
u32 PS2MouseGetReadMode();

int PS2MouseSetThres(u32 thres);
u32 PS2MouseGetThres();
int PS2MouseSetAccel(float accel);
float PS2MouseGetAccel();
int PS2MouseSetBoundary(int minx, int maxx, int miny, int maxy);
int PS2MouseGetBoundary(int *minx, int *maxx, int *miny, int *maxy);
int PS2MouseSetPosition(int x, int y);
int PS2MouseReset();
u32 PS2MouseEnum();
int PS2MouseSetDblClickTime(u32 msec);
u32 PS2MouseGetDblClickTIme();
u32 PS2MouseGetVersion();

#ifdef __cplusplus
}
#endif

#endif
