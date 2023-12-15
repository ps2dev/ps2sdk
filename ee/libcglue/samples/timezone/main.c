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
#include <time.h>
#include <unistd.h>
#include <kernel.h>

#if defined(SCREEN_DEBUG)
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
#if defined(SCREEN_DEBUG)
    init_scr();
    sleep(3);
#endif
    custom_printf("\n\nStarting Timezone test...\n");
 
    // Get the value of the TZ environment variable
    char *time_zone = getenv("TZ");

    if (time_zone != NULL) {
        custom_printf("Current time zone: %s\n", time_zone);
    } else {
        perror("Error getting time zone");
        return 1;
    }

    while (1) {
        // Clear the screen part where the time is printed
        scr_setXY(0, 4);
        scr_clearline(4);

        // Get the current time
        time_t raw_time;
        struct tm *time_info;

        time(&raw_time);
        time_info = localtime(&raw_time);

        if (time_info == NULL) {
            perror("Error getting local time");
            return 1;
        }

        // Print the current hour
        custom_printf("Current hour: %02d:%02d:%02d\n",
               time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

        // Refresh every second
        sleep(1);
        // You can use usleep(1000000) for more precise intervals if needed
    }

    SleepThread();

    return 0;
}