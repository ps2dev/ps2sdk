/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <atad.h>
#include <cdvdman.h>
#include <errno.h>
#include <intrman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thevent.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include "libapa.h"
#include "misc_hdsk.h"

void *AllocMemory(int size)
{
    int OldState;
    void *result;

    CpuSuspendIntr(&OldState);

    if ((result = AllocSysMemory(ALLOC_FIRST, size, NULL)) == NULL) {
        printf("hdsk: error: out of memory\n");
    }

    CpuResumeIntr(OldState);

    return result;
}

int HdskUnlockHdd(int unit)
{
    unsigned char id[32];
    int result;

    if ((result = apaGetIlinkID(id)) == 0) {
        result = ata_device_sce_sec_unlock(unit, id);
    }

    return result;
}

int HdskCreateEventFlag(void)
{
    iop_event_t EventFlagData;

    EventFlagData.attr = EA_MULTI;
    EventFlagData.bits = 0;
    return CreateEventFlag(&EventFlagData);
}

int HdskCreateThread(void (*function)(void *arg), int StackSize)
{
    iop_thread_t ThreadData;

    ThreadData.attr      = TH_C;
    ThreadData.thread    = function;
    ThreadData.priority  = 0x7b;
    ThreadData.stacksize = StackSize;
    return CreateThread(&ThreadData);
}
