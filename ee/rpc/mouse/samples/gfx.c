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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <tamtypes.h>
#include <debug.h>
#include <gs_psm.h>
#include <dma.h>
#include <packet.h>
#include <graph.h>
#include <draw.h>
#include <draw2d.h>

#include "gfx.h"

/*
 * cursor.raw is the pointer image: 16x16 pixels, 4 bytes per pixel (R, G, B, A).
 * The alpha goes from 0 (transparent) to 0x80 (opaque), as the GS expects.
 */
#include "cursor.c"

extern unsigned char cursor[];

/* A rectangle uses 3 quadwords and the pointer 4. */
#define PACKET_QWORDS 200

int screen_height;

static packet_t *packet;
static qword_t *current;

/* Send the packet to the GS and wait until everything in it is drawn. */
static void send_packet(qword_t *end)
{
    dma_wait_fast();
    dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, end - packet->data, 0, 0);
    draw_wait_finish();
}

/*
 * The text is drawn by libdebug in a 32 bit framebuffer at the start of the VRAM, so the
 * rectangles and the pointer are drawn in the same framebuffer.
 *
 * libdebug shows only the top half of that framebuffer (each row is stretched to two lines
 * of the TV), so everything drawn in the bottom half stays out of the screen.
 * graph_initialize() sets the display again, to show the whole framebuffer.
 */
void gfx_init(void)
{
    framebuffer_t frame;
    zbuffer_t z;
    texbuffer_t texbuf;
    clutbuffer_t clut;
    lod_t lod;
    qword_t *q;

    screen_height = (graph_get_region() == GRAPH_MODE_PAL) ? 512 : 448;

    dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
    dma_channel_fast_waits(DMA_CHANNEL_GIF);

    frame.width = SCREEN_WIDTH;
    frame.height = screen_height;
    frame.mask = 0;
    frame.psm = GS_PSM_32;
    frame.address = graph_vram_allocate(frame.width, frame.height, frame.psm, GRAPH_ALIGN_PAGE);

    graph_initialize(frame.address, frame.width, frame.height, frame.psm, 0, 0);

    /* No depth test is needed for 2D drawing. */
    z.enable = DRAW_DISABLE;
    z.mask = 1;
    z.method = ZTEST_METHOD_ALLPASS;
    z.zsm = GS_ZBUF_32;
    z.address = graph_vram_allocate(frame.width, frame.height, z.zsm, GRAPH_ALIGN_PAGE);

    /* The 16x16 pointer image is stored in a texture buffer that is 64 pixels wide. */
    texbuf.width = 64;
    texbuf.psm = GS_PSM_32;
    texbuf.address = graph_vram_allocate(64, CURSOR_SIZE, GS_PSM_32, GRAPH_ALIGN_BLOCK);

    packet = packet_init(PACKET_QWORDS, PACKET_NORMAL);

    q = packet->data;
    q = draw_setup_environment(q, 0, &frame, &z);
    q = draw_finish(q);
    send_packet(q);

    q = packet->data;
    q = draw_texture_transfer(q, cursor, CURSOR_SIZE, CURSOR_SIZE, GS_PSM_32, texbuf.address, texbuf.width);
    q = draw_texture_flush(q);
    dma_wait_fast();
    dma_channel_send_chain(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
    dma_wait_fast();

    lod.calculation = LOD_USE_K;
    lod.max_level = 0;
    lod.mag_filter = LOD_MAG_NEAREST;
    lod.min_filter = LOD_MIN_NEAREST;
    lod.l = 0;
    lod.k = 0;

    texbuf.info.width = draw_log2(CURSOR_SIZE);
    texbuf.info.height = draw_log2(CURSOR_SIZE);
    texbuf.info.components = TEXTURE_COMPONENTS_RGBA;
    texbuf.info.function = TEXTURE_FUNCTION_MODULATE;

    clut.storage_mode = CLUT_STORAGE_MODE1;
    clut.start = 0;
    clut.psm = 0;
    clut.load_method = CLUT_NO_LOAD;
    clut.address = 0;

    q = packet->data;
    q = draw_texture_sampling(q, 0, &lod);
    q = draw_texturebuffer(q, 0, &texbuf, &clut);
    q = draw_finish(q);
    send_packet(q);

    /* The pointer image has an alpha channel. */
    draw_enable_blending();
}

void gfx_wait_vsync(void)
{
    graph_wait_vsync();
}

void gfx_begin(void)
{
    current = packet->data;
}

/* Draw what is in the packet, and start a new one. */
static void flush(void)
{
    current = draw_finish(current);
    send_packet(current);
    current = packet->data;
}

void gfx_rect(int x, int y, int width, int height, u32 color)
{
    rect_t rect;

    if (current - packet->data > PACKET_QWORDS - 8)
        flush();

    rect.v0.x = x;
    rect.v0.y = y;
    rect.v0.z = 0;
    rect.v1.x = x + width - 1;
    rect.v1.y = y + height - 1;
    rect.v1.z = 0;
    rect.color.r = color & 0xFF;
    rect.color.g = (color >> 8) & 0xFF;
    rect.color.b = (color >> 16) & 0xFF;
    rect.color.a = 0x80;
    rect.color.q = 1.0f;

    current = draw_rect_filled(current, 0, &rect);
}

void gfx_pointer(int x, int y)
{
    texrect_t pointer;

    if (current - packet->data > PACKET_QWORDS - 8)
        flush();

    pointer.v0.x = x;
    pointer.v0.y = y;
    pointer.v0.z = 0;
    pointer.t0.u = 0.0f;
    pointer.t0.v = 0.0f;
    pointer.v1.x = x + CURSOR_SIZE - 1;
    pointer.v1.y = y + CURSOR_SIZE - 1;
    pointer.t1.u = CURSOR_SIZE;
    pointer.t1.v = CURSOR_SIZE;
    pointer.color.r = 0x80;
    pointer.color.g = 0x80;
    pointer.color.b = 0x80;
    pointer.color.a = 0x80;
    pointer.color.q = 1.0f;

    current = draw_rect_textured(current, 0, &pointer);
}

void gfx_end(void)
{
    flush();
}

static void draw_string(int x, int y, u32 color, u32 background, const char *text, int length)
{
    int i;

    scr_setbgcolor(background);
    for (i = 0; i < length; i++)
        scr_putchar(x + i * 8, y, color, text[i]);
    scr_setbgcolor(GFX_BLACK);
}

void gfx_text(int x, int y, u32 color, u32 background, const char *format, ...)
{
    char text[TEXT_COLUMNS + 1];
    va_list args;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);

    draw_string(x, y, color, background, text, strlen(text));
}

/*
 * If the line was longer the last time, the rest of it is cleared, so the black background
 * of the letters is drawn only where it is needed.
 */
void gfx_print_line(int line, u32 background, const char *format, ...)
{
    static int old_length[TEXT_LINES];
    char text[TEXT_COLUMNS + 1];
    va_list args;
    int length, i;

    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);

    length = strlen(text);
    for (i = length; i < old_length[line]; i++)
        text[i] = ' ';
    text[i] = '\0';

    draw_string(TEXT_X, TEXT_Y + line * TEXT_LINE, GFX_WHITE, background, text, strlen(text));
    old_length[line] = length;
}

void gfx_clear_lines(int first, int last)
{
    int line;

    for (line = first; line <= last; line++)
        gfx_print_line(line, GFX_BLACK, "%s", "");
}
