#ifndef __DRAW_SAMPLING_H__
#define __DRAW_SAMPLING_H__

#include <tamtypes.h>

// Level of Detail
#define LOD_FORMULAIC				0
#define LOD_USE_K					1

// Texture Scaling
#define LOD_MAG_NEAREST				0
#define LOD_MAG_LINEAR				1
#define LOD_MIN_NEAREST				0
#define LOD_MIN_LINEAR				1
#define LOD_MIN_NEAR_MIPMAP_NEAR	2
#define LOD_MIN_NEAR_MIPMAP_LINE	3
#define LOD_MIN_LINE_MIPMAP_NEAR	4
#define LOD_MIN_LINE_MIPMAP_LINE	5

// Mipmaps
#define LOD_MIPMAP_REGISTER			0
#define LOD_MIPMAP_CALCULATE		1

// Texture wrapping
#define WRAP_REPEAT					0
#define WRAP_CLAMP					1
#define WRAP_REGION_CLAMP			2
#define WRAP_REGION_REPEAT			3

// Texture Alpha Expansion
#define ALPHA_EXPAND_NORMAL			0
#define ALPHA_EXPAND_TRANSPARENT	1

typedef struct {
	unsigned char calculation;
	unsigned char max_level;
	unsigned char mag_filter;
	unsigned char min_filter;
	unsigned char mipmap_select;
	unsigned char l;
	float k;
} LOD;

typedef struct {
	int address1;
	int address2;
	int address3;
	char width1;
	char width2;
	char width3;
} MIPMAP;

typedef struct {
	unsigned char horizontal;
	unsigned char vertical;
	int minu, maxu;
	int minv, maxv;
} WRAP;

#ifdef __cplusplus
extern "C" {
#endif

	// Texture Sampling, Level-of-Detail, and Filtering
	QWORD *draw_texture_sampling(QWORD *q, int context, LOD *lod);

	// Mipmap levels 1-3
	QWORD *draw_mipmap1(QWORD *q, int context, MIPMAP *mipmap);

	// Mipmap levels 4-6
	QWORD *draw_mipmap2(QWORD *q, int context, MIPMAP *mipmap);

	// Texture Clamping
	QWORD *draw_texture_wrapping(QWORD *q, int context, WRAP *wrap);

	// Alpha Expansion Values
	QWORD *draw_texture_expand_alpha(QWORD *q, unsigned char zero_value, int expand, unsigned char one_value);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_SAMPLING__*/
