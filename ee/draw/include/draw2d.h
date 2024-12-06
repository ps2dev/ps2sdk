/**
 * @file
 * Draw library 2D functions
 */

#ifndef __DRAW2D_H__
#define __DRAW2D_H__

#include <tamtypes.h>

#include <draw_types.h>

typedef struct {
	vertex_t v0;
	color_t  color;
} point_t;

typedef struct {
	vertex_t v0;
	vertex_t v1;
	color_t  color;
} line_t;

typedef struct {
	vertex_t v0;
	vertex_t v1;
	vertex_t v2;
	color_t  color;
} triangle_t;

typedef struct {
	vertex_t v0;
	vertex_t v1;
	color_t  color;
} rect_t;

typedef struct {
	vertex_t v0;
	texel_t  t0;
	vertex_t v1;
	texel_t  t1;
	color_t  color;
} texrect_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Enables alpha blending */
extern void draw_enable_blending();

/** Disables alpha blending */
extern void draw_disable_blending();

/** Draws a single point */
extern qword_t *draw_point(qword_t *q, int context, point_t *point);

/** Draws a single line */
extern qword_t *draw_line(qword_t *q, int context, line_t *line);

/** Draws a triangle using a line strip */
extern qword_t *draw_triangle_outline(qword_t *q, int context, triangle_t *triangle);

/** Draws a single triangle */
extern qword_t *draw_triangle_filled(qword_t *q, int context,triangle_t *triangle);

/** Draws a rectangle using line primitives */
extern qword_t *draw_rect_outline(qword_t *q, int context, rect_t *rect);

/** Draws a single sprite */
extern qword_t *draw_rect_filled(qword_t *q, int context, rect_t *rect);

/** Draws a single texture mapped sprite */
extern qword_t *draw_rect_textured(qword_t *q, int context, texrect_t *rect);

/** Draws multiple sprite primitives */
extern qword_t *draw_rect_filled_strips(qword_t *q, int context, rect_t *rect);

/** Draws multiple strips to render a large texture
 * width must be multiple of 32 - 0.9375
 */
extern qword_t *draw_rect_textured_strips(qword_t *q, int context, texrect_t *rect);

/** Draws filled round rectangle, buggy with partial coordinates */
extern qword_t *draw_round_rect_filled(qword_t *q, int context, rect_t *rect);

/** Draws a hollow round rectangle, buggy with partial coordinates */
extern qword_t *draw_round_rect_outline(qword_t *q, int context, rect_t *rect);

/** Draws an arc using line primitives */
extern qword_t *draw_arc_outline(qword_t *q, int context, point_t *center, float radius, float angle_start, float angle_end);

/** Draws multiple triangle fans */
extern qword_t *draw_arc_filled(qword_t *q, int context, point_t *center, float radius, float angle_start, float angle_end);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW2D_H__ */
