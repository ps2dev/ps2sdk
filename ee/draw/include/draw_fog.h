/**
 * @file
 * Draw library fog functions
 */

#ifndef __DRAW_FOG_H__
#define __DRAW_FOG_H__

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Fog Color */
qword_t *draw_fog_color(qword_t *q, unsigned char r, unsigned char g, unsigned char b);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW_FOG_H__ */
