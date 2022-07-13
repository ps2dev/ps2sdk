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
#include <inttypes.h>
#include <timer.h>

int main(int argc, char *argv[])
{
    uint64_t previous;

    printf("\n\nStarting GetTimerSystemTime example!\n");
    previous = GetTimerSystemTime();
    
    while (1)
    {
        uint64_t current;

        current = GetTimerSystemTime();
        printf("Diff: %" PRIu64 " clocks\n", current - previous);
        previous = current;
    }
    return 0;
}