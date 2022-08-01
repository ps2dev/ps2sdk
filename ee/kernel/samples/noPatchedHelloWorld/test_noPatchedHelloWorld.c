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

// Disable all the patched functions
DISABLE_PATCHED_FUNCTIONS();

int main(int argc, char *argv[])
{    
    while (1)
    {
        printf("Hello World from no patch kernel!\n");
    }
    return 0;
}