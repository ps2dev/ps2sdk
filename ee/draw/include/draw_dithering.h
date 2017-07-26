/**
 * @file
 * Draw library dithering functions
 */

#ifndef __DRAW_DITHERING_H__
#define __DRAW_DITHERING_H__

#include <tamtypes.h>

typedef signed char dithermx_t[16];

#ifdef __cplusplus
extern "C" {
#endif

/** Dithering Switch */
qword_t *draw_dithering(qword_t *q, int enable);

/** Dithering Matrix */
qword_t *draw_dither_matrix(qword_t *q,char *dm);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_DITHERING_H__*/
