/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: tries everything that the mouse driver can do. The calls to the
# driver are in this file and in settings.c, the other files only draw and use input_t.
#
# The square (square.c):
#   - button 1: changes its color, holding it drags it, a double click puts it in the center
#   - button 2: opens the menu of sizes (menu.c), holding it changes the size all the time,
#     a double click gives it the original size
#   - wheel: changes its size
# The settings (settings.c), with the pointer on the text:
#   - wheel: changes the setting, or button 1 to raise it and button 2 to lower it
# Everywhere else:
#   - button 2: puts the pointer back in the center of the screen
#   - button 3: changes the read mode (absolute or difference), a double click resets everything
# The pointer is drawn only while a mouse is plugged in, and it comes back to the place
# where it was when the mouse is plugged in again.
*/

#include <stdio.h>
#include <string.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <debug.h>
#include <libmouse.h>

#include "input.h"
#include "gfx.h"
#include "square.h"
#include "menu.h"
#include "settings.h"

extern unsigned char usbd_irx[] __attribute__((aligned(16)));
extern unsigned int size_usbd_irx;

extern unsigned char ps2mouse_irx[] __attribute__((aligned(16)));
extern unsigned int size_ps2mouse_irx;

/* Lines of the text. The settings are in SETTINGS_FIRST_LINE and the next ones. */
#define LINE_TITLE   0
#define LINE_ERROR   1
#define LINE_MICE    2
#define LINE_MODE    3
#define LINE_POINTER 4
#define LINE_WHEEL   5
#define LINE_BUTTONS 6
#define LINE_DOUBLE  7
#define LINE_HELP    29

typedef struct
{
    int mode;
    int pointer_x, pointer_y;
    int old_x, old_y;
    /* What is left of the movement after dividing it by POINTER_SLOWDOWN, in difference mode. */
    int rest_x, rest_y;
    int wheel_total;
    const char *last_double_click;
    area_t area;
    square_t square;
    menu_t menu;
    settings_t settings;
} app_t;

static void load_module(const char *name, void *irx, unsigned int size)
{
    int ret = SifExecModuleBuffer(irx, size, 0, NULL, NULL);

    if (ret < 0) {
        scr_printf("Failed to load %s: %d\n", name, ret);
        SleepThread();
    }
}

static const char *state(u32 buttons, u32 button)
{
    return (buttons & button) ? "down" : "up";
}

/*
 * In absolute mode the driver keeps the position in raw counts, inside the area of the pointer.
 * In difference mode the driver has no position: what is set there would be read as a movement.
 */
static void set_driver_position(const app_t *app)
{
    if (app->mode == PS2MOUSE_READMODE_ABS)
        PS2MouseSetPosition((app->pointer_x - app->area.left) * POINTER_SLOWDOWN, (app->pointer_y - app->area.top) * POINTER_SLOWDOWN);
}

/* Put the pointer somewhere, and tell the driver. */
static void move_pointer(app_t *app, int x, int y)
{
    app->pointer_x = clamp(x, app->area.left, app->area.right);
    app->pointer_y = clamp(y, app->area.top, app->area.bottom);
    app->rest_x = 0;
    app->rest_y = 0;
    set_driver_position(app);
}

static void change_read_mode(app_t *app)
{
    app->mode = (PS2MouseGetReadMode() == PS2MOUSE_READMODE_ABS) ? PS2MOUSE_READMODE_DIFF : PS2MOUSE_READMODE_ABS;
    PS2MouseSetReadMode(app->mode);
    move_pointer(app, app->pointer_x, app->pointer_y);
}

/* Everything as when the sample starts. */
static void reset_all(app_t *app)
{
    PS2MouseReset();

    app->mode = PS2MOUSE_READMODE_ABS;
    PS2MouseSetReadMode(app->mode);
    settings_reset(&app->settings, &app->area);

    move_pointer(app, SCREEN_WIDTH / 2, screen_height / 2);
    app->wheel_total = 0;
    app->last_double_click = "none";

    square_reset(&app->square);
    menu_close(&app->menu);
}

/* Fills the input of this frame, from what the driver says and from the last frame. */
static void make_input(input_t *input, const PS2MouseData *data, const app_t *app, u32 *previous_buttons, int *held_frames)
{
    u32 down = data->buttons & 0xFF;
    int i;

    input->x = app->pointer_x;
    input->y = app->pointer_y;
    input->wheel = (int)data->wheel;
    input->buttons = data->buttons;
    input->pressed = down & ~*previous_buttons & 0x7;
    input->released = *previous_buttons & ~down & 0x7;

    for (i = 0; i < 3; i++) {
        if (down & (1 << i))
            held_frames[i]++;
        input->hold[i] = held_frames[i];
        if (input->released & (1 << i))
            held_frames[i] = 0;
    }

    *previous_buttons = down;
}

