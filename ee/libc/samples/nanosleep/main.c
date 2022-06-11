/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Malloc tester
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <time.h>

#if defined(SCREEN_DEBUG)
#include <unistd.h>
#include <debug.h>
#endif

#if defined(SCREEN_DEBUG)
#define custom_printf(args...) \
    printf(args);              \
    scr_printf(args);
#else
#define custom_printf(args...) printf(args);
#endif

int main(int argc, char *argv[])
{
    int second_waited = 0;
    clock_t start, diff, diff_error, error_tolerance;
    struct timespec tv = {0};
    tv.tv_sec = 1;
    tv.tv_nsec = 0;
    error_tolerance = 5; // 5 miliseconds of error tolerance

#if defined(SCREEN_DEBUG)
    init_scr();
    sleep(3);
#endif
    custom_printf("\n\nStarting NANOSLEEP TESTS...\n");

    start = clock();
    while(tv.tv_sec <= 10) {
        custom_printf("Waiting %lli second...\n", tv.tv_sec);
        nanosleep(&tv, NULL);
        second_waited += tv.tv_sec;
        tv.tv_sec++;
    }
    diff = (clock() - start);
    diff_error = (second_waited * 1000) - diff;

    custom_printf("Checking if we have waited %i seconds...\n", second_waited);
    custom_printf("We have waited: %lu milliseconds\n", diff);

    if (abs(diff_error)  < error_tolerance) {
        custom_printf("nanosecond looks to works properly\n");
    } else {
        custom_printf("nanosecond is not accurate there is a difference of %lu milliseconds \n", diff_error);
    }

    SleepThread();

    return 0;
}
