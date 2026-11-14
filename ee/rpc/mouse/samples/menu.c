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

#include <libmouse.h>

#include "gfx.h"
#include "menu.h"
#include "square.h"

#define MENU_ITEMS  SQUARE_SIZE_COUNT
#define MENU_PAD    3
#define MENU_ROW    12
#define MENU_ITEM_W 40
#define MENU_WIDTH  (MENU_ITEM_W + 2 * MENU_PAD)
#define MENU_HEIGHT (MENU_ITEMS * MENU_ROW + 2 * MENU_PAD)

static int item_at(const menu_t *menu, int x, int y)
{
    int row = y - menu->y - MENU_PAD;

    if (x < menu->x + MENU_PAD || x >= menu->x + MENU_PAD + MENU_ITEM_W || row < 0 || row >= MENU_ITEMS * MENU_ROW)
        return -1;
    return row / MENU_ROW;
}

static int is_inside(const menu_t *menu, int x, int y)
{
    return x >= menu->x && x < menu->x + MENU_WIDTH && y >= menu->y && y < menu->y + MENU_HEIGHT;
}

void menu_init(menu_t *menu)
{
    menu->open = 0;
    menu->hover = -1;
    menu->drawn = 0;
}

void menu_open(menu_t *menu, int pointer_x, int pointer_y)
{
    menu->open = 1;
    menu->x = clamp(pointer_x, 0, SCREEN_WIDTH - MENU_WIDTH);
    menu->y = clamp(pointer_y, 0, screen_height - MENU_HEIGHT);
    menu->hover = item_at(menu, pointer_x, pointer_y);
    menu->pointer_x = pointer_x;
    menu->pointer_y = pointer_y;
}

void menu_close(menu_t *menu)
{
    menu->open = 0;
    menu->hover = -1;
}

int menu_is_open(const menu_t *menu)
{
    return menu->open;
}

int menu_update(menu_t *menu, const input_t *input, int *size)
{
    /* The item under the pointer is the one that is marked while the pointer moves. */
    if (input->x != menu->pointer_x || input->y != menu->pointer_y) {
        menu->hover = item_at(menu, input->x, input->y);
        menu->pointer_x = input->x;
        menu->pointer_y = input->y;
    }

    /* The wheel moves the mark up and down the items. */
    if (input->wheel != 0) {
        /* Without a mark, the first step of the wheel marks the top item. */
        if (menu->hover < 0)
            menu->hover = (input->wheel > 0) ? input->wheel : -1;
        menu->hover = clamp(menu->hover - input->wheel, 0, MENU_ITEMS - 1);
    }

    if (input->pressed & (PS2MOUSE_BTN1 | PS2MOUSE_BTN2)) {
        if (!is_inside(menu, input->x, input->y)) {
            menu_close(menu);
        } else if ((input->pressed & PS2MOUSE_BTN1) && menu->hover >= 0) {
            *size = square_sizes[menu->hover];
            menu_close(menu);
            return 1;
        }
    }

    return 0;
}

void menu_erase(menu_t *menu)
{
    if (menu->drawn)
        gfx_rect(menu->old_x, menu->old_y, MENU_WIDTH, MENU_HEIGHT, GFX_BLACK);
    menu->drawn = 0;
}

void menu_draw(menu_t *menu, int current_size)
{
    int selected = square_nearest_size(current_size);
    int i, row_y;
    u32 background;

    if (!menu->open)
        return;

    /* White frame, black inside, and a bar behind the selected item and behind the one that is marked. */
    gfx_begin();
    gfx_rect(menu->x, menu->y, MENU_WIDTH, MENU_HEIGHT, GFX_WHITE);
    gfx_rect(menu->x + 1, menu->y + 1, MENU_WIDTH - 2, MENU_HEIGHT - 2, GFX_BLACK);
    for (i = 0; i < MENU_ITEMS; i++) {
        row_y = menu->y + MENU_PAD + i * MENU_ROW;
        if (i == selected)
            gfx_rect(menu->x + MENU_PAD, row_y, MENU_ITEM_W, MENU_ROW, GFX_SELECTED);
        else if (i == menu->hover)
            gfx_rect(menu->x + MENU_PAD, row_y, MENU_ITEM_W, MENU_ROW, GFX_HOVER);
    }
    gfx_end();

    for (i = 0; i < MENU_ITEMS; i++) {
        row_y = menu->y + MENU_PAD + i * MENU_ROW;
        background = (i == selected) ? GFX_SELECTED : (i == menu->hover) ? GFX_HOVER : GFX_BLACK;
        gfx_text(menu->x + MENU_PAD, row_y + (MENU_ROW - 8) / 2, GFX_WHITE, background, " %3d ", square_sizes[i]);
    }

    menu->drawn = 1;
    menu->old_x = menu->x;
    menu->old_y = menu->y;
}
