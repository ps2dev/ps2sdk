/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# PS2 Configuration settings
# note: the 'set' methods are only valid till the ps2 gets
# turned off or reset!
*/

#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <string.h>
#include <osd_config.h>

// config param data as stored on T-10000 (TOOLs)
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

// stores romname of ps2
char g_RomName[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif


// gets the romname from the current ps2
// 14 chars - doesnt set a null terminator
// 
// args:	buffer to hold romname (14 chars long)
// returns:	pointer to buffer containing romname
#ifdef F_GetRomName
char* GetRomName(char *romname)
{
	s32 fd = 0;
	// the romname from my pal bios
//	memcpy(romname, "0120EC20000902", 14);
	
	fioInit();
	fd = fioOpen("rom0:ROMVER", O_RDONLY);
	fioRead(fd, romname, 14);
	fioClose(fd);
	return romname;
}
#endif


// check whether ps2 is actually dev model T-10000
// 
// returns: 1 if T-10000
//			0 if not
#ifdef F_IsT10K
s32 IsT10K(void)
{
	// only read in the romname the first time
	if(g_RomName[0] == 0)
		GetRomName(g_RomName);
	return (g_RomName[4] == 'T') ? 1 : 0;
}
#endif


// check if ps2 is one of the really early japanese models
// 
// args:	u32 config value from GetOsdConfigParam() syscall
// returns:	1 if early jap model
//			0 if not
#ifdef F_IsEarlyJap
s32 IsEarlyJap(ConfigParam config)
{
	return config.region == 0;
}
#endif


// get the language the ps2 is currently set to
// 
// returns:	0 = japanese
//			1 = english
//			2 = french
//			3 = spanish
//			4 = german
//			5 = italian
//			6 = dutch
//			7 = portuguese
#ifdef F_configGetLanguage
s32  configGetLanguage(void)
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


// sets the default language of the ps2
// 
// args:	0 = japanese
//			1 = english
//			2 = french
//			3 = spanish
//			4 = german
//			5 = italian
//			6 = dutch
//			7 = portuguese
#ifdef F_configSetLanguage
void configSetLanguage(s32 language)
{
	ConfigParam config;
	
	// make sure language is valid
	if(language < 0 || language > 7)
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


// get the tv screen type the ps2 is setup for
// 
// returns:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
#ifdef F_configGetTvScreenType
s32  configGetTvScreenType(void)
{
	ConfigParam config;

	if(IsT10K())
		return g_t10KConfig.screenType;
	
	GetOsdConfigParam(&config);
	return config.screenType;
}
#endif


#ifdef F_configSetTvScreenType
// set the tv screen type
// 
// args:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
void configSetTvScreenType(s32 screenType)
{
	ConfigParam config;

	// make sure screen type is valid
	if(screenType < 0 || screenType > 2)
		return;
	if(IsT10K())
		g_t10KConfig.screenType = screenType;
	
	// set screen type
	GetOsdConfigParam(&config);
	config.screenType = screenType;
	SetOsdConfigParam(&config);
}
#endif


// gets the date display format
// 
// returns:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
#ifdef F_configGetDateFormat
s32  configGetDateFormat(void)
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


// sets the date display format
// 
// args:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
#ifdef F_configSetDateFormat
void configSetDateFormat(s32 dateFormat)
{
	ConfigParam config;
	Config2Param config2;
	
	// make sure date format is valid
	if(dateFormat < 0 || dateFormat > 2)
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


// gets the time display format
// (whether 24hour time or not)
// 
// returns:	0 = 24hour
//			1 = 12hour
#ifdef F_configGetTimeFormat
s32  configGetTimeFormat(void)
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


// sets the time display format
// (whether 24hour time or not)
// 
// args:	0 = 24hour
//			1 = 12hour
#ifdef F_configSetTimeFormat
void configSetTimeFormat(s32 timeFormat)
{
	ConfigParam config;
	Config2Param config2;
	
	// make sure time format is valid
	if(timeFormat < 0 || timeFormat > 1)
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


// get timezone
// 
// returns: offset in minutes from GMT
#ifdef F_configGetTimezone
s32  configGetTimezone(void)
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


// set timezone
// 
// args:	offset in minutes from GMT
#ifdef F_configSetTimezone
void configSetTimezone(s32 timezoneOffset)
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


// checks whether the spdif is enabled or not
// 
// returns:	1 = on
//			0 = off
#ifdef F_configIsSpdifEnabled
s32  configIsSpdifEnabled(void)
{
	ConfigParam config;
	
	if(IsT10K())
		return g_t10KConfig.spdifMode ^ 1;
	
	GetOsdConfigParam(&config);
	return config.spdifMode ^ 1;
}
#endif


// sets whether the spdif is enabled or not
// 
// args:	1 = on
//			0 = off
#ifdef F_configSetSpdifEnabled
void configSetSpdifEnabled(s32 enabled)
{
	ConfigParam config;
	
	if(IsT10K())
		g_t10KConfig.spdifMode = enabled ^ 1;
	
	GetOsdConfigParam(&config);
	config.spdifMode = enabled ^ 1;
	SetOsdConfigParam(&config);
}
#endif


// checks whether daylight saving is currently set
// 
// returns:	1 = on
//			0 = off
#ifdef F_configIsDaylightSavingEnabled
s32  configIsDaylightSavingEnabled(void)
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


// checks whether daylight saving is currently set
// 
// returns:	1 = on
//			0 = off
#ifdef F_configSetDaylightSavingEnabled
void configSetDaylightSavingEnabled(s32 daylightSaving)
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
u8 frombcd(u8 bcd)
{
	return bcd - (bcd>>4)*6;
}
u8 tobcd(u8 dec)
{
	return dec + (dec/10)*6;
}

void converttobcd(CdvdClock_t* time)
{
	time->second= tobcd(time->second);
	time->minute= tobcd(time->minute);
	time->hour	= tobcd(time->hour);
	time->day	= tobcd(time->day);
	time->month	= tobcd(time->month);
	time->year	= tobcd(time->year);
}
void convertfrombcd(CdvdClock_t* time)
{
	time->second= frombcd(time->second);
	time->minute= frombcd(time->minute);
	time->hour	= frombcd(time->hour);
	time->day	= frombcd(time->day);
	time->month	= frombcd(time->month);
	time->year	= frombcd(time->year);
}

static u8 gDaysInMonths[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void adddate(CdvdClock_t* time)
{
	// get the days in each month and fix up feb depending on leap year
	u8 days_in_months[12];
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
void subdate(CdvdClock_t* time)
{
	// get the days in each month and fix up feb depending on leap year
	u8 days_in_months[12];
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

void addhour(CdvdClock_t* time)
{
	time->hour++;
	if(time->hour == 24)
	{
		adddate(time);
		time->hour = 0;
	}
}
void subhour(CdvdClock_t* time)
{
	if(time->hour == 0)
	{
		subdate(time);
		time->hour = 23;
	}
	else
		time->hour--;
}

void AdjustTime(CdvdClock_t* time, s32 offset)
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


// converts the time returned from the ps2's clock into GMT time
// (ps2 clock is in JST time)
void configConvertToGmtTime(CdvdClock_t* time)
{
	AdjustTime(time, -540);
}

// converts the time returned from the ps2's clock into LOCAL time
// (ps2 clock is in JST time)
void configConvertToLocalTime(CdvdClock_t* time)
{
	s32 timezone_offset = configGetTimezone();
	s32 daylight_saving = configIsDaylightSavingEnabled();
	AdjustTime(time, timezone_offset - 540 + (daylight_saving * 60));
}
#endif

