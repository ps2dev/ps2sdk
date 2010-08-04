#ifndef __DRAW3D_H__
#define __DRAW3D_H__

#include <tamtypes.h>

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
	QWORD *draw_prim_start(QWORD *q, int context, PRIMITIVE *prim, COLOR *color);

	// Ends a primitive by calculating the number of qwords used, the number of registers,
	// the register list
	QWORD *draw_prim_end(QWORD *q,int nreg, unsigned long reglist);

	// Converts floating point color, replacing alpha with constant value, and calculates q
	int draw_convert_rgbq(COLOR *output, int count, VERTEXF *vertices, COLORF *colours, unsigned char alpha);

	// Converts floating point color and calculates q value
	int draw_convert_rgbaq(COLOR *output, int count, VERTEXF *vertices, COLORF *colours);

	// Calculates the st coordinates from the perspective coordinate q = 1/w
	int draw_convert_st(TEXEL *output, int count, VERTEXF *vertices, TEXELF *coords);

	// Converts and translates floating point vertices to fixed point vertices
	int draw_convert_xyz(XYZ *output, float x, float y, int z, int count, VERTEXF *vertices);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW3D_H__*/
