#ifndef __DRAW3D_H__
#define __DRAW3D_H__

#include <tamtypes.h>

#include <draw_primitives.h>
#include <draw_types.h>
#include <gif_tags.h>

// Register lists
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

#ifdef __cplusplus
extern "C" {
#endif

	// Begins a primitive, allowing for vertex data to be filled in the packet directly
	qword_t *draw_prim_start(qword_t *q, int context, prim_t *prim, color_t *color);

	// Ends a primitive by calculating the number of qwords used, the number of registers,
	// the register list
	qword_t *draw_prim_end(qword_t *q,int nreg, unsigned long reglist);

	// Converts floating point color, replacing alpha with constant value, and calculates q
	int draw_convert_rgbq(color_t *output, int count, vertex_f_t *vertices, color_f_t *colours, unsigned char alpha);

	// Converts floating point color and calculates q value
	int draw_convert_rgbaq(color_t *output, int count, vertex_f_t *vertices, color_f_t *colours);

	// Calculates the st coordinates from the perspective coordinate q = 1/w
	int draw_convert_st(texel_t *output, int count, vertex_f_t *vertices, texel_f_t *coords);

	// Converts and translates floating point vertices to fixed point vertices
	int draw_convert_xyz(xyz_t *output, float x, float y, int z, int count, vertex_f_t *vertices);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW3D_H__*/
