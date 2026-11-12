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

#ifndef SQUARE_H
#define SQUARE_H

#include "input.h"

#define SQUARE_SIZE        64
#define SQUARE_MIN_SIZE    2
#define SQUARE_MAX_SIZE    128
#define SQUARE_SIZE_COUNT  7

/* The sizes that the menu offers and that holding button 2 goes through, from the biggest to the smallest. */
extern const int square_sizes[SQUARE_SIZE_COUNT];

/* The item of square_sizes that is nearest to the given size. */
int square_nearest_size(int size);

typedef struct
{
    int x, y, size;
    int is_blue;
    /* Button 1 is holding the square, and where in the square. */
    int dragging;
    int grab_x, grab_y;
    /* Button 2 was pressed on the square, and it must not open the menu when it is released. */
    int right_down;
    int ignore_release;
    /* While button 2 is held the size goes through square_sizes: toward the smallest (1) or the biggest (-1). */
    int cycle_direction;
    /* Where the square was drawn the last time, to erase it. */
    int old_x, old_y, old_size;
} square_t;

/* The square in the center of the screen, with its original size and color. */
void square_init(square_t *square);

/* The same, but the old square is still erased. */
void square_reset(square_t *square);

/* Stop what the buttons were doing, for when the mouse is unplugged. */
void square_cancel(square_t *square);

int square_contains(const square_t *square, int x, int y);

/* The square is busy while it is dragged or while button 2 is held on it. */
int square_is_busy(const square_t *square);

/* Change the size, the center of the square stays. */
void square_set_size(square_t *square, int size);

/* Button 2 was double clicked on the square: it gets its original size. */
void square_double_click_button2(square_t *square);

/*
 * Button 1: changes the color, holding it drags the square, a double click puts it back in
 * the center.
 * Button 2: a click asks for the menu of sizes (open_menu), holding it goes through the sizes
 * of the menu, up and down, while the pointer is on the square.
 * Wheel: changes the size.
 * Returns 1 if the pointer is on the square.
 */
int square_update(square_t *square, const input_t *input, int *open_menu);

/* Between gfx_begin() and gfx_end(). */
void square_erase(const square_t *square);
void square_draw(square_t *square);

#endif
