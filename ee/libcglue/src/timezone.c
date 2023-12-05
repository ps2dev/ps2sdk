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

#include <ps2sdkapi.h>
#define OSD_CONFIG_NO_LIBCDVD
#include "osd_config.h"

__attribute__((weak))
void _libcglue_timezone_update()
{
    /* Initialize timezone from PS2 OSD configuration */
    int tzOffset = 0;
	
	// Set ps2sdk functions
	if (_ps2sdk_open == NULL) _set_ps2sdk_open();
	if (_ps2sdk_close == NULL) _set_ps2sdk_close();
	if (_ps2sdk_read == NULL) _set_ps2sdk_read();

	_io_driver driver = { _ps2sdk_open, _ps2sdk_close, _ps2sdk_read };
	configGetTimezoneWithIODriver(&driver);
    int tzOffsetAbs = tzOffset < 0 ? -tzOffset : tzOffset;
    int hours = tzOffsetAbs / 60;
    int minutes = tzOffsetAbs - hours * 60;
    int daylight = configIsDaylightSavingEnabledWithIODriver(&driver);
    static char tz[15];
    sprintf(tz, "GMT%s%02i:%02i%s", tzOffset < 0 ? "+" : "-", hours, minutes, daylight ? "DST" : "");
    setenv("TZ", tz, 1);
}
