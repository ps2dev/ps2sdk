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
 * Note: the 'set' methods are only valid till the ps2 gets
 * turned off or reset!
 *
 * Note 2: Early Japanese consoles (The SCPH-10000 and SCPH-15000) have kernels based on older specifications.
 * 	The SCPH-18000 has the same kernel as the first expansion bay model (SCPH-30000).
 * 	The early kernels can be modernized if patched.
 *	Newer games will automatically patch ExecPS2 of such a kernel when they detect one,
 *	although only HDDOSDs will fully patch kernel completely (causes the kernel to appear as a newer kernel to everything).
 */

#ifndef __OSD_CONFIG_H__
#define __OSD_CONFIG_H__

#include <tamtypes.h>
#include <rom0_info.h>
#ifndef OSD_CONFIG_NO_LIBCDVD
#include <libcdvd.h>
#endif

/** language values returned and used by: configGetLanguage() and configSetLanguage() */
enum OSD_LANGUAGES {
    LANGUAGE_JAPANESE = 0,
    LANGUAGE_ENGLISH,
    LANGUAGE_FRENCH,
    LANGUAGE_SPANISH,
    LANGUAGE_GERMAN,
    LANGUAGE_ITALIAN,
    LANGUAGE_DUTCH,
    LANGUAGE_PORTUGUESE,
    /*	Language values after here are not officially documented.
        Are supported by neither this library nor the original Sony libscf library,
        but can be found in the MECHACON's NVRAM and are used exclusively by the OSDs of consoles that support these languages.
        However, these consoles will always specify English within ConfigParam.

        When ConfigParam.version is 2, the true language can be found in Config2Param.language. */
    LANGUAGE_RUSSIAN,
    LANGUAGE_KOREAN,
    LANGUAGE_TRAD_CHINESE,
    LANGUAGE_SIMPL_CHINESE
};

/** tv screen values returned by: int getTvScreenType(void) */
enum TV_SCREEN_TYPES {
    TV_SCREEN_43 = 0,
    TV_SCREEN_FULL,
    TV_SCREEN_169
};

/** Video output type values */
enum VIDEO_OUTPUT_TYPES {
    VIDEO_OUTPUT_RGB = 0,
    VIDEO_OUTPUT_COMPONENT
};

/** date format values returned by: int getDateFormat(void) */
enum DATE_FORMAT_TYPES {
    DATE_YYYYMMDD = 0,
    DATE_MMDDYYYY,
    DATE_DDMMYYYY
};

/** Time format values returned by: int getTimeFormat(void) */
enum TIME_FORMAT_TYPES {
    TIME_24H = 0,
    TIME_12H,
};

/** parameter struct as used by GetOsdConfigParam/SetOsdConfigParam */
typedef struct
{
    /** 0=enabled, 1=disabled */
    /*00*/ u32 spdifMode      : 1;
    /** 0=4:3, 1=fullscreen, 2=16:9 */
    /*01*/ u32 screenType     : 2;
    /** 0=rgb(scart), 1=component */
    /*03*/ u32 videoOutput    : 1;
    /** 0=japanese, 1=english(non-japanese) */
    /*04*/ u32 japLanguage    : 1;
    /** Playstation driver settings. */
    /*05*/ u32 ps1drvConfig   : 8;
    /** 0 = early Japanese OSD, 1 = OSD2, 2 = OSD2 with extended languages. Early kernels cannot retain the value set in this field (Hence always 0). */
    /*13*/ u32 version        : 3;
    /** LANGUAGE_??? value */
    /*16*/ u32 language       : 5;
    /** timezone minutes offset from gmt */
    /*21*/ u32 timezoneOffset : 11;
} ConfigParam;

/** parameter struct as used by GetOsdConfigParam2/SetOsdConfigParam2
 * (Not supported by unpatched, early kernels. Do NOT invoke GetOsdConfigParam2 or SetOsdConfigParam2 on one!)
 */
typedef struct
{
    /*00*/ u8 format;

    /*00*/ u8 reserved       : 4;
    /** 0=standard(winter), 1=daylight savings(summer) */
    /*04*/ u8 daylightSaving : 1;
    /** 0=24 hour, 1=12 hour */
    /*05*/ u8 timeFormat     : 1;
    /** 0=YYYYMMDD, 1=MMDDYYYY, 2=DDMMYYYY */
    /*06*/ u8 dateFormat     : 2;

    // Only used if ConfigParam.version = 2
    /** Set to 2 */
    /*00*/ u8 version;
    /** The true language, unlike the one from ConfigParam */
    /*00*/ u8 language;
} Config2Param;

