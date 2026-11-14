/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: the square that the buttons and the wheel of the mouse act on.
*/

#include <libmouse.h>

#include "gfx.h"
#include "square.h"

/* Pixels that the size changes for each step of the wheel. */
#define WHEEL_STEP 4

/* Frames between two sizes, while button 2 is held. */
#define CYCLE_FRAMES 20

const int square_sizes[SQUARE_SIZE_COUNT] = {128, 64, 32, 16, 8, 4, 2};

int square_nearest_size(int size)
{
    int best = 0;
    int i, distance, best_distance = 1000;

    for (i = 0; i < SQUARE_SIZE_COUNT; i++) {
        distance = (size > square_sizes[i]) ? size - square_sizes[i] : square_sizes[i] - size;
        if (distance < best_distance) {
            best = i;
            best_distance = distance;
        }
    }
    return best;
}

static void keep_inside_screen(square_t *square)
{
    square->x = clamp(square->x, 0, SCREEN_WIDTH - square->size);
    square->y = clamp(square->y, 0, screen_height - square->size);
}

static void go_to_center(square_t *square)
{
    square->x = (SCREEN_WIDTH - square->size) / 2;
    square->y = (screen_height - square->size) / 2;
}

void square_reset(square_t *square)
{
    square->size = SQUARE_SIZE;
    square->is_blue = 0;
    square_cancel(square);
    go_to_center(square);
}

void square_init(square_t *square)
{
    square_reset(square);
    square->old_x = square->x;
    square->old_y = square->y;
    square->old_size = square->size;
}

void square_cancel(square_t *square)
{
    square->dragging = 0;
    square->right_down = 0;
    square->ignore_release = 0;
    square->cycle_direction = 1;
}

int square_contains(const square_t *square, int x, int y)
{
    return x >= square->x && x < square->x + square->size && y >= square->y && y < square->y + square->size;
}

int square_is_busy(const square_t *square)
{
    return square->dragging || square->right_down;
}

void square_set_size(square_t *square, int size)
{
    int center_x = square->x + square->size / 2;
    int center_y = square->y + square->size / 2;

    square->size = clamp(size, SQUARE_MIN_SIZE, SQUARE_MAX_SIZE);
    square->x = center_x - square->size / 2;
    square->y = center_y - square->size / 2;
    keep_inside_screen(square);
}

void square_double_click_button2(square_t *square)
{
    square_set_size(square, SQUARE_SIZE);
    square->right_down = 1;
    square->ignore_release = 1;
    square->cycle_direction = 1;
}

int square_update(square_t *square, const input_t *input, int *open_menu)
{
    int on_square = square_contains(square, input->x, input->y);

    /* Button 1 changes the color and holds the square, to drag it. */
    if ((input->pressed & PS2MOUSE_BTN1) && on_square) {
        square->is_blue = !square->is_blue;
        square->dragging = 1;
        square->grab_x = input->x - square->x;
        square->grab_y = input->y - square->y;
    }
    if (!(input->buttons & PS2MOUSE_BTN1))
        square->dragging = 0;

    /* A double click of button 1 puts the square back in the center. */
    if ((input->buttons & PS2MOUSE_BTN1DBL) && on_square) {
        square->dragging = 0;
        go_to_center(square);
    }

    if (square->dragging) {
        square->x = input->x - square->grab_x;
        square->y = input->y - square->grab_y;
    }

    /*
     * Button 2: a click asks for the menu of sizes when it is released. Holding it goes through the
     * sizes of the menu, from the one that is nearest to the current size to the smallest, then to
     * the biggest, and so on. It stops while the pointer is not on the square.
     */
    if ((input->pressed & PS2MOUSE_BTN2) && on_square) {
        square->right_down = 1;
        square->ignore_release = 0;
        square->cycle_direction = 1;
    }
    if (square->right_down) {
        if ((input->buttons & PS2MOUSE_BTN2) && on_square && input->hold[1] >= LONG_PRESS_FRAMES &&
            (input->hold[1] - LONG_PRESS_FRAMES) % CYCLE_FRAMES == 0) {
            int item = square_nearest_size(square->size);

            if (item + square->cycle_direction < 0 || item + square->cycle_direction >= SQUARE_SIZE_COUNT)
                square->cycle_direction = -square->cycle_direction;
            item = clamp(item + square->cycle_direction, 0, SQUARE_SIZE_COUNT - 1);
            square_set_size(square, square_sizes[item]);
        }
        if (input->released & PS2MOUSE_BTN2) {
            if (!square->ignore_release && input->hold[1] < LONG_PRESS_FRAMES)
                *open_menu = 1;
            square->right_down = 0;
            square->ignore_release = 0;
        }
    }

    /* The wheel changes the size while the pointer is on the square, and stops when it is not. */
    if (input->wheel != 0 && on_square && !square->dragging && !square->right_down)
        square_set_size(square, square->size + input->wheel * WHEEL_STEP);

    keep_inside_screen(square);

    return on_square;
}

void square_erase(const square_t *square)
{
    gfx_rect(square->old_x, square->old_y, square->old_size, square->old_size, GFX_BLACK);
}

void square_draw(square_t *square)
{
    gfx_rect(square->x, square->y, square->size, square->size, square->is_blue ? GFX_BLUE : GFX_RED);

    square->old_x = square->x;
    square->old_y = square->y;
    square->old_size = square->size;
}
