/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: shows the state of a USB mouse as text, draws the
# mouse pointer from a 16x16 RGBA image and has a square that changes
# color when it is clicked.
*/

#include <stdio.h>
#include <unistd.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <debug.h>
#include <libmouse.h>
#include <gs_psm.h>
#include <dma.h>
#include <packet.h>
#include <graph.h>
#include <draw.h>
#include <draw2d.h>

/*
 * cursor.raw is the pointer image: 16x16 pixels, 4 bytes per pixel (R, G, B, A).
 * The alpha goes from 0 (transparent) to 0x80 (opaque), as the GS expects.
 */
#include "cursor.c"

extern unsigned char cursor[];

extern unsigned char usbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbd_irx;

extern unsigned char ps2mouse_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2mouse_irx;

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 448

#define CURSOR_SIZE   16

/* Many TVs do not show the borders of the picture, so the pointer stays away from them. */
#define SAFE_X        16
#define SAFE_Y        48

#define AREA_LEFT     SAFE_X
#define AREA_TOP      SAFE_Y
#define AREA_RIGHT    (SCREEN_WIDTH - SAFE_X - CURSOR_SIZE)
#define AREA_BOTTOM   (SCREEN_HEIGHT - SAFE_Y - CURSOR_SIZE)

/* The mouse moves this many times slower than its raw counts, 1 would be the raw speed. */
#define POINTER_SLOWDOWN 3

#define SQUARE_X      270
#define SQUARE_Y      250
#define SQUARE_SIZE   100

static packet_t *packet;

static void load_module(const char *name, void *irx, unsigned int size)
{
    int ret = SifExecModuleBuffer(irx, size, 0, NULL, NULL);

    if (ret < 0) {
        scr_printf("Failed to load %s: %d\n", name, ret);
        SleepThread();
    }
}

/* Send the packet to the GS and wait until everything in it is drawn. */
static void send_packet(qword_t *end)
{
    dma_wait_fast();
    dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, end - packet->data, 0, 0);
    draw_wait_finish();
}

/*
 * The text of scr_printf() is drawn by libdebug in a 640x448 32 bit framebuffer at the
 * start of the VRAM, so the pointer and the square are drawn in the same framebuffer.
 */