#ifdef __cplusplus
extern "C" {
#endif

/** get the language the ps2 is currently set to
 * @return Language value (See OSD_LANGUAGES above)
 */
extern int configGetLanguage(void);
static inline int configGetLanguageWithIODriver(const _io_driver *driver) { return configGetLanguage(); }

/** sets the default language of the ps2
 * @param language Language value (See OSD_LANGUAGES above)
 */
extern void configSetLanguage(int language);
static inline void configSetLanguageWithIODriver(int language, const _io_driver *driver) { configSetLanguage(language); }

/** get the tv screen type the ps2 is setup for
 * @return 0 = 4:3; 1 = fullscreen; 2 = 16:9
 */
extern int configGetTvScreenType(void);
static inline int configGetTvScreenTypeWithIODriver(const _io_driver *driver) { return configGetTvScreenType(); }

/** set the tv screen type
 * @param screenType 0 = 4:3; 1 = fullscreen; 2 = 16:9
 */
extern void configSetTvScreenType(int screenType);
static inline void configSetTvScreenTypeWithIODriver(int screenType, const _io_driver *driver) { configSetTvScreenType(screenType); }

/** gets the date display format
 * @return 0 = yyyy/mm/dd; 1 = mm/dd/yyyy; 2 = dd/mm/yyyy
 */
extern int configGetDateFormat(void);
static inline int configGetDateFormatWithIODriver(const _io_driver *driver) { return configGetDateFormat(); }

/** sets the date display format
 * @param dateFormat 0 = yyyy/mm/dd; 1 = mm/dd/yyyy; 2 = dd/mm/yyyy
 */
extern void configSetDateFormat(int dateFormat);
static inline void configSetDateFormatWithIODriver(int dateFormat, const _io_driver *driver) { configSetDateFormat(dateFormat); }

/** gets the time display format
 * (whether 24hour time or not)
 * @return 0 = 24hour; 1 = 12hour
 */
extern int configGetTimeFormat(void);
static inline int configGetTimeFormatWithIODriver(const _io_driver *driver) { return configGetTimeFormat(); }

/** sets the time display format
 * (whether 24hour time or not)
 * @param timeFormat 0 = 24hour; 1 = 12hour
 */
extern void configSetTimeFormat(int timeFormat);
static inline void configSetTimeFormatWithIODriver(int timeFormat, const _io_driver *driver) { configSetTimeFormat(timeFormat); }

/** get timezone
 * @return offset in minutes from GMT
 */
extern int configGetTimezone(void);
static inline int configGetTimezoneWithIODriver(const _io_driver *driver) { return configGetTimezone(); }

/** set timezone
 * @param offset offset in minutes from GMT
 */
extern void configSetTimezone(int offset);
static inline void configSetTimezoneWithIODriver(int offset, const _io_driver *driver, void *finishedCallback) { configSetTimezone(offset); }

/** checks whether the spdif is enabled or not
 * @return 1 = on; 0 = off
 */
extern int configIsSpdifEnabled(void);
static inline int configIsSpdifEnabledWithIODriver(const _io_driver *driver) { return configIsSpdifEnabled(); }

/** sets whether the spdif is enabled or not
 * @param enabled 1 = on; 0 = off
 */
extern void configSetSpdifEnabled(int enabled);
static inline void configSetSpdifEnabledWithIODriver(int enabled, const _io_driver *driver) { configSetSpdifEnabled(enabled); }

/** checks whether daylight saving is currently set
 * @return 1 = on; 0 = off
 */
extern int configIsDaylightSavingEnabled(void);
static inline int configIsDaylightSavingEnabledWithIODriver(const _io_driver *driver) { return configIsDaylightSavingEnabled(); }

/** sets daylight saving
 * @param enabled 1 = on; 0 = off
 */
extern void configSetDaylightSavingEnabled(int enabled);
static inline void configSetDaylightSavingEnabledWithIODriver(int enabled, const _io_driver *driver, void *finishedCallback) { configSetDaylightSavingEnabled(enabled); }

#ifndef OSD_CONFIG_NO_LIBCDVD
/** converts the time returned from the ps2's clock into GMT time
 * (ps2 clock is in JST time)
 */
extern void configConvertToGmtTime(sceCdCLOCK *time);

/** converts the time returned from the ps2's clock into local time
 * (ps2 clock is in JST time)
 */
extern void configConvertToLocalTime(sceCdCLOCK *time);
static inline void configConvertToLocalTimeWithIODriver(sceCdCLOCK *time, const _io_driver *driver) { configConvertToLocalTime(time); }
#endif

// Internal functions.
/** check if ps2 has a 'Protokernel' (Really early Japanese models)
 *
 * @param config unsigned int config value from GetOsdConfigParam() syscall
 * @return 1 if early jap model; 0 if not
 */
extern int IsEarlyJap(ConfigParam config);

#ifdef __cplusplus
}
#endif

#endif /* __OSD_CONFIG_H__ */
