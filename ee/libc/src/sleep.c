/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * sleep implementation
 */

#include <unistd.h>
#include <kernel.h>
#include <time.h>

#define HSYNC_COUNT 600

struct sleep_data
{
    s32	 s;
    clock_t wait;
};

static void _sleep_waker(s32 alarm_id, u16 time, void *arg2)
{
    struct sleep_data *sd = (struct sleep_data *) arg2;
    if (clock() >= sd->wait)
        iSignalSema(sd->s);
    else
        iSetAlarm(HSYNC_COUNT, _sleep_waker, arg2);

    ExitHandler();
}

unsigned int sleep(unsigned int seconds)
{
    ee_sema_t sema;
    struct sleep_data sd;

	sema.init_count = 0;
	sema.max_count  = 1;
	sema.option     = 0;

    sd.wait = clock() + seconds * CLOCKS_PER_SEC;
    sd.s = CreateSema(&sema);
    SetAlarm(HSYNC_COUNT, _sleep_waker, &sd);
    WaitSema(sd.s);
    DeleteSema(sd.s);

	return 0;
}