/* Move the pointer with what the mouse says. */
static void update_pointer(app_t *app, const PS2MouseData *data)
{
    int step_x, step_y;

    if (app->mode == PS2MOUSE_READMODE_ABS) {
        app->pointer_x = app->area.left + data->x / POINTER_SLOWDOWN;
        app->pointer_y = app->area.top + data->y / POINTER_SLOWDOWN;
    } else {
        /* Keep what is left after the division, so that slow movements are not lost. */
        app->rest_x += data->x;
        app->rest_y += data->y;
        step_x = app->rest_x / POINTER_SLOWDOWN;
        step_y = app->rest_y / POINTER_SLOWDOWN;
        app->rest_x -= step_x * POINTER_SLOWDOWN;
        app->rest_y -= step_y * POINTER_SLOWDOWN;
        app->pointer_x += step_x;
        app->pointer_y += step_y;
    }
    app->pointer_x = clamp(app->pointer_x, app->area.left, app->area.right);
    app->pointer_y = clamp(app->pointer_y, app->area.top, app->area.bottom);
}

/* Give the input to the square, the menu and the settings. */
static void handle_input(app_t *app, const input_t *input)
{
    int consumed = 0;
    int open_menu = 0;
    int area_changed = 0;
    int size;
    int on_square = square_contains(&app->square, input->x, input->y);
    int settings_active = !menu_is_open(&app->menu) && !square_is_busy(&app->square) && !on_square;

    if ((input->buttons & PS2MOUSE_BTN2DBL) && on_square) {
        /* A double click of button 2 on the square: it gets its original size. */
        menu_close(&app->menu);
        square_double_click_button2(&app->square);
        consumed = 1;
    } else if (menu_is_open(&app->menu)) {
        if (menu_update(&app->menu, input, &size))
            square_set_size(&app->square, size);
        consumed = 1;
    } else {
        consumed = settings_update(&app->settings, input, settings_active, &app->area, &area_changed);
        if (!consumed) {
            consumed = square_update(&app->square, input, &open_menu);
            if (open_menu)
                menu_open(&app->menu, input->x, input->y);
        }
    }

    if (area_changed)
        move_pointer(app, app->pointer_x, app->pointer_y);

    /* Button 2 anywhere else puts the pointer in the center of the screen. */
    if (!consumed && (input->pressed & PS2MOUSE_BTN2))
        move_pointer(app, SCREEN_WIDTH / 2, screen_height / 2);
}

static void draw_text(app_t *app, const PS2MouseData *data, u32 mice, u32 version, int mouse_present)
{
    gfx_print_line(LINE_TITLE, GFX_BLACK, "PS2 USB mouse sample (driver 0x%x)", (unsigned int)version);
    gfx_print_line(LINE_MICE, GFX_BLACK, "Mice connected: %u", (unsigned int)mice);

    if (!mouse_present) {
        gfx_print_line(LINE_ERROR, GFX_BLACK, "%s", "");
        gfx_print_line(LINE_MODE, GFX_BLACK, "Waiting for a USB mouse...");
        gfx_clear_lines(LINE_POINTER, TEXT_LINES - 1);
        return;
    }

    gfx_print_line(LINE_MODE, GFX_BLACK, "Read mode: %s", (PS2MouseGetReadMode() == PS2MOUSE_READMODE_ABS) ? "absolute" : "difference");
    gfx_print_line(LINE_POINTER, GFX_BLACK, "Pointer: x=%d y=%d", app->pointer_x, app->pointer_y);
    gfx_print_line(LINE_WHEEL, GFX_BLACK, "Wheel: %d", app->wheel_total);
    gfx_print_line(LINE_BUTTONS, GFX_BLACK, "Buttons: 1=%s 2=%s 3=%s", state(data->buttons, PS2MOUSE_BTN1),
                   state(data->buttons, PS2MOUSE_BTN2), state(data->buttons, PS2MOUSE_BTN3));
    gfx_print_line(LINE_DOUBLE, GFX_BLACK, "Last double click: %s", app->last_double_click);
    settings_print(&app->settings);

    gfx_print_line(LINE_HELP, GFX_BLACK, "Button 1: color; hold: drag; twice: home");
    gfx_print_line(LINE_HELP + 1, GFX_BLACK, "Button 2: size menu; hold: cycle; twice: 64");
    gfx_print_line(LINE_HELP + 2, GFX_BLACK, "Button 2 elsewhere: center the pointer");
    gfx_print_line(LINE_HELP + 3, GFX_BLACK, "Wheel on the square: change its size");
    gfx_print_line(LINE_HELP + 4, GFX_BLACK, "Wheel on a setting: change it");
    gfx_print_line(LINE_HELP + 5, GFX_BLACK, "Button 3: read mode; twice: reset all");
}

