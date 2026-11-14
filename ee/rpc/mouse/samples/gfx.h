/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: the screen. Draws rectangles, the pointer and text.
*/

#ifndef GFX_H
#define GFX_H

#include <tamtypes.h>

#define SCREEN_WIDTH 640
#define CURSOR_SIZE  16

/* The text is drawn with the 8x8 font of libdebug, TEXT_LINE is the distance between two lines. */
#define TEXT_X       8
#define TEXT_Y       8
#define TEXT_LINE    12
#define TEXT_COLUMNS 44
#define TEXT_LINES   36

/* Colors of the rectangles and of the text. */
#define GFX_RGB(r, g, b) ((u32)(r) | ((u32)(g) << 8) | ((u32)(b) << 16))
#define GFX_BLACK        GFX_RGB(0x00, 0x00, 0x00)
#define GFX_WHITE        GFX_RGB(0xFF, 0xFF, 0xFF)
#define GFX_RED          GFX_RGB(0xFF, 0x00, 0x00)
#define GFX_BLUE         GFX_RGB(0x00, 0x00, 0xFF)
#define GFX_SELECTED     GFX_RGB(0x00, 0x60, 0xC0)
#define GFX_HOVER        GFX_RGB(0x40, 0x40, 0x40)
#define GFX_GUIDE        GFX_RGB(0x60, 0x60, 0x60)

/* The height of the screen is 448 on NTSC consoles and 512 on PAL consoles. Set by gfx_init(). */
extern int screen_height;

void gfx_init(void);
void gfx_wait_vsync(void);

/* Rectangles and the pointer are collected between gfx_begin() and gfx_end(), and drawn at once. */
void gfx_begin(void);
void gfx_rect(int x, int y, int width, int height, u32 color);
void gfx_pointer(int x, int y);
void gfx_end(void);

/* Text at any position of the screen. The letters have a background of the given color. */
void gfx_text(int x, int y, u32 color, u32 background, const char *format, ...) __attribute__((format(printf, 5, 6)));

/* A line of text on the grid of TEXT_LINE. What was left of a longer line is cleared. */
void gfx_print_line(int line, u32 background, const char *format, ...) __attribute__((format(printf, 3, 4)));
void gfx_clear_lines(int first, int last);

#endif
