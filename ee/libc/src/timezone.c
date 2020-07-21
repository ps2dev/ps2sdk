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
#define OSD_CONFIG_NO_LIBCDVD
#include "osd_config.h"

static char _ps2sdk_tzname[11];

__attribute__((weak))
void _ps2sdk_timezone_update()
{
	// Set TZ and call tzset to ensure that timezone information won't get overwritten when tszet is called multiple times
	setenv("TZ", "", 0);
	tzset();

	__tzinfo_type *tz = __gettzinfo();

	// _timezone is in seconds, while the return value of configGetTimezone is in minutes
	// Add one hour if configIsDaylightSavingEnabled is 1
	_timezone = (configGetTimezone() + (configIsDaylightSavingEnabled() * 60)) * 60;
	tz->__tzrule[0].offset = _timezone;
	snprintf(_ps2sdk_tzname, sizeof(_ps2sdk_tzname), "Etc/GMT%+ld", _timezone);
	_tzname[0] = _ps2sdk_tzname;
	_tzname[1] = _ps2sdk_tzname;

	// Don't perform DST conversion
	_daylight = 0;
}
