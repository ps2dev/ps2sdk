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

int HdskReadClock(apa_ps2time_t *time)
{
    static apa_ps2time_t CurrentTime = {0, 7, 6, 5, 4, 3, 2000};
    sceCdCLOCK CdClock;

    if (sceCdReadClock(&CdClock) != 0) {
        CurrentTime.sec   = btoi(CdClock.second);
        CurrentTime.min   = btoi(CdClock.minute);
        CurrentTime.hour  = btoi(CdClock.hour);
        CurrentTime.day   = btoi(CdClock.day);
        CurrentTime.month = btoi(CdClock.month & 0x7F); // NOTE: the newer CDVDMAN module automatically file off the highest bit of this field.
        CurrentTime.year  = btoi(CdClock.year) + 2000;
    }
    memcpy(time, &CurrentTime, sizeof(apa_ps2time_t));

    return 0;
}

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

int HdskRI(unsigned char *id)
{
    u32 stat;
    int result;

    memset(id, 0, 32);
    stat = 0;
    if (sceCdRI(id, &stat) == 0 || stat) {
        printf("hdsk: error: cannot get ilink id\n");
        result = -EIO;
    } else {
        result = 0;
    }

    return result;
}

int HdskUnlockHdd(int unit)
{
    unsigned char id[32];
    int result;

    if ((result = HdskRI(id)) == 0) {
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
