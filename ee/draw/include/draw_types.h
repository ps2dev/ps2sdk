/**
 * @file
 * Draw library types
 */

#ifndef __DRAW_TYPES_H__
#define __DRAW_TYPES_H__

#include <math3d.h>

#ifdef ftoi4
 #undef ftoi4
 #define ftoi4(F) ((int)(((float)F)*16.0f))
#else
 #define ftoi4(F) ((int)(((float)F)*16.0f))
#endif

typedef union {
	u64 xyz;
	struct {
		u16 x;
		u16 y;
		u32 z;
	};
} __attribute__((packed,aligned(8))) xyz_t;

typedef union {
	u64 uv;
	struct {
		float s;
		float t;
	};
	struct {
		float u;
		float v;
	};
} __attribute__((packed,aligned(8))) texel_t;

typedef union {
	u64 rgbaq;
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
		float q;
	};
} __attribute__((packed,aligned(8))) color_t;

// Using shorts complicates things for normal usage
typedef struct {
	float x;
	float y;
	unsigned int z;
} vertex_t;

typedef union {
	VECTOR strq;
	struct {
		float s;
		float t;
		float r;
		float q;
	};
} __attribute__((packed,aligned(16))) texel_f_t;

typedef union {
	VECTOR rgba;
	struct {
		float r;
		float g;
		float b;
		float a;
	};
} __attribute__((packed,aligned(16))) color_f_t;

typedef union {
	VECTOR xyzw;
	struct {
		float x;
		float y;
		float z;
		float w;
	};
} __attribute__((packed,aligned(16))) vertex_f_t;

#endif /*__DRAW_TYPES_H__*/
