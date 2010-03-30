#ifndef __DRAW_DITHER_H__
#define __DRAW_DITHER_H__

#include <tamtypes.h>

typedef signed char DITHERMATRIX[16];

#ifdef __cplusplus
extern "C" {
#endif

	// Dithering Switch
	QWORD *draw_dithering(QWORD *q, int enable);

	// Dithering Matrix
	QWORD *draw_dither_matrix(QWORD *q,DITHERMATRIX dm);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_DITHER_H__*/
