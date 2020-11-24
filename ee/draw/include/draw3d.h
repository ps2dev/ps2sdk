/**
 * @file
 * Draw library 3D functions
 */

#ifndef __DRAW3D_H__
#define __DRAW3D_H__

#include <tamtypes.h>

#include <draw_primitives.h>
#include <draw_types.h>
#include <gif_tags.h>

/** Register lists */
#define DRAW_XYZ_REGLIST \
	((u64)GIF_REG_XYZ2) << 0 | \
	((u64)GIF_REG_XYZ2) << 4

#define DRAW_RGBAQ_REGLIST \
	((u64)GIF_REG_RGBAQ) <<  0 | \
	((u64)GIF_REG_XYZ2)  <<  4

#define DRAW_UV_REGLIST \
	((u64)GIF_REG_UV)   << 0 | \
	((u64)GIF_REG_XYZ2) << 4

#define DRAW_RGBAQ_UV_REGLIST \
	((u64)GIF_REG_RGBAQ)  << 0 | \
	((u64)GIF_REG_UV)     << 4 | \
	((u64)GIF_REG_XYZ2)   << 8

#define DRAW_STQ_REGLIST \
	((u64)GIF_REG_RGBAQ)  << 0 | \
	((u64)GIF_REG_ST)     << 4 | \
	((u64)GIF_REG_XYZ2)   << 8

/** 
 * Sandro: 
 * Similar to DRAW_STQ_REGLIST, but order of ST and RGBAQ is swapped. 
 * Needed for REGLIST mode which is used mostly in VU1, because of nature of 128bit registers. 
 * Without that, texture perspective correction will not work. 
 * Bad example: 
 * 1. RGBA -> RGBAQ (q was not set!) 
 * 2. STQ  -> ST 
 * 3. XYZ2 
 * Good example: 
 * 1. STQ  -> ST 
 * 2. RGBA -> RGBAQ (q grabbed from STQ) 
 * 3. XYZ2 
 * For more details, please check: 
 * EE_Overview_Manual.pdf - 3.3 Data transfer to GS 
 * GS_Users_Manual.pdf - 3.4.10 Perspective correction
 */
#define DRAW_STQ2_REGLIST \
	((u64)GIF_REG_ST)     << 0 | \
	((u64)GIF_REG_RGBAQ)  << 4 | \
	((u64)GIF_REG_XYZ2)   << 8

#ifdef __cplusplus
extern "C" {
#endif

/** Begins a primitive, allowing for vertex data to be filled in the packet directly */
qword_t *draw_prim_start(qword_t *q, int context, prim_t *prim, color_t *color);

/** Ends a primitive by calculating the number of qwords used, the number of registers, the register list */
qword_t *draw_prim_end(qword_t *q,int nreg, u64 reglist);

/** Converts floating point color, replacing alpha with constant value, and calculates q */
int draw_convert_rgbq(color_t *output, int count, vertex_f_t *vertices, color_f_t *colours, unsigned char alpha);

/** Converts floating point color and calculates q value */
int draw_convert_rgbaq(color_t *output, int count, vertex_f_t *vertices, color_f_t *colours);

/** Calculates the st coordinates from the perspective coordinate q = 1/w */
int draw_convert_st(texel_t *output, int count, vertex_f_t *vertices, texel_f_t *coords);

/** Converts and translates floating point vertices to fixed point vertices */
int draw_convert_xyz(xyz_t *output, float x, float y, int z, int count, vertex_f_t *vertices);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW3D_H__ */
