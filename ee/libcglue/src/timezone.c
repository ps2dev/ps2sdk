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
#include <reent.h>

#include <ps2sdkapi.h>
#define OSD_CONFIG_NO_LIBCDVD
#include "osd_config.h"

#ifdef F__libcglue_timezone_update_impl
void _libcglue_timezone_update_impl()
{
    /* Initialize timezone from PS2 OSD configuration */
	int tzOffset = configGetTimezone();
    int tzOffsetAbs = tzOffset < 0 ? -tzOffset : tzOffset;
    int hours = tzOffsetAbs / 60;
    int minutes = tzOffsetAbs - hours * 60;
    int daylight = configIsDaylightSavingEnabled();
    static char tz[28];
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-overflow"
    snprintf(tz, sizeof(tz), "GMT%s%02i:%02i%s", tzOffset < 0 ? "+" : "-", hours, minutes, daylight ? "DST" : "");
	#pragma GCC diagnostic pop
    setenv("TZ", tz, 1);
}
#endif

#ifdef F__libcglue_timezone_update
// Defined in newlib: newlib/libc/time/tzset_r.c
void __attribute((weak)) _tzset_unlocked_r(struct _reent *reent_ptr);
__attribute__((weak))
void _libcglue_timezone_update()
{
	// cppcheck-suppress knownConditionTrueFalse
	if (&_tzset_unlocked_r)
	{
		_libcglue_timezone_update_impl();
	}
}
#endif

#ifdef F_ps2sdk_setTimezone
void ps2sdk_setTimezone(int timezone) {
	configSetTimezone(timezone);
	_libcglue_timezone_update();
}
#endif

#ifdef F_ps2sdk_setDaylightSaving
void ps2sdk_setDaylightSaving(int daylightSaving) {
	configSetDaylightSavingEnabled(daylightSaving);
	_libcglue_timezone_update();
}
#endif