#ifndef __DRAW_MASKING_H__
#define __DRAW_MASKING_H__

#include <tamtypes.h>

// Scan masking
#define SCAN_MASK_NORMAL		0
#define SCAN_MASK_ODD			2
#define SCAN_MASK_EVEN			3

// Color Clamping
#define COLOR_CLAMP_MASK		0
#define COLOR_CLAMP_ENABLE		1

#ifdef __cplusplus
extern "C" {
#endif

	// Scanline Masking (framebuffer)
	QWORD *draw_scan_masking(QWORD *q, int mask);

	// Color Masking/Clamping
	QWORD *draw_color_clamping(QWORD *q, int enable);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_MASKING_H__*/
