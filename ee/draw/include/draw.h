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

#define DRAW_DISABLE	0
#define DRAW_ENABLE		1

#ifdef __cplusplus
extern "C" {
#endif

	// Sets up the drawing environment based on the framebuffer and zbuffer settings
	QWORD *draw_setup_environment(QWORD *q, int context, FRAMEBUFFER *frame, ZBUFFER *z);

	// Clear the screen based on the screen's origin, width, and height using the defined color.
	QWORD *draw_clear(QWORD *q, int context, float x, float y, float width, float height, int r, int g, int b);

	// Signal that drawing is finished
	QWORD *draw_finish(QWORD *q);

	// Wait until finish event occurs
	void draw_wait_finish(void);

	// Creates a dma chain filled with image information
	QWORD *draw_texture_transfer(QWORD *q, void *src, int bytes, int width, int height, int psm, int buffer_width, int dest);

	// Flush the texture cache
	QWORD *draw_texture_flush(QWORD *q);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_H__*/
