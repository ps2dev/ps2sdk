#ifndef __FONT_H__
#define __FONT_H__

// FontX2 types
#define SINGLE_BYTE 0
#define DOUBLE_BYTE 1

typedef struct {
	char name[9];	// Name of font
	char bold;		// Enable/Disable bold effect
	char w_margin;	// Margin width between characters
	char h_margin;	// Margin height between lines
	int rowsize;	// Size in byte of each row
	int charsize;	// Size in bytes of each character
	int offset;		// Offset from beginning of font to character
	char *font;		// The font data
} fontx_t;

// FontStudio type fonts
typedef struct {
	char A;
	char C;
	char width;
	char height;
	unsigned short u1;
	unsigned short v1;
	unsigned short u2;
	unsigned short v2;
} inidata_t;

typedef struct {
	float scale;
	char lineheight;
	unsigned short *charmap;
	int totalchars;
	int spacewidth;
	inidata_t *chardata;
} fsfont_t;

// Alignments
#define LEFT_ALIGN 0
#define CENTER_ALIGN 1
#define RIGHT_ALIGN 2

#ifdef __cplusplus
extern "C" {
#endif

	// FontX2 type fonts

	// README: dma heavy
	// A 16x16 pt font with a character that consists of nothing but points
	// would take roughly 2 kilobytes to draw (16x16)*8 bytes

	// Loads a FontX2 type font with set characteristics
	// Use "rom0:KROM" as the path to load the PS2's internal FontX2 font
	int fontx_load(const char *path, fontx_t* fontx, int type, int wmargin, int hmargin, int bold);

	// Frees memory if a font is loaded
	void fontx_unload(fontx_t *fontx);

	// Prints an ascii/JISX201 formatted string
	qword_t *fontx_print_ascii(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, fontx_t *fontx);

	// Prints a SJIS formatted string
	qword_t *fontx_print_sjis(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, fontx_t *ascii, fontx_t *kanji);

	// FontStudio type fonts

	// README:
	// Make sure to setup the texture buffer with the font texture prior to printing

	// Loads the FontStudio ini file that contains the font texture's characteristics
	int fontstudio_load_ini(fsfont_t *font, const char *path, float width, float height, int lineheight);

	// Frees memory if a ini is loaded
	void fontstudio_unload_ini(fsfont_t *font);

	// Prints a unicode formatted string (UTF+8)
	qword_t *fontstudio_print_string(qword_t *q, int context, const unsigned char *string, int alignment, vertex_t *v0, color_t *c0, fsfont_t *font);

#ifdef __cplusplus
}
#endif

#endif /*__FONT_H__*/
