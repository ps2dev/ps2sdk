/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <kernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <timer.h>

int main(int argc, char **argv)
{
    uint64_t current, previous;

    printf("\n\nStarting ps2_clock example!\n");
    previous = GetTimerSystemTime();
    
    while (1)
    {
        current = GetTimerSystemTime();
        printf("Diff: %llu clocks\n", current - previous);
        previous = current;
    }
    return 0;
}