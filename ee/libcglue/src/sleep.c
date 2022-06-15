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

#include <kernel.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <timer_alarm.h>

#ifdef F_nanosleep

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    uint64_t cycles;

    cycles = TimerUSec2BusClock(req->tv_sec, req->tv_nsec / 1000);
    ThreadWaitClock(cycles);

    if (rem != NULL) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

	return 0;
}

#endif
