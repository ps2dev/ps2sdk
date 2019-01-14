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
 * PS2 Configuration settings
 * note: the 'set' methods are only valid till the ps2 gets
 * turned off or reset!
 */

#include <tamtypes.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>
#include <osd_config.h>

/** config param data as stored on a DTL-T10000(H) TOOL */
typedef struct {
	u16 timezoneOffset;
	u8  screenType;
	u8  dateFormat;
	u8  language;
	u8  spdifMode;
	u8  daylightSaving;
	u8  timeFormat;
} ConfigParamT10K;

extern ConfigParamT10K g_t10KConfig;
extern char g_RomName[];

#ifdef F__config_internals
ConfigParamT10K g_t10KConfig = {540, TV_SCREEN_43, DATE_YYYYMMDD, LANGUAGE_JAPANESE, 0, 0, 0};

/** stores romname of ps2 */
char g_RomName[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#ifdef F_GetRomName
char* GetRomName(char *romname)
{
	int fd;

	fd = _ps2sdk_open("rom0:ROMVER", O_RDONLY);
	_ps2sdk_read(fd, romname, 14);
	_ps2sdk_close(fd);
	return romname;
}
#endif

#ifdef F_IsT10K
int IsT10K(void)
{
	// only read in the romname the first time
	if(g_RomName[0] == 0)
		GetRomName(g_RomName);
	return (g_RomName[4] == 'T') ? 1 : 0;
}
#endif

#ifdef F_IsEarlyJap
int IsEarlyJap(ConfigParam config)
{
	return config.version == 0;
}
#endif

#ifdef F_configGetLanguage
int  configGetLanguage(void)
{
	ConfigParam config;

	if(IsT10K())
		return g_t10KConfig.language;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return config.japLanguage;
	return config.language;
}
#endif

#ifdef F_configSetLanguage
void configSetLanguage(int language)
{
	ConfigParam config;

	// make sure language is valid
	if(language < LANGUAGE_JAPANESE || language > LANGUAGE_PORTUGUESE)
		return;
	if(IsT10K())
		g_t10KConfig.language = language;

	// set language
	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		config.japLanguage = language;
	else
		config.language = language;
	SetOsdConfigParam(&config);
}
#endif

#ifdef F_configGetTvScreenType
int  configGetTvScreenType(void)
{
	ConfigParam config;

	if(IsT10K())
		return g_t10KConfig.screenType;

	GetOsdConfigParam(&config);
	return config.screenType;
}
#endif

#ifdef F_configSetTvScreenType
void configSetTvScreenType(int screenType)
{
	ConfigParam config;

	// make sure screen type is valid
	if(screenType < TV_SCREEN_43 || screenType > TV_SCREEN_169)
		return;
	if(IsT10K())
		g_t10KConfig.screenType = screenType;

	// set screen type
	GetOsdConfigParam(&config);
	config.screenType = screenType;
	SetOsdConfigParam(&config);
}
#endif

#ifdef F_configGetDateFormat
int  configGetDateFormat(void)
{
	ConfigParam config;
	Config2Param config2;

	if(IsT10K())
		return g_t10KConfig.dateFormat;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return 0;
	GetOsdConfigParam2(&config2, 1, 1);
	return config2.dateFormat;
}
#endif

#ifdef F_configSetDateFormat
void configSetDateFormat(int dateFormat)
{
	ConfigParam config;
	Config2Param config2;

	// make sure date format is valid
	if(dateFormat < DATE_YYYYMMDD || dateFormat > DATE_DDMMYYYY)
		return;
	if(IsT10K())
		g_t10KConfig.dateFormat = dateFormat;

	// set date format
	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return;
	GetOsdConfigParam2(&config2, 1, 1);
	config2.dateFormat = dateFormat;
	SetOsdConfigParam2(&config2, 1, 1);
}
#endif

#ifdef F_configGetTimeFormat
int  configGetTimeFormat(void)
{
	ConfigParam config;
	Config2Param config2;

	if(IsT10K())
		return g_t10KConfig.timeFormat;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return 0;
	GetOsdConfigParam2(&config2, 1, 1);
	return config2.timeFormat;
}
#endif

#ifdef F_configSetTimeFormat
void configSetTimeFormat(int timeFormat)
{
	ConfigParam config;
	Config2Param config2;

	// make sure time format is valid
	if(timeFormat < TIME_24H || timeFormat > TIME_12H)
		return;
	if(IsT10K())
		g_t10KConfig.timeFormat = timeFormat;

	// set time format
	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return;
	GetOsdConfigParam2(&config2, 1, 1);
	config2.timeFormat = timeFormat;
	SetOsdConfigParam2(&config2, 1, 1);
}
#endif

#ifdef F_configGetTimezone
int  configGetTimezone(void)
{
	ConfigParam config;

	if(IsT10K())
		return g_t10KConfig.timezoneOffset;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return 540;
	return config.timezoneOffset;
}
#endif

#ifdef F_configSetTimezone
void configSetTimezone(int timezoneOffset)
{
	ConfigParam config;

	// set offset from GMT
	if(IsT10K())
		g_t10KConfig.timezoneOffset = timezoneOffset;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return;
	config.timezoneOffset = timezoneOffset;
	SetOsdConfigParam(&config);
}
#endif

#ifdef F_configIsSpdifEnabled
int  configIsSpdifEnabled(void)
{
	ConfigParam config;

	if(IsT10K())
		return g_t10KConfig.spdifMode ^ 1;

	GetOsdConfigParam(&config);
	return config.spdifMode ^ 1;
}
#endif

#ifdef F_configSetSpdifEnabled
void configSetSpdifEnabled(int enabled)
{
	ConfigParam config;

	if(IsT10K())
		g_t10KConfig.spdifMode = enabled ^ 1;

	GetOsdConfigParam(&config);
	config.spdifMode = enabled ^ 1;
	SetOsdConfigParam(&config);
}
#endif

#ifdef F_configIsDaylightSavingEnabled
int  configIsDaylightSavingEnabled(void)
{
	ConfigParam config;
	Config2Param config2;

	if(IsT10K())
		return g_t10KConfig.daylightSaving;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return 0;
	GetOsdConfigParam2(&config2, 1, 1);

	return config2.daylightSaving;
}
#endif

#ifdef F_configSetDaylightSavingEnabled
void configSetDaylightSavingEnabled(int daylightSaving)
{
	ConfigParam config;
	Config2Param config2;

	if(IsT10K())
		g_t10KConfig.daylightSaving = daylightSaving;

	GetOsdConfigParam(&config);
	if(IsEarlyJap(config))
		return;
	GetOsdConfigParam2(&config2, 1, 1);
	config2.daylightSaving = daylightSaving;
	SetOsdConfigParam2(&config2, 1, 1);
}
#endif

// the following functions are all used in time conversion

#ifdef F_configGetTime
unsigned char frombcd(unsigned char bcd)
{
	return bcd - (bcd>>4)*6;
}
unsigned char tobcd(unsigned char dec)
{
	return dec + (dec/10)*6;
}

void converttobcd(sceCdCLOCK* time)
{
	time->second= tobcd(time->second);
	time->minute= tobcd(time->minute);
	time->hour	= tobcd(time->hour);
	time->day	= tobcd(time->day);
	time->month	= tobcd(time->month);
	time->year	= tobcd(time->year);
}
void convertfrombcd(sceCdCLOCK* time)
{
	time->second= frombcd(time->second);
	time->minute= frombcd(time->minute);
	time->hour	= frombcd(time->hour);
	time->day	= frombcd(time->day);
	time->month	= frombcd(time->month);
	time->year	= frombcd(time->year);
}

static const unsigned char gDaysInMonths[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void adddate(sceCdCLOCK* time)
{
	// get the days in each month and fix up feb depending on leap year
	unsigned char days_in_months[12];
	memcpy(days_in_months, gDaysInMonths, 12);
	if((time->year & 3) == 0)
		days_in_months[1] = 29;

	// increment the day and check its within the "day of the month" bounds
	time->day++;
	if(time->day > days_in_months[time->month - 1])
	{
		time->day = 1;

		// increment the month and check its within the "months in a year" bounds
		time->month++;
		if(time->month == 13)
		{
			time->month = 1;

			// check the year and increment it
			time->year++;
			if(time->year == 100)
			{
				time->year = 0;
			}
		}
	}
}
void subdate(sceCdCLOCK* time)
{
	// get the days in each month and fix up feb depending on leap year
	unsigned char days_in_months[12];
	memcpy(days_in_months, gDaysInMonths, 12);
	if((time->year & 3) == 0)
		days_in_months[1] = 29;

	// decrement the day and check its within the "day of the month" bounds
	time->day--;
	if(time->day == 0)
	{
		// decrement the month and check its within the "months in a year" bounds
		time->month--;
		if(time->month == 0)
		{
			time->month = 12;

			// check the year and decrement it
			if(time->year == 0)
				time->year = 99;
			else
				time->year--;
		}

		time->day = days_in_months[time->month-1];
	}
}

void addhour(sceCdCLOCK* time)
{
	time->hour++;
	if(time->hour == 24)
	{
		adddate(time);
		time->hour = 0;
	}
}
void subhour(sceCdCLOCK* time)
{
	if(time->hour == 0)
	{
		subdate(time);
		time->hour = 23;
	}
	else
		time->hour--;
}

void AdjustTime(sceCdCLOCK* time, int offset)
{
	convertfrombcd(time);
	offset += time->minute;

	if(offset >= 0)
	{
		while(offset >= 60)
		{
			addhour(time);
			offset -= 60;
		}
		time->minute = offset;
	}
	else
	{
		while(offset < 0)
		{
			subhour(time);
			offset += 60;
		}
		time->minute = offset;
	}

	converttobcd(time);
}

void configConvertToGmtTime(sceCdCLOCK* time)
{
	AdjustTime(time, -540);
}

void configConvertToLocalTime(sceCdCLOCK* time)
{
	int timezone_offset = configGetTimezone();
	int daylight_saving = configIsDaylightSavingEnabled();
	AdjustTime(time, timezone_offset - 540 + (daylight_saving * 60));
}
#endif
