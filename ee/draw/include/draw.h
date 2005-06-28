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

 #include <graph.h>
 #include <math3d.h>

#ifdef __cplusplus
extern "C" {
#endif

 ////////////////////
 // DRAW FUNCTIONS //
 ////////////////////

 int draw_initialize(int mode, int bpp, int zbpp);
 // Initialize the draw library and set the specified mode.

 int draw_swap(void);
 // Swap the current display and draw buffers.

 int draw_clear(float red, float green, float blue);
 // Clear the current draw buffer with the specified colours.
 // Valid range for red, green and blue is: 0.00f - 1.00f

 /////////////////////////////
 // DRAW GENERATE FUNCTIONS //
 /////////////////////////////

 int draw_generate_xyz(u64 *output, int count, VECTOR *vertices);
 // Generate XYZ register values for an array of vertices.
 // Valid range for vertex values are: -1.00f - 1.00f

 int draw_generate_rgbaq(u64 *output, int count, VECTOR *vertices, VECTOR *colours);
 // Generate RGBAQ register values from an array of vertices and colours.
 // Valid range for vertex values are: -1.00f - 1.00f
 // Valid range for colour values are:  0.00f - 1.00f

 int draw_generate_st(u64 *output, int count, VECTOR *vertices, VECTOR *coordinates);
 // Generate ST register values from an array of vertices and texture coordinates.
 // Valid range for texture coordinates are: 0.00f - 1.00f

 //////////////////////////////
 // DRAW PRIMITIVE FUNCTIONS //
 //////////////////////////////

 int draw_triangles(int *points, int count, u64 *xyz, u64 *rgbaq);
 // Draw untextured triangles from an array of points and register values.

 int draw_triangles_textured(int *points, int count, u64 *xyz, u64 *rgbaq, u64 *st);
 // Draw textured triangles from an array of points and register values.

#ifdef __cplusplus
}
#endif

#endif
