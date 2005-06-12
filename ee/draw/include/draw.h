/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Dan Peori <peori@oopo.net>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#ifndef __DRAW_H__
#define __DRAW_H__

 #include <tamtypes.h>

 #include <math3d.h>

#ifdef __cplusplus
extern "C" {
#endif

 ////////////////////
 // DRAW FUNCTIONS //
 ////////////////////

 int draw_initialize(int mode, int bpp, int zbpp);

 int draw_swap(void);

 int draw_clear(float red, float green, float blue);

 /////////////////////////////
 // DRAW GENERATE FUNCTIONS //
 /////////////////////////////

 int draw_generate_xyz(u64 *output, int count, VECTOR *vertices);

 int draw_generate_rgbaq(u64 *output, int count, VECTOR *vertices, VECTOR *colours);

 int draw_generate_st(u64 *output, int count, VECTOR *coords);

 //////////////////////////////
 // DRAW PRIMITIVE FUNCTIONS //
 //////////////////////////////

 int draw_triangles(int *points, int count, u64 *xyz, u64 *rgbaq);

#ifdef __cplusplus
}
#endif

#endif
