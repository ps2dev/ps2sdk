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
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <string.h>
#include <osd_config.h>
#include <rom0_info.h>
#define NEWLIB_PORT_AWARE
#include <fileio.h>

/** config param data as stored on a DTL-T10000(H) TOOL */
typedef struct
{
    s16 timezoneOffset;
    u8 screenType;
    u8 dateFormat;
    u8 language;
    u8 spdifMode;
    u8 daylightSaving;
    u8 timeFormat;
} ConfigParamT10K;

#define defaultIODriver { (void *)fioOpen, fioClose, fioRead }

extern ConfigParamT10K g_t10KConfig;

#ifdef F__config_internals
ConfigParamT10K g_t10KConfig = {540, TV_SCREEN_43, DATE_YYYYMMDD, LANGUAGE_JAPANESE, 0, 0, 0};
#endif

#ifdef F_converttobcd
static inline unsigned char tobcd(unsigned char dec)
{
    return dec + (dec / 10) * 6;
}

void converttobcd(sceCdCLOCK *time)
{
    time->second = tobcd(time->second);
    time->minute = tobcd(time->minute);
    time->hour   = tobcd(time->hour);
    time->day    = tobcd(time->day);
    time->month  = tobcd(time->month);
    time->year   = tobcd(time->year);
}
#else
extern void converttobcd(sceCdCLOCK *time);
#endif

#ifdef F_convertfrombcd
static inline unsigned char frombcd(unsigned char bcd)
{
    return bcd - (bcd >> 4) * 6;
}


void convertfrombcd(sceCdCLOCK *time)
{
    time->second = frombcd(time->second);
    time->minute = frombcd(time->minute);
    time->hour   = frombcd(time->hour);
    time->day    = frombcd(time->day);
    time->month  = frombcd(time->month);
    time->year   = frombcd(time->year);
}
#else
extern void convertfrombcd(sceCdCLOCK *time);
#endif

