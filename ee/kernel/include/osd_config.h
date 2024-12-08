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
extern int configGetLanguageWithIODriver(_io_driver *driver);

/** sets the default language of the ps2
 * @param language Language value (See OSD_LANGUAGES above)
 */
extern void configSetLanguage(int language);
extern void configSetLanguageWithIODriver(int language, _io_driver *driver);


/** get the tv screen type the ps2 is setup for
 * @return 0 = 4:3; 1 = fullscreen; 2 = 16:9
 */
extern int configGetTvScreenType(void);
extern int configGetTvScreenTypeWithIODriver(_io_driver *driver);

/** set the tv screen type
 * @param screenType 0 = 4:3; 1 = fullscreen; 2 = 16:9
 */
extern void configSetTvScreenType(int screenType);
extern void configSetTvScreenTypeWithIODriver(int screenType, _io_driver *driver);

/** gets the date display format
 * @return 0 = yyyy/mm/dd; 1 = mm/dd/yyyy; 2 = dd/mm/yyyy
 */
extern int configGetDateFormat(void);
extern int configGetDateFormatWithIODriver(_io_driver *driver);

/** sets the date display format
 * @param dateFormat 0 = yyyy/mm/dd; 1 = mm/dd/yyyy; 2 = dd/mm/yyyy
 */
extern void configSetDateFormat(int dateFormat);
extern void configSetDateFormatWithIODriver(int dateFormat, _io_driver *driver);

/** gets the time display format
 * (whether 24hour time or not)
 * @return 0 = 24hour; 1 = 12hour
 */
extern int configGetTimeFormat(void);
extern int configGetTimeFormatWithIODriver(_io_driver *driver);

/** sets the time display format
 * (whether 24hour time or not)
 * @param timeFormat 0 = 24hour; 1 = 12hour
 */
extern void configSetTimeFormat(int timeFormat);
extern void configSetTimeFormatWithIODriver(int timeFormat, _io_driver *driver);

/** get timezone
 * @return offset in minutes from GMT
 */
extern int configGetTimezone(void);
extern int configGetTimezoneWithIODriver(_io_driver *driver);

/** set timezone
 * @param offset offset in minutes from GMT
 */
extern void configSetTimezone(int offset);
extern void configSetTimezoneWithIODriver(int timezoneOffset, _io_driver *driver, void (*finishedCallback)(void));

/** checks whether the spdif is enabled or not
 * @return 1 = on; 0 = off
 */
extern int configIsSpdifEnabled(void);
extern int configIsSpdifEnabledWithIODriver(_io_driver *driver);

/** sets whether the spdif is enabled or not
 * @param enabled 1 = on; 0 = off
 */
extern void configSetSpdifEnabled(int enabled);
extern void configSetSpdifEnabledWithIODriver(int enabled, _io_driver *driver);

/** checks whether daylight saving is currently set
 * @return 1 = on; 0 = off
 */
extern int configIsDaylightSavingEnabled(void);
extern int configIsDaylightSavingEnabledWithIODriver(_io_driver *driver);

/** sets daylight saving
 * @param enabled 1 = on; 0 = off
 */
extern void configSetDaylightSavingEnabled(int enabled);
extern void configSetDaylightSavingEnabledWithIODriver(int daylightSaving, _io_driver *driver, void (*finishedCallback)(void));

#ifndef OSD_CONFIG_NO_LIBCDVD
/** converts the time returned from the ps2's clock into GMT time
 * (ps2 clock is in JST time)
 */
extern void configConvertToGmtTime(sceCdCLOCK *time);
extern void configConvertToLocalTimeWithIODriver(sceCdCLOCK *time, _io_driver *driver);
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
