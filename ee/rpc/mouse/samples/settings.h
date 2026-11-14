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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "input.h"

/* Number of settings, and the line of the text where the first one is. */
#define SETTINGS_COUNT      4
#define SETTINGS_FIRST_LINE 9

typedef struct
{
    /* What the driver says, read after each change. */
    int accel_tenths;
    u32 threshold;
    u32 double_click_time;
    int boundary_width, boundary_height;
    /* The pointer stays away from the borders of the screen. */
    int margin;
    /* The setting under the pointer, or -1. */
    int hover;
    /* The frame that shows the margin is on the screen, or it has to be erased. */
    int guide_drawn;
    int erase_guide;
} settings_t;

/* Sets the driver to the original settings and gives the area of the pointer. */
void settings_init(settings_t *settings, area_t *area);

/* The same, for the reset of everything. */
void settings_reset(settings_t *settings, area_t *area);

/*
 * Acceleration, threshold, double click time and boundary. The wheel changes the setting that is
 * under the pointer, button 1 raises it and button 2 lowers it.
 * Only when active, the mouse can change them. Returns 1 if the pointer is on a setting.
 * When the boundary changes, area_changed is set to 1 and area has the new area.
 */
int settings_update(settings_t *settings, const input_t *input, int active, area_t *area, int *area_changed);

/* The text of the settings. */
void settings_print(settings_t *settings);

/* Between gfx_begin() and gfx_end(). */
void settings_erase_guide(settings_t *settings);
void settings_draw_guide(settings_t *settings, const area_t *area);

#endif
