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
#include <stdint.h>
#include <stdio.h>
#include <ps2sdkapi.h>

// Disable all the patched functions
DISABLE_PATCHED_FUNCTIONS();
// Disable extra timer functionality
DISABLE_TimerSystemTime();
// Disable pthread functionality
PS2_DISABLE_AUTOSTART_PTHREAD();

int main(int argc, char *argv[])
{    
    while (1)
    {
        printf("Hello World from a nano binary!\n");
    }
    return 0;
}