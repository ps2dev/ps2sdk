#ifndef __DRAW2D_H__
#define __DRAW2D_H__

#include <tamtypes.h>

#include <draw_types.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Draws a single point
	QWORD *draw_point(QWORD *q, int context, VERTEX *v0, COLOR *color);

	// Draws a single line
	QWORD *draw_line(QWORD *q, int context, VERTEX *v0, VERTEX *v1, COLOR *color);

	// Draws a triangle using a line strip
	QWORD *draw_triangle_outline(QWORD *q, int context, VERTEX *v0, VERTEX *v1, VERTEX *v2, COLOR *color);

	// Draws a single triangle
	QWORD *draw_triangle_filled(QWORD *q, int context,VERTEX *v0, VERTEX *v1, VERTEX *v2, COLOR *color);

	// Draws a rectangle using line primitives
	QWORD *draw_rect_outline(QWORD *q, int context, VERTEX *v0, VERTEX *v1, COLOR *color);

	// Draws a single sprite
	QWORD *draw_rect_filled(QWORD *q, int context, VERTEX *v0, VERTEX *v1, COLOR *color);

	// Draws a single texture mapped sprite
	QWORD *draw_rect_textured(QWORD *q, int context, VERTEX *v0, TEXEL *t0, VERTEX *v1, TEXEL *t1, COLOR *color);

	// Draws multiple sprite primitives
	QWORD *draw_rect_filled_strips(QWORD *q, int context, VERTEX *v0, VERTEX *v1, COLOR *color);

	// Draws multiple strips to render a large texture
	// width must be multiple of 32 - 0.9375
	QWORD *draw_rect_textured_strips(QWORD *q, int context, VERTEX *v0, TEXEL *t0, VERTEX *v1, TEXEL *t1, COLOR *color);

	// Draws an arc using line primitives
	QWORD *draw_arc_outline(QWORD *q, int context, VERTEX *center, float radius, float angle_start, float angle_end, COLOR *color);

	// Draws multiple triangle fans
	QWORD *draw_arc_filled(QWORD *q, int context, VERTEX *center, float radius, float angle_start, float angle_end, COLOR *color);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW2D_H__*/