#ifdef F___adjustTime
static const unsigned char gDaysInMonths[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static void adddate(sceCdCLOCK *time)
{
    // get the days in each month and fix up feb depending on leap year
    unsigned char days_in_months[12];
    memcpy(days_in_months, gDaysInMonths, 12);
    if ((time->year & 3) == 0)
        days_in_months[1] = 29;

    // increment the day and check its within the "day of the month" bounds
    time->day++;
    if (time->day > days_in_months[time->month - 1]) {
        time->day = 1;

        // increment the month and check its within the "months in a year" bounds
        time->month++;
        if (time->month == 13) {
            time->month = 1;

            // check the year and increment it
            time->year++;
            if (time->year == 100) {
                time->year = 0;
            }
        }
    }
}

static void subdate(sceCdCLOCK *time)
{
    // get the days in each month and fix up feb depending on leap year
    unsigned char days_in_months[12];
    memcpy(days_in_months, gDaysInMonths, 12);
    if ((time->year & 3) == 0)
        days_in_months[1] = 29;

    // decrement the day and check its within the "day of the month" bounds
    time->day--;
    if (time->day == 0) {
        // decrement the month and check its within the "months in a year" bounds
        time->month--;
        if (time->month == 0) {
            time->month = 12;

            // check the year and decrement it
            if (time->year == 0)
                time->year = 99;
            else
                time->year--;
        }

        time->day = days_in_months[time->month - 1];
    }
}

static void addhour(sceCdCLOCK *time)
{
    time->hour++;
    if (time->hour == 24) {
        adddate(time);
        time->hour = 0;
    }
}

static void subhour(sceCdCLOCK *time)
{
    if (time->hour == 0) {
        subdate(time);
        time->hour = 23;
    } else
        time->hour--;
}

void __adjustTime(sceCdCLOCK *time, int offset)
{
    convertfrombcd(time);
    offset += time->minute;

    if (offset >= 0) {
        while (offset >= 60) {
            addhour(time);
            offset -= 60;
        }
        time->minute = offset;
    } else {
        while (offset < 0) {
            subhour(time);
            offset += 60;
        }
        time->minute = offset;
    }

    converttobcd(time);
}
#else
extern void __adjustTime(sceCdCLOCK *time, int offset);
#endif

#ifdef F_IsEarlyJap
int IsEarlyJap(ConfigParam config)
{
    return config.version == 0;
}
#endif

#ifdef F_configGetLanguageWithIODriver
int configGetLanguageWithIODriver(_io_driver *driver)
{
    ConfigParam config;

    if (IsT10KWithIODriver(driver))
        return g_t10KConfig.language;

    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return config.japLanguage;
    return config.language;
}
#endif

#ifdef F_configGetLanguage
int configGetLanguage(void)
{
    _io_driver driver = defaultIODriver;
    return configGetLanguageWithIODriver(&driver);
}
#endif

#ifdef F_configSetLanguageWithIODriver
void configSetLanguageWithIODriver(int language, _io_driver *driver)
{
    ConfigParam config;

    // make sure language is valid
    if (language < LANGUAGE_JAPANESE || language > LANGUAGE_PORTUGUESE)
        return;
    if (IsT10KWithIODriver(driver))
        g_t10KConfig.language = language;

    // set language
    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        config.japLanguage = language;
    else
        config.language = language;
    SetOsdConfigParam(&config);
}
#endif

#ifdef F_configSetLanguage
void configSetLanguage(int language)
{
    _io_driver driver = defaultIODriver;
    configSetLanguageWithIODriver(language, &driver);
}
#endif

#ifdef F_configGetTvScreenTypeWithIODriver
int configGetTvScreenTypeWithIODriver(_io_driver *driver)
{
    ConfigParam config;

    if (IsT10KWithIODriver(driver))
        return g_t10KConfig.screenType;

    GetOsdConfigParam(&config);
    return config.screenType;
}
#endif

#ifdef F_configGetTvScreenType
int configGetTvScreenType(void)
{
    _io_driver driver = defaultIODriver;
    return configGetTvScreenTypeWithIODriver(&driver);
}
#endif

#ifdef F_configSetTvScreenTypeWithIODriver
void configSetTvScreenTypeWithIODriver(int screenType, _io_driver *driver)
{
    ConfigParam config;

    // make sure screen type is valid
    if (screenType < TV_SCREEN_43 || screenType > TV_SCREEN_169)
        return;
    if (IsT10KWithIODriver(driver))
        g_t10KConfig.screenType = screenType;

    // set screen type
    GetOsdConfigParam(&config);
    config.screenType = screenType;
    SetOsdConfigParam(&config);
}
#endif

#ifdef F_configSetTvScreenType
void configSetTvScreenType(int screenType)
{
    _io_driver driver = defaultIODriver;
    configSetTvScreenTypeWithIODriver(screenType, &driver);
}
#endif

#ifdef F_configGetDateFormatWithIODriver
int configGetDateFormatWithIODriver(_io_driver *driver)
{
    ConfigParam config;
    Config2Param config2;

    if (IsT10KWithIODriver(driver))
        return g_t10KConfig.dateFormat;

    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return 0;
    GetOsdConfigParam2(&config2, sizeof(config2), 0);
    return config2.dateFormat;
}
#endif

#ifdef F_configGetDateFormat
int configGetDateFormat(void)
{
    _io_driver driver = defaultIODriver;
    return configGetDateFormatWithIODriver(&driver);
}
#endif

#ifdef F_configSetDateFormatWithIODriver
void configSetDateFormatWithIODriver(int dateFormat, _io_driver *driver)
{
    ConfigParam config;
    Config2Param config2;

    // make sure date format is valid
    if (dateFormat < DATE_YYYYMMDD || dateFormat > DATE_DDMMYYYY)
        return;
    if (IsT10KWithIODriver(driver))
        g_t10KConfig.dateFormat = dateFormat;

    // set date format
    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return;
    GetOsdConfigParam2(&config2, sizeof(config2), 0);
    config2.dateFormat = dateFormat;
    SetOsdConfigParam2(&config2, sizeof(config2), 0);
}
#endif

#ifdef F_configSetDateFormat
void configSetDateFormat(int dateFormat)
{
    _io_driver driver = defaultIODriver;
    configSetDateFormatWithIODriver(dateFormat, &driver);
}
#endif

#ifdef F_configGetTimeFormatWithIODriver
int configGetTimeFormatWithIODriver(_io_driver *driver)
{
    ConfigParam config;
    Config2Param config2;

    if (IsT10KWithIODriver(driver))
        return g_t10KConfig.timeFormat;

    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return 0;
    GetOsdConfigParam2(&config2, sizeof(config2), 0);
    return config2.timeFormat;
}
#endif

#ifdef F_configGetTimeFormat
int configGetTimeFormat(void)
{
    _io_driver driver = defaultIODriver;
    return configGetTimeFormatWithIODriver(&driver);
}
#endif

#ifdef F_configSetTimeFormatWithIODriver
void configSetTimeFormatWithIODriver(int timeFormat, _io_driver *driver)
{
    ConfigParam config;
    Config2Param config2;

    // make sure time format is valid
    if (timeFormat < TIME_24H || timeFormat > TIME_12H)
        return;
    if (IsT10KWithIODriver(driver))
        g_t10KConfig.timeFormat = timeFormat;

    // set time format
    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return;
    GetOsdConfigParam2(&config2, sizeof(config2), 0);
    config2.timeFormat = timeFormat;
    SetOsdConfigParam2(&config2, sizeof(config2), 0);
}
#endif

#ifdef F_configSetTimeFormat
void configSetTimeFormat(int timeFormat)
{
    _io_driver driver = defaultIODriver;
    configSetTimeFormatWithIODriver(timeFormat, &driver);
}
#endif

#ifdef F_configGetTimezoneWithIODriver
int configGetTimezoneWithIODriver(_io_driver *driver)
{
    ConfigParam config;
    int timezoneOffset;

    if (IsT10KWithIODriver(driver))
    {
        timezoneOffset = g_t10KConfig.timezoneOffset;
    }
    else
    {
        GetOsdConfigParam(&config);
        if (IsEarlyJap(config))
        {
            timezoneOffset = 540;
        }
        else
        {
            timezoneOffset = config.timezoneOffset;
            // Check if this is negative, and manually make it positive using bit manipulation
            if ((timezoneOffset & 0x400) != 0)
            {
                // Flip bits
                timezoneOffset ^= 0x7ff;
                // Add one
                timezoneOffset += 1;
                // Make it negative
                timezoneOffset *= -1;
            }
        }
    }
    return timezoneOffset;
}
#endif

#ifdef F_configGetTimezone
int configGetTimezone(void)
{
    _io_driver driver = defaultIODriver;
    return configGetTimezoneWithIODriver(&driver);
}
#endif

#ifdef F_configSetTimezoneWithIODriver
void configSetTimezoneWithIODriver(int timezoneOffset, _io_driver *driver, void (*finishedCallback)(void))
{
    ConfigParam config;

    // set offset from GMT
    if (IsT10KWithIODriver(driver))
        g_t10KConfig.timezoneOffset = timezoneOffset;

    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return;

    {
        u32 wantedTimezoneOffset;

        // Reduce it to signed 11 bits if it is negative using bit manipulation
        if (timezoneOffset < 0)
        {
            // Make it positive
            wantedTimezoneOffset = -timezoneOffset;
            // Subtract one
            wantedTimezoneOffset -= 1;
            // Flip bits
            wantedTimezoneOffset ^= 0x7ff;
        }
        else
        {
            wantedTimezoneOffset = timezoneOffset;
        }

        config.timezoneOffset = wantedTimezoneOffset;
    }

    SetOsdConfigParam(&config);
    if (finishedCallback)
        finishedCallback();
}
#endif

#ifdef F_configSetTimezone
void configSetTimezone(int timezoneOffset)
{
    _io_driver driver = defaultIODriver;
    configSetTimezoneWithIODriver(timezoneOffset, &driver, NULL);
}
#endif

#ifdef F_configIsSpdifEnabledWithIODriver
int configIsSpdifEnabledWithIODriver(_io_driver *driver)
{
    ConfigParam config;

    if (IsT10KWithIODriver(driver))
        return g_t10KConfig.spdifMode ^ 1;

    GetOsdConfigParam(&config);
    return config.spdifMode ^ 1;
}
#endif

#ifdef F_configIsSpdifEnabled
int configIsSpdifEnabled(void)
{
    _io_driver driver = defaultIODriver;
    return configIsSpdifEnabledWithIODriver(&driver);
}
#endif

#ifdef F_configSetSpdifEnabledWithIODriver
void configSetSpdifEnabledWithIODriver(int enabled, _io_driver *driver)
{
    ConfigParam config;

    if (IsT10KWithIODriver(driver))
        g_t10KConfig.spdifMode = enabled ^ 1;

    GetOsdConfigParam(&config);
    config.spdifMode = enabled ^ 1;
    SetOsdConfigParam(&config);
}
#endif

#ifdef F_configSetSpdifEnabled
void configSetSpdifEnabled(int enabled)
{
    _io_driver driver = defaultIODriver;
    configSetSpdifEnabledWithIODriver(enabled, &driver);
}
#endif

#ifdef F_configIsDaylightSavingEnabledWithIODriver
int configIsDaylightSavingEnabledWithIODriver(_io_driver *driver)
{
    ConfigParam config;
    Config2Param config2;

    if (IsT10KWithIODriver(driver))
        return g_t10KConfig.daylightSaving;

    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return 0;
    GetOsdConfigParam2(&config2, sizeof(config2), 0);

    return config2.daylightSaving;
}
#endif

#ifdef F_configIsDaylightSavingEnabled
int configIsDaylightSavingEnabled(void)
{
    _io_driver driver = defaultIODriver;
    return configIsDaylightSavingEnabledWithIODriver(&driver);
}
#endif

#ifdef F_configSetDaylightSavingEnabledWithIODriver
void configSetDaylightSavingEnabledWithIODriver(int daylightSaving, _io_driver *driver, void (*finishedCallback)(void))
{
    ConfigParam config;
    Config2Param config2;

    if (IsT10KWithIODriver(driver))
        g_t10KConfig.daylightSaving = daylightSaving;

    GetOsdConfigParam(&config);
    if (IsEarlyJap(config))
        return;
    GetOsdConfigParam2(&config2, sizeof(config2), 0);
    config2.daylightSaving = daylightSaving;
    SetOsdConfigParam2(&config2, sizeof(config2), 0);
    if (finishedCallback)
        finishedCallback();
}
#endif

#ifdef F_configSetDaylightSavingEnabled
void configSetDaylightSavingEnabled(int daylightSaving)
{
    _io_driver driver = defaultIODriver;
    configSetDaylightSavingEnabledWithIODriver(daylightSaving, &driver, NULL);
}
#endif

#ifdef F_configConvertToGmtTime
void configConvertToGmtTime(sceCdCLOCK *time)
{
    __adjustTime(time, -540);
}
#endif

#ifdef F_configConvertToLocalTimeWithIODriver
void configConvertToLocalTimeWithIODriver(sceCdCLOCK *time, _io_driver *driver)
{
    int timezone_offset = configGetTimezoneWithIODriver(driver);
    int daylight_saving = configIsDaylightSavingEnabledWithIODriver(driver);
    __adjustTime(time, timezone_offset - 540 + (daylight_saving * 60));
}
#endif

#ifdef F_configConvertToLocalTime
void configConvertToLocalTime(sceCdCLOCK *time)
{
    _io_driver driver = defaultIODriver;
    configConvertToLocalTimeWithIODriver(time, &driver);
}
#endif