int main(int argc, char *argv[])
{
    static app_t app;
    PS2MouseData data;
    input_t input;
    u32 previous_buttons = 0;
    u32 mice;
    u32 version;
    int held_frames[3] = {0, 0, 0};
    int mouse_present = 0;

    (void)argc;
    (void)argv;

    memset(&data, 0, sizeof(data));
    memset(&input, 0, sizeof(input));

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

    version = PS2MouseGetVersion();

    scr_clear();
    gfx_init();

    /* Everything starts in the center of the screen. */
    app.mode = PS2MOUSE_READMODE_ABS;
    app.last_double_click = "none";
    settings_init(&app.settings, &app.area);
    square_init(&app.square);
    menu_init(&app.menu);
    PS2MouseSetReadMode(app.mode);
    app.pointer_x = SCREEN_WIDTH / 2;
    app.pointer_y = screen_height / 2;
    app.old_x = app.pointer_x;
    app.old_y = app.pointer_y;
    set_driver_position(&app);

    while (1) {
        gfx_wait_vsync();

        mice = PS2MouseEnum();

        if (mice == 0 && mouse_present) {
            /* The mouse was unplugged, the pointer is not drawn until it is plugged in again. */
            mouse_present = 0;
            square_cancel(&app.square);
            menu_close(&app.menu);
            app.settings.hover = -1;
            memset(&data, 0, sizeof(data));
        } else if (mice != 0 && !mouse_present) {
            /* A mouse was plugged in, the pointer starts where it was the last time. */
            mouse_present = 1;
            move_pointer(&app, app.pointer_x, app.pointer_y);
            previous_buttons = 0;
            memset(held_frames, 0, sizeof(held_frames));
        }

        if (mouse_present) {
            if (PS2MouseRead(&data) < 0) {
                gfx_print_line(LINE_ERROR, GFX_BLACK, "PS2MouseRead failed");
                continue;
            }
            gfx_print_line(LINE_ERROR, GFX_BLACK, "%s", "");

            /* Button 3 changes the read mode, and two clicks reset everything. */
            if ((data.buttons & 0x7 & ~previous_buttons) & PS2MOUSE_BTN3) {
                change_read_mode(&app);
                /* The data of this read belongs to the old mode, do not move the pointer with it. */
                data.x = (app.pointer_x - app.area.left) * POINTER_SLOWDOWN;
                data.y = (app.pointer_y - app.area.top) * POINTER_SLOWDOWN;
                if (app.mode == PS2MOUSE_READMODE_DIFF) {
                    data.x = 0;
                    data.y = 0;
                }
            }

            update_pointer(&app, &data);
            app.wheel_total += (int)data.wheel;
            make_input(&input, &data, &app, &previous_buttons, held_frames);

            if (data.buttons & PS2MOUSE_BTN3DBL)
                reset_all(&app);
            else
                handle_input(&app, &input);

            if (data.buttons & PS2MOUSE_BTN1DBL)
                app.last_double_click = "button 1";
            if (data.buttons & PS2MOUSE_BTN2DBL)
                app.last_double_click = "button 2";
            if (data.buttons & PS2MOUSE_BTN3DBL)
                app.last_double_click = "button 3";
        }

        /* Erase what was drawn the last time, and draw the square, the text, the menu and the pointer, in this order. */
        gfx_begin();
        square_erase(&app.square);
        menu_erase(&app.menu);
        settings_erase_guide(&app.settings);
        gfx_rect(app.old_x, app.old_y, CURSOR_SIZE, CURSOR_SIZE, GFX_BLACK);
        gfx_end();

        gfx_begin();
        square_draw(&app.square);
        settings_draw_guide(&app.settings, &app.area);
        gfx_end();

        draw_text(&app, &data, mice, version, mouse_present);
        menu_draw(&app.menu, app.square.size);

        if (mouse_present) {
            gfx_begin();
            gfx_pointer(app.pointer_x, app.pointer_y);
            gfx_end();
        }

        app.old_x = app.pointer_x;
        app.old_y = app.pointer_y;
    }

    return 0;
}
