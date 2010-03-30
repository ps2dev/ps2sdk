#ifndef __DRAW_PRIMITIVES_H__
#define __DRAW_PRIMITIVES_H__

#include <tamtypes.h>

// Types
#define PRIM_POINT				0x00
#define PRIM_LINE				0x01
#define PRIM_LINE_STRIP			0x02
#define PRIM_TRIANGLE			0x03
#define PRIM_TRIANGLE_STRIP		0x04
#define PRIM_TRIANGLE_FAN		0x05
#define PRIM_SPRITE				0x06

// Shading
#define PRIM_SHADE_FLAT			0
#define PRIM_SHADE_GOURAUD		1

// Texture Mapping Coordinates
#define PRIM_MAP_ST				0
#define PRIM_MAP_UV				1

// Fixed Color Value
#define PRIM_UNFIXED			0
#define PRIM_FIXED				1

// Primitive Override Control
#define PRIM_OVERRIDE_ENABLE	0
#define PRIM_OVERRIDE_DISABLE	1

typedef struct {
	unsigned char type;
	unsigned char shading;
	unsigned char mapping;
	unsigned char fogging;
	unsigned char blending;
	unsigned char antialiasing;
	unsigned char mapping_type;
	unsigned char colorfix;
} PRIMITIVE;

#ifdef __cplusplus
extern "C" {
#endif

	// Primitive Coordinate System offset
	QWORD *draw_primitive_xyoffset(QWORD *q, int context, float x, float y);

	// Primitive Control
	QWORD *draw_primitive_override(QWORD *q, int mode);

	// Overridden Primitive Attributes
	QWORD *draw_primitive_override_setting(QWORD *q, int context, PRIMITIVE *prim);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_PRIMITIVES_H__*/
