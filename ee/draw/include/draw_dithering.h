#ifndef __DRAW_DITHER_H__
#define __DRAW_DITHER_H__

#include <tamtypes.h>

typedef signed char dithermx_t[16];

#ifdef __cplusplus
extern "C" {
#endif

	// Dithering Switch
	qword_t *draw_dithering(qword_t *q, int enable);

	// Dithering Matrix
	qword_t *draw_dither_matrix(qword_t *q,char *dm);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_DITHER_H__*/
