/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Set timezone-related variables based on the OSD configuration.
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <ps2sdkapi.h>
#define OSD_CONFIG_NO_LIBCDVD
#include "osd_config.h"

#define posixIODriver { open, close, (int (*)(int, void *, int))read, O_RDONLY }

#ifdef F__libcglue_timezone_update
__attribute__((weak))
void _libcglue_timezone_update()
{
    /* Initialize timezone from PS2 OSD configuration */
	_io_driver driver = posixIODriver;
	int tzOffset = configGetTimezoneWithIODriver(&driver);
    int tzOffsetAbs = tzOffset < 0 ? -tzOffset : tzOffset;
    int hours = tzOffsetAbs / 60;
    int minutes = tzOffsetAbs - hours * 60;
    int daylight = configIsDaylightSavingEnabledWithIODriver(&driver);
    static char tz[15];
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-overflow"
    sprintf(tz, "GMT%s%02i:%02i%s", tzOffset < 0 ? "+" : "-", hours, minutes, daylight ? "DST" : "");
	#pragma GCC diagnostic pop
    setenv("TZ", tz, 1);
}
#endif

#ifdef F_ps2sdk_setTimezone
void ps2sdk_setTimezone(int timezone) {
	_io_driver driver = posixIODriver;
	configSetTimezoneWithIODriver(timezone, &driver, _libcglue_timezone_update);
}
#endif

#ifdef F_ps2sdk_setDaylightSaving
void ps2sdk_setDaylightSaving(int daylightSaving) {
	_io_driver driver = posixIODriver;
	configSetDaylightSavingEnabledWithIODriver(daylightSaving, &driver, _libcglue_timezone_update);
}
#endif