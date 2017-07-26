/**
 * @file
 * Draw library main functions
 */

#ifndef __DRAW_H__
#define __DRAW_H__

#include <tamtypes.h>

#include <draw_blending.h>
#include <draw_buffers.h>
#include <draw_dithering.h>
#include <draw_fog.h>
#include <draw_masking.h>
#include <draw_primitives.h>
#include <draw_sampling.h>
#include <draw_tests.h>
#include <draw_types.h>

#include <draw2d.h>
#include <draw3d.h>

#define DRAW_DISABLE 0
#define DRAW_ENABLE  1

#ifdef __cplusplus
extern "C" {
#endif

/** Sets up the drawing environment based on the framebuffer and zbuffer settings */
qword_t *draw_setup_environment(qword_t *q, int context, framebuffer_t *frame, zbuffer_t *z);

/** Clear the screen based on the screen's origin, width, and height using the defined color. **/
qword_t *draw_clear(qword_t *q, int context, float x, float y, float width, float height, int r, int g, int b);

/** Signal that drawing is finished */
qword_t *draw_finish(qword_t *q);

/** Wait until finish event occurs */
void draw_wait_finish(void);

/** Creates a dma chain filled with image information */
qword_t *draw_texture_transfer(qword_t *q, void *src, int width, int height, int psm, int dest, int dest_width);

/** Flush the texture cache */
qword_t *draw_texture_flush(qword_t *q);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW_H__ */
