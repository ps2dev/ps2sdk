#ifndef __DRAW_BUFFER_H__
#define __DRAW_BUFFER_H__

#include <tamtypes.h>

// Texture Color Components
#define TEXTURE_COMPONENTS_RGB		0
#define TEXTURE_COMPONENTS_RGBA		1

// Texture Function
#define TEXTURE_FUNCTION_MODULATE	0
#define TEXTURE_FUNCTION_DECAL		1
#define TEXTURE_FUNCTION_HIGHLIGHT	2
#define TEXTURE_FUNCTION_HIGHLIGHT2	3

// CLUT Storage Mode
#define CLUT_STORAGE_MODE1			0
#define CLUT_STORAGE_MODE2			1

// CLUT Load Control
#define CLUT_NO_LOAD				0
#define CLUT_LOAD					1
#define CLUT_LOAD_COPY_CBP0			2
#define CLUT_LOAD_COPY_CBP1			3
#define CLUT_COMPARE_CBP0			4
#define CLUT_COMPARE_CBP1			5

typedef struct {
	unsigned int address;
	unsigned int width;
	unsigned int height;
	unsigned int psm;
	unsigned int mask;
} FRAMEBUFFER;

typedef struct {
	unsigned int enable;
	unsigned int method;
	unsigned int address;
	unsigned int zsm;
	unsigned int mask;
} ZBUFFER;

typedef struct {
	unsigned int address;
	unsigned int width;
	unsigned int psm;
} TEXBUFFER;

typedef struct {
	unsigned int address;
	unsigned int psm;
	unsigned int storage_mode;
	unsigned int start;
	unsigned int load_method;
} CLUTBUFFER;

typedef struct {
	unsigned char width;
	unsigned char height;
	unsigned char components;
	unsigned char function;
} TEXTURE;

#ifdef __cplusplus
extern "C" {
#endif

	// Returns the power of 2 needed for texture width and height
	unsigned char draw_log2(unsigned int x);

	// Framebuffer Attributes
	QWORD *draw_framebuffer(QWORD *q, int context, FRAMEBUFFER *frame);

	// ZBuffer Attributes
	QWORD *draw_zbuffer(QWORD *q, int context, ZBUFFER *zbuffer);

	// TextureBuffer Attributes
	QWORD *draw_texturebuffer(QWORD *q, int context, TEXBUFFER *texbuffer, TEXTURE *texture, CLUTBUFFER *clut);

	// CLUT Storage Mode 1 Information
	QWORD *draw_clutbuffer(QWORD *q, int context, int psm, CLUTBUFFER *clut);

	// CLUT Storage Mode 2 Information
	QWORD *draw_clut_offset(QWORD *q, int cbw, int u, int v);

#ifdef __cplusplus
}
#endif

#endif /*__DRAW_BUFFER_H__*/
