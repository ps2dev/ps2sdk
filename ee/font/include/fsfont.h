#ifndef __FSFONT_H__
#define __FSFONT_H__

typedef struct {
	char A;
	char C;
	char width;
	char height;
	unsigned short u1;
	unsigned short v1;
	unsigned short u2;
	unsigned short v2;
} inidata;

typedef struct {
	float scale;
	char lineheight;
	unsigned short *charmap;
	int totalchars;
	int spacewidth;
	inidata *chardata;
} FSFONT;

#ifndef LEFT_ALIGN
#define LEFT_ALIGN 0
#endif

#ifndef CENTER_ALIGN
#define CENTER_ALIGN 1
#endif

#ifndef RIGHT_ALIGN
#define RIGHT_ALIGN 2
#endif

#ifdef __cplusplus
extern "C" {
#endif

	int load_fontstudio_ini(FSFONT *font, const char *path, float width, float height, int lineheight);

	void unload_fontstudio_ini(FSFONT *font);

	qword_t *fontstudio_print_string(qword_t *q, int context, const unsigned char *string, int alignment, vertex_t *v0, color_t *c0, FSFONT *font);

#ifdef __cplusplus
}
#endif

#endif /*__FSFONT_H__*/
