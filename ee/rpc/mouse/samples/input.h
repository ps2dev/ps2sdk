/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB mouse sample: what the mouse did in one frame.
*/

#ifndef INPUT_H
#define INPUT_H

#include <tamtypes.h>
#include <libmouse.h>

/* The mouse moves this many times slower than its raw counts, 1 would be the raw speed. */
#define POINTER_SLOWDOWN 3

/* A button held for this many frames (half a second) is a long press. */
#define LONG_PRESS_FRAMES 30

/* The area where the top left corner of the pointer can be. */
typedef struct
{
    int left, top, right, bottom;
} area_t;

/* The state of the mouse in one frame, read by main() and given to the other files. */
typedef struct
{
    /* Position of the pointer. */
    int x, y;
    /* Steps of the wheel since the last frame, positive when it is moved up. */
    int wheel;
    /* Buttons that are down (PS2MOUSE_BTN1 ...) and double clicks of this frame (PS2MOUSE_BTN1DBL ...). */
    u32 buttons;
    /* Buttons that were pressed or released in this frame. */
    u32 pressed;
    u32 released;
    /* Frames that each button (1, 2 and 3) is down. In the frame that it is released, how long it was down. */
    int hold[3];
} input_t;

static inline int clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

#endif
