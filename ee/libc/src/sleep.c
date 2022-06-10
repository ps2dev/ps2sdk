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
#include <errno.h>
#include <time.h>
#include <timer_alarm.h>

#ifdef F_nanosleep
static inline u64 timespec_to_nanosecs(const struct timespec *pts) {
    return (pts->tv_sec * 1000000000ULL) + pts->tv_nsec;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    u64 nano_secs, cycles;

    nano_secs = timespec_to_nanosecs(req);
    cycles = NSecToCycles(nano_secs);
    ThreadWaitClock(cycles);

    if (rem != NULL) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

	return 0;
}

#endif
