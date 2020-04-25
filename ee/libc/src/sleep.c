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

#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "ps2sdkapi.h"

#define MIN_HSYNC_PER_SEC (240*50)
#define MIN_HSYNC_DELAY 100
#define MAX_PS2CLOCK_PER_HSYNC (PS2_CLOCKS_PER_SEC/MIN_HSYNC_PER_SEC)

// Start with the highest possible value (so we sleep too short)
static unsigned int iPS2ClockPerHSync = MAX_PS2CLOCK_PER_HSYNC;

static void _sleep_waker(s32 alarm_id, u16 time, void *arg2)
{
    s32 *pSema = (s32 *)arg2;
    iSignalSema(*pSema);
    ExitHandler();
}

static inline u64 timespec_to_us(const struct timespec *pts) {
    return (pts->tv_sec * 1000000ULL) + (pts->tv_nsec / 1000ULL);
}

static inline clock_t us_to_ps2_clock(u64 us) {
    return (us * PS2_CLOCKS_PER_MSEC) / 1000;
}

static inline clock_t timespec_to_ps2_clock(const struct timespec *pts) {
    return us_to_ps2_clock(timespec_to_us(pts));
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    ee_sema_t sema;
    s32 sema_id;
    clock_t clock_delay, clock_end, clock_real_end;
    u16 hsync_delay;

    clock_delay = timespec_to_ps2_clock(req);
    clock_end   = ps2_clock() + clock_delay;
    hsync_delay = clock_delay / iPS2ClockPerHSync;

    if (hsync_delay >= MIN_HSYNC_DELAY) {
        sema.init_count = 0;
        sema.max_count  = 1;
        sema.option     = 0;
        sema_id = CreateSema(&sema);

        SetAlarm(hsync_delay, _sleep_waker, &sema_id);
        WaitSema(sema_id);
        DeleteSema(sema_id);

        // Correct the HSync delay
        // NOTE: We're always trying to end btween 0 and 1 HSyncs too short
        clock_real_end = ps2_clock();
        if (clock_real_end < (clock_end - iPS2ClockPerHSync)) {
            // We sleeped too short, so HSyncs must be faster
            // Correct the HSyncs for next time
            iPS2ClockPerHSync -= (clock_end - iPS2ClockPerHSync - clock_real_end) / hsync_delay;
        }
        else if (clock_real_end > clock_end) {
            // We sleeped too long, so HSyncs must be slower
            // Correct the HSyncs for next time
            iPS2ClockPerHSync += (clock_real_end - clock_end) / (hsync_delay-1);
        }
    }

    // Delay whatever time is left
    while(clock_end > ps2_clock())
        ;

    if (rem != NULL) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

	return 0;
}

unsigned int sleep(unsigned int seconds)
{
    struct timespec ts;

    ts.tv_sec = seconds;
    ts.tv_nsec = 0;

    if (!nanosleep(&ts, &ts))
        return 0;

    if (errno == EINTR)
        return ts.tv_sec;

    return -1;
}
