#ifndef __DRAW_BLENDING_H__
#define __DRAW_BLENDING_H__

#include <tamtypes.h>

// color = (c1-c2)*a>>7 + c3

// Alpha Blending
#define BLEND_COLOR_SOURCE		0
#define BLEND_COLOR_DEST		1
#define BLEND_COLOR_ZERO		2

#define BLEND_ALPHA_SOURCE		0
#define BLEND_ALPHA_DEST		1
#define BLEND_ALPHA_FIXED		2

// Alpha Correction
#define ALPHA_CORRECT_RGBA32	0
#define ALPHA_CORRECT_RGBA16	1

typedef struct {
	char color1;
	char color2;
	char alpha;
	char color3;
	unsigned char fixed_alpha;
} BLEND;

#ifdef __cplusplus
extern "C" {
#endif

	// Alpha Blending Per-Pixel MSB Control
	QWORD *draw_pixel_alpha_control(QWORD *q, int enable);

	// Alpha Blending
	QWORD *draw_alpha_blending(QWORD *q, int context, BLEND *blend);

	// Alpha Correction
	QWORD *draw_alpha_correction(QWORD *q, int context, int alpha);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_BLENDING_H__*/
