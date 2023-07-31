/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <thbase.h>
#include <thevent.h>

#include "pfs-opt.h"
#include "libpfs.h"
#include "misc_fssk.h"

int fsskCreateEventFlag(void)
{
    iop_event_t EventFlagData;

    EventFlagData.attr = EA_MULTI;
    EventFlagData.bits = 0;
    return CreateEventFlag(&EventFlagData);
}

int fsskCreateThread(void (*function)(void *arg), int StackSize)
{
    iop_thread_t ThreadData;

    ThreadData.attr      = TH_C;
    ThreadData.thread    = function;
    ThreadData.priority  = 0x7b;
    ThreadData.stacksize = StackSize;
    return CreateThread(&ThreadData);
}