static void setup_graphics(void)
{
    framebuffer_t frame;
    zbuffer_t z;
    texbuffer_t texbuf;
    clutbuffer_t clut;
    lod_t lod;
    qword_t *q;

    dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
    dma_channel_fast_waits(DMA_CHANNEL_GIF);

    frame.width = SCREEN_WIDTH;
    frame.height = SCREEN_HEIGHT;
    frame.mask = 0;
    frame.psm = GS_PSM_32;
    frame.address = graph_vram_allocate(frame.width, frame.height, frame.psm, GRAPH_ALIGN_PAGE);

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

    packet = packet_init(50, PACKET_NORMAL);

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

static qword_t *add_rect(qword_t *q, int x, int y, int size, int r, int g, int b)
{
    rect_t rect;

    rect.v0.x = x;
    rect.v0.y = y;
    rect.v0.z = 0;
    rect.v1.x = x + size - 1;
    rect.v1.y = y + size - 1;
    rect.v1.z = 0;
    rect.color.r = r;
    rect.color.g = g;
    rect.color.b = b;
    rect.color.a = 0x80;
    rect.color.q = 1.0f;

    return draw_rect_filled(q, 0, &rect);
}

static void erase_pointer(int x, int y)
{
    qword_t *q = packet->data;

    q = add_rect(q, x, y, CURSOR_SIZE, 0, 0, 0);
    q = draw_finish(q);
    send_packet(q);
}

static void draw_scene(int square_is_blue, int pointer_x, int pointer_y)
{
    qword_t *q = packet->data;
    texrect_t pointer;

    if (square_is_blue)
        q = add_rect(q, SQUARE_X, SQUARE_Y, SQUARE_SIZE, 0x00, 0x00, 0xFF);
    else
        q = add_rect(q, SQUARE_X, SQUARE_Y, SQUARE_SIZE, 0xFF, 0x00, 0x00);

    pointer.v0.x = pointer_x;
    pointer.v0.y = pointer_y;
    pointer.v0.z = 0;
    pointer.t0.u = 0.0f;
    pointer.t0.v = 0.0f;
    pointer.v1.x = pointer_x + CURSOR_SIZE - 1;
    pointer.v1.y = pointer_y + CURSOR_SIZE - 1;
    pointer.t1.u = CURSOR_SIZE;
    pointer.t1.v = CURSOR_SIZE;
    pointer.color.r = 0x80;
    pointer.color.g = 0x80;
    pointer.color.b = 0x80;
    pointer.color.a = 0x80;
    pointer.color.q = 1.0f;
    q = draw_rect_textured(q, 0, &pointer);

    q = draw_finish(q);
    send_packet(q);
}

static const char *state(u32 buttons, u32 button)
{
    return (buttons & button) ? "down" : "up";
}

static int clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

/* In absolute mode the driver keeps the position in raw counts, inside the area of the pointer. */
static void set_driver_position(int pointer_x, int pointer_y)
{
    PS2MouseSetPosition((pointer_x - AREA_LEFT) * POINTER_SLOWDOWN, (pointer_y - AREA_TOP) * POINTER_SLOWDOWN);
}

int main(int argc, char *argv[])
{
    PS2MouseData data;
    const char *last_double_click = "none";
    u32 previous_buttons = 0;
    u32 pressed;
    int mode = PS2MOUSE_READMODE_ABS;
    int pointer_x = SCREEN_WIDTH / 2, pointer_y = SCREEN_HEIGHT / 2;
    int old_x = pointer_x, old_y = pointer_y;
    int rest_x = 0, rest_y = 0;
    int step_x, step_y;
    int square_is_blue = 0;

    (void)argc;
    (void)argv;

    init_scr();

    SifInitRpc(0);
    while (!SifIopReset(NULL, 0)) {};
    while (!SifIopSync()) {};
    SifInitRpc(0);

    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    load_module("usbd.irx", usbd_irx, size_usbd_irx);
    load_module("ps2mouse.irx", ps2mouse_irx, size_ps2mouse_irx);

    if (PS2MouseInit() < 0) {
        scr_printf("PS2MouseInit failed\n");
        SleepThread();
    }

    scr_printf("PS2 USB mouse sample (driver version 0x%x)\n", (unsigned int)PS2MouseGetVersion());
    scr_printf("Waiting for a USB mouse...\n");
    while (PS2MouseEnum() == 0)
        usleep(100000);

    PS2MouseSetBoundary(0, (AREA_RIGHT - AREA_LEFT) * POINTER_SLOWDOWN, 0, (AREA_BOTTOM - AREA_TOP) * POINTER_SLOWDOWN);
    PS2MouseSetReadMode(mode);
    set_driver_position(pointer_x, pointer_y);

    scr_clear();
    setup_graphics();

    while (1) {
        graph_wait_vsync();

        if (PS2MouseRead(&data) < 0) {
            scr_clearline(14);
            scr_setXY(0, 14);
            scr_printf("PS2MouseRead failed");
            continue;
        }

        pressed = data.buttons & ~previous_buttons & 0xFF;
        previous_buttons = data.buttons & 0xFF;

        if (pressed & PS2MOUSE_BTN3) {
            mode = (mode == PS2MOUSE_READMODE_ABS) ? PS2MOUSE_READMODE_DIFF : PS2MOUSE_READMODE_ABS;
            PS2MouseSetReadMode(mode);
            set_driver_position(pointer_x, pointer_y);
            rest_x = 0;
            rest_y = 0;
            /* The data of this read belongs to the old mode, do not move the pointer with it. */
            data.x = (pointer_x - AREA_LEFT) * POINTER_SLOWDOWN;
            data.y = (pointer_y - AREA_TOP) * POINTER_SLOWDOWN;
            if (mode == PS2MOUSE_READMODE_DIFF) {
                data.x = 0;
                data.y = 0;
            }
        }

        if (mode == PS2MOUSE_READMODE_ABS) {
            pointer_x = AREA_LEFT + data.x / POINTER_SLOWDOWN;
            pointer_y = AREA_TOP + data.y / POINTER_SLOWDOWN;
        } else {
            /* Keep what is left after the division, so that slow movements are not lost. */
            rest_x += data.x;
            rest_y += data.y;
            step_x = rest_x / POINTER_SLOWDOWN;
            step_y = rest_y / POINTER_SLOWDOWN;
            rest_x -= step_x * POINTER_SLOWDOWN;
            rest_y -= step_y * POINTER_SLOWDOWN;
            pointer_x += step_x;
            pointer_y += step_y;
        }
        pointer_x = clamp(pointer_x, AREA_LEFT, AREA_RIGHT);
        pointer_y = clamp(pointer_y, AREA_TOP, AREA_BOTTOM);

        /* A click with button 1 on the square changes its color. */
        if ((pressed & PS2MOUSE_BTN1) &&
            pointer_x >= SQUARE_X && pointer_x < SQUARE_X + SQUARE_SIZE &&
            pointer_y >= SQUARE_Y && pointer_y < SQUARE_Y + SQUARE_SIZE)
            square_is_blue = !square_is_blue;

        if (data.buttons & PS2MOUSE_BTN1DBL)
            last_double_click = "button 1";
        if (data.buttons & PS2MOUSE_BTN2DBL)
            last_double_click = "button 2";
        if (data.buttons & PS2MOUSE_BTN3DBL)
            last_double_click = "button 3";

        /* Erase the old pointer first, then draw the text, the square and the new pointer on top. */
        erase_pointer(old_x, old_y);

        scr_setXY(0, 1);
        scr_printf("PS2 USB mouse sample");

        scr_clearline(4);
        scr_setXY(0, 4);
        scr_printf("Mice connected: %u", (unsigned int)PS2MouseEnum());

        scr_clearline(5);
        scr_setXY(0, 5);
        scr_printf("Read mode: %s", (mode == PS2MOUSE_READMODE_ABS) ? "absolute" : "difference");

        scr_clearline(6);
        scr_setXY(0, 6);
        scr_printf("Pointer: x=%d y=%d", pointer_x, pointer_y);

        scr_clearline(7);
        scr_setXY(0, 7);
        scr_printf("Wheel: %d", (int)data.wheel);

        scr_clearline(8);
        scr_setXY(0, 8);
        scr_printf("Buttons: 1=%s 2=%s 3=%s", state(data.buttons, PS2MOUSE_BTN1),
                   state(data.buttons, PS2MOUSE_BTN2), state(data.buttons, PS2MOUSE_BTN3));

        scr_clearline(9);
        scr_setXY(0, 9);
        scr_printf("Last double click: %s", last_double_click);

        scr_setXY(0, 11);
        scr_printf("Button 3 (wheel click): switch between absolute and difference mode");
        scr_setXY(0, 12);
        scr_printf("Button 1 on the square: change its color");

        draw_scene(square_is_blue, pointer_x, pointer_y);

        old_x = pointer_x;
        old_y = pointer_y;
    }

    return 0;
}
