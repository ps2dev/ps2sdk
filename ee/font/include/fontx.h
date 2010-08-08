#ifndef __FONTX_H__
#define __FONTX_H__

typedef struct {
	char name[9];	// Name of font
	char bold;		// Enable/Disable bold effect
	char w_margin;	// Margin width between characters
	char h_margin;	// Margin height between lines
	int rowsize;	// Size in byte of each row
	int charsize;	// Size in bytes of each character
	int offset;		// Offset from beginning of font to character
	char *font;		// The font data
} FONTX;

#define SINGLE_BYTE 0
#define DOUBLE_BYTE 1

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

	int load_krom_single(FONTX *fontx);

	int load_krom_double(FONTX *fontx);

	int load_fontx(const char *path, FONTX* fontx, int type, int wmargin, int hmargin, int bold);

	void unload_fontx(FONTX *fontx);

	qword_t *fontx_print_ascii(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, FONTX *fontx);

	qword_t *fontx_print_sjis(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, FONTX *ascii, FONTX *kanji);
#ifdef __cplusplus
}
#endif

#endif /*__FONTX_H__*/
