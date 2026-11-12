/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: the settings of the driver that are changed with the mouse.
*/

#include <stdio.h>
#include <string.h>
#include <libmouse.h>

#include "gfx.h"
#include "settings.h"

/* The original settings of the driver. */
#define DEFAULT_ACCEL_TENTHS 10
#define DEFAULT_THRESHOLD    65536
#define DEFAULT_DOUBLE_CLICK 500

/* The acceleration changes by half a unit, and the double click time by 100 milliseconds. */
#define ACCEL_MIN_TENTHS   5
#define ACCEL_MAX_TENTHS   40
#define DOUBLE_CLICK_MIN   100
#define DOUBLE_CLICK_MAX   1000
#define DOUBLE_CLICK_STEP  100

/* Distance between the pointer and the borders of the screen when the margin is on. */
#define MARGIN 32

/*
 * A movement of the mouse that is as big as the threshold is multiplied by the acceleration.
 * The original threshold is so big that the acceleration is never used.
 */
static const u32 thresholds[] = {1, 2, 4, 8, 16, 32, DEFAULT_THRESHOLD};
#define THRESHOLD_COUNT (sizeof(thresholds) / sizeof(thresholds[0]))

static void make_area(int with_margin, area_t *area)
{
    int margin = with_margin ? MARGIN : 0;

    area->left = margin;
    area->top = margin;
    area->right = SCREEN_WIDTH - margin - CURSOR_SIZE;
    area->bottom = screen_height - margin - CURSOR_SIZE;
}

/* Read what the driver has, to show it. */
static void read_driver(settings_t *settings)
{
    int min_x, max_x, min_y, max_y;

    settings->accel_tenths = (int)(PS2MouseGetAccel() * 10.0f + 0.5f);
    settings->threshold = PS2MouseGetThres();
    settings->double_click_time = PS2MouseGetDblClickTIme();

    PS2MouseGetBoundary(&min_x, &max_x, &min_y, &max_y);
    settings->boundary_width = (max_x - min_x) / POINTER_SLOWDOWN + CURSOR_SIZE;
    settings->boundary_height = (max_y - min_y) / POINTER_SLOWDOWN + CURSOR_SIZE;
}

/* In absolute mode, the driver keeps the pointer inside the boundary. */
static void write_boundary(settings_t *settings, area_t *area)
{
    make_area(settings->margin, area);
    PS2MouseSetBoundary(0, (area->right - area->left) * POINTER_SLOWDOWN, 0, (area->bottom - area->top) * POINTER_SLOWDOWN);
}

void settings_reset(settings_t *settings, area_t *area)
{
    settings->margin = 0;
    settings->hover = -1;

    PS2MouseSetAccel(DEFAULT_ACCEL_TENTHS / 10.0f);
    PS2MouseSetThres(DEFAULT_THRESHOLD);
    PS2MouseSetDblClickTime(DEFAULT_DOUBLE_CLICK);
    write_boundary(settings, area);

    if (settings->guide_drawn)
        settings->erase_guide = 1;

    read_driver(settings);
}

void settings_init(settings_t *settings, area_t *area)
{
    settings->guide_drawn = 0;
    settings->erase_guide = 0;
    settings_reset(settings, area);
}

/* The text of a setting. */
static void format_setting(const settings_t *settings, int index, char *text, size_t size)
{
    switch (index) {
        case 0:
            snprintf(text, size, "Acceleration: %d.%d", settings->accel_tenths / 10, settings->accel_tenths % 10);
            break;
        case 1:
            if (settings->threshold >= DEFAULT_THRESHOLD)
                snprintf(text, size, "Threshold: off");
            else
                snprintf(text, size, "Threshold: %u", (unsigned int)settings->threshold);
            break;
        case 2:
            snprintf(text, size, "Double click time: %u ms", (unsigned int)settings->double_click_time);
            break;
        default:
            snprintf(text, size, "Boundary: %dx%d", settings->boundary_width, settings->boundary_height);
            break;
    }
}

