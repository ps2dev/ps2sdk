/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <intrman.h>
#include <romdrv.h>
#include <ssbusc.h>
#include <sysmem.h>
#include <loadcore.h>

IRX_ID("Additional_ROM_Driver_2", 1, 1);
// Mainly based on the module from ROM 1.8.0cd.

int _start(int argc, char **argv)
{
    int BaseAddress;
    int Delay;
    int state;

    (void)argc;
    (void)argv;
    CpuSuspendIntr(&state);
    BaseAddress = GetBaseAddress(1);
    Delay       = GetDelay(1);
    SetBaseAddress(1, 0xBE000000);
    SetDelay(1, 0x183444);
    if (romAddDevice(2, (void *)0xBE400000) != 0) {
        Kprintf("ROM directory not found\n");
        SetDelay(1, Delay);
        SetBaseAddress(1, BaseAddress);
    }
    CpuResumeIntr(state);
    return MODULE_NO_RESIDENT_END;
}
