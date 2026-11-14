/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: the menu of sizes that opens where the pointer is.
*/

#ifndef MENU_H
#define MENU_H

#include "input.h"

typedef struct
{
    int open;
    /* Top left corner. */
    int x, y;
    /* The item under the pointer, or reached with the wheel. -1 if none. */
    int hover;
    int pointer_x, pointer_y;
    /* Where the menu was drawn the last time, to erase it. */
    int drawn;
    int old_x, old_y;
} menu_t;

void menu_init(menu_t *menu);

/* The menu opens with its top left corner on the pointer, or as near as the screen allows. */
void menu_open(menu_t *menu, int pointer_x, int pointer_y);
void menu_close(menu_t *menu);
int menu_is_open(const menu_t *menu);

/*
 * The pointer and the wheel choose an item, button 1 takes it. A click anywhere else closes the
 * menu. Returns 1 when a size was chosen, and the menu is closed.
 */
int menu_update(menu_t *menu, const input_t *input, int *size);

/* Between gfx_begin() and gfx_end(). */
void menu_erase(menu_t *menu);

/* The menu draws its own rectangles and text, so it is not between gfx_begin() and gfx_end(). */
void menu_draw(menu_t *menu, int current_size);

#endif
