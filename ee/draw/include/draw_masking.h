/**
 * @file
 * Draw library masking functions
 */

#ifndef __DRAW_MASKING_H__
#define __DRAW_MASKING_H__

#include <tamtypes.h>

/** Scan masking */
#define SCAN_MASK_NORMAL		0
#define SCAN_MASK_ODD			2
#define SCAN_MASK_EVEN			3

/** Color Clamping */
#define COLOR_CLAMP_MASK		0
#define COLOR_CLAMP_ENABLE		1

#ifdef __cplusplus
extern "C" {
#endif

/** Scanline Masking (framebuffer) */
qword_t *draw_scan_masking(qword_t *q, int mask);

/** Color Masking/Clamping */
qword_t *draw_color_clamping(qword_t *q, int enable);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW_MASKING_H__ */
