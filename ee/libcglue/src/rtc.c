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
 * Set real time clock-related variables based on the real time clock value in the Mechacon.
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "ps2sdkapi.h"
#define OSD_CONFIG_NO_LIBCDVD
#include "osd_config.h"
#include "timer_alarm.h"

// The definition for this function is located in ee/rpc/cdvd/src/scmd.c
extern time_t ps2time(time_t *t);

#ifdef F__libcglue_rtc_data
s64 _ps2sdk_rtc_offset_from_busclk = 0;
#endif

#ifdef F__libcglue_rtc_get_offset_from_busclk
s64 _libcglue_rtc_get_offset_from_busclk(void)
{
	return _ps2sdk_rtc_offset_from_busclk;
}

void _libcglue_rtc_update_impl()
{
	time_t rtc_sec;
	u32 busclock_sec;
	u32 busclock_usec;

	rtc_sec = ps2time(NULL);
	TimerBusClock2USec(GetTimerSystemTime(), &busclock_sec, &busclock_usec);

	_ps2sdk_rtc_offset_from_busclk = ((s64)rtc_sec) - ((s64)busclock_sec);
}
#endif

#ifdef F__libcglue_rtc_update
void __attribute__((weak)) _libcglue_rtc_update_impl();
__attribute__((weak))
void _libcglue_rtc_update()
{
	// cppcheck-suppress knownConditionTrueFalse
	if (&_libcglue_rtc_update_impl)
	{
		_libcglue_rtc_update_impl();
	}
}
#endif