/* The setting under the pointer, or -1. */
static int setting_at(const settings_t *settings, int x, int y)
{
    char text[TEXT_COLUMNS + 1];
    int i, line_y;

    for (i = 0; i < SETTINGS_COUNT; i++) {
        line_y = TEXT_Y + (SETTINGS_FIRST_LINE + i) * TEXT_LINE;
        format_setting(settings, i, text, sizeof(text));
        if (x >= TEXT_X && x < TEXT_X + (int)strlen(text) * 8 && y >= line_y - 2 && y < line_y + TEXT_LINE - 2)
            return i;
    }
    return -1;
}

/* Raise (1) or lower (-1) a setting. */
static void change(settings_t *settings, int index, int direction, area_t *area, int *area_changed)
{
    int i, nearest;
    u32 distance, best;

    switch (index) {
        case 0:
            PS2MouseSetAccel(clamp(settings->accel_tenths + direction * 5, ACCEL_MIN_TENTHS, ACCEL_MAX_TENTHS) / 10.0f);
            break;
        case 1:
            /* The threshold goes through the list, starting at the item that is nearest to the current one. */
            nearest = 0;
            best = 0xFFFFFFFF;
            for (i = 0; i < (int)THRESHOLD_COUNT; i++) {
                distance = (settings->threshold > thresholds[i]) ? settings->threshold - thresholds[i] : thresholds[i] - settings->threshold;
                if (distance < best) {
                    best = distance;
                    nearest = i;
                }
            }
            PS2MouseSetThres(thresholds[clamp(nearest + direction, 0, (int)THRESHOLD_COUNT - 1)]);
            break;
        case 2:
            PS2MouseSetDblClickTime(clamp((int)settings->double_click_time + direction * DOUBLE_CLICK_STEP, DOUBLE_CLICK_MIN, DOUBLE_CLICK_MAX));
            break;
        default:
            /* The boundary is the whole screen, or the screen without a margin. */
            settings->margin = (direction > 0);
            write_boundary(settings, area);
            *area_changed = 1;
            if (settings->guide_drawn && !settings->margin)
                settings->erase_guide = 1;
            break;
    }

    read_driver(settings);
}

int settings_update(settings_t *settings, const input_t *input, int active, area_t *area, int *area_changed)
{
    int direction = 0;
    int index = active ? setting_at(settings, input->x, input->y) : -1;
    int steps;

    settings->hover = index;
    if (index < 0)
        return 0;

    if (input->wheel > 0)
        direction = 1;
    else if (input->wheel < 0)
        direction = -1;
    if (input->pressed & PS2MOUSE_BTN1)
        direction = 1;
    if (input->pressed & PS2MOUSE_BTN2)
        direction = -1;

    if (direction != 0) {
        steps = (input->wheel != 0 && !(input->pressed & (PS2MOUSE_BTN1 | PS2MOUSE_BTN2))) ? clamp(input->wheel * direction, 1, 5) : 1;
        while (steps-- > 0)
            change(settings, index, direction, area, area_changed);
    }

    return 1;
}

void settings_print(settings_t *settings)
{
    char text[TEXT_COLUMNS + 1];
    int i;

    for (i = 0; i < SETTINGS_COUNT; i++) {
        format_setting(settings, i, text, sizeof(text));
        gfx_print_line(SETTINGS_FIRST_LINE + i, (i == settings->hover) ? GFX_HOVER : GFX_BLACK, "%s", text);
    }
}

/* A thin frame around the area of the pointer, drawn with four rectangles. */
static void frame(const area_t *area, u32 color)
{
    int right = area->right + CURSOR_SIZE;
    int bottom = area->bottom + CURSOR_SIZE;

    gfx_rect(area->left - 1, area->top - 1, right - area->left + 2, 1, color);
    gfx_rect(area->left - 1, bottom, right - area->left + 2, 1, color);
    gfx_rect(area->left - 1, area->top, 1, bottom - area->top, color);
    gfx_rect(right, area->top, 1, bottom - area->top, color);
}

void settings_erase_guide(settings_t *settings)
{
    area_t area;

    if (!settings->erase_guide)
        return;

    make_area(1, &area);
    frame(&area, GFX_BLACK);

    settings->erase_guide = 0;
    settings->guide_drawn = 0;
}

void settings_draw_guide(settings_t *settings, const area_t *area)
{
    if (!settings->margin)
        return;

    frame(area, GFX_GUIDE);
    settings->guide_drawn = 1;
}
