/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# PS2 Configuration settings
# Note: the 'set' methods are only valid till the ps2 gets
# turned off or reset!
#
# Note 2: Early Japanese consoles (The SCPH-10000 and SCPH-15000) have kernels based on older specifications.
# 	The SCPH-18000 has the same kernel as the first expansion bay model (SCPH-30000).
# 	The early kernels can be modernized if patched.
#	Newer games will automatically patch ExecPS2 of such a kernel when they detect one,
#	although only HDDOSDs will fully patch kernel completely (causes the kernel to appear as a newer kernel to everything).
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <tamtypes.h>
#include <libcdvd.h>

// language values returned and used by: configGetLanguage() and configSetLanguage()
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

// tv screen values returned by: int getTvScreenType(void)
enum TV_SCREEN_TYPES {
	TV_SCREEN_43 = 0,
	TV_SCREEN_FULL,
	TV_SCREEN_169
};

// date format values returned by: int getDateFormat(void)
enum DATE_FORMAT_TYPES {
	DATE_YYYYMMDD = 0,
	DATE_MMDDYYYY,
	DATE_DDMMYYYY
};

// Time format values returned by: int getTimeFormat(void)
enum TIME_FORMAT_TYPES {
	TIME_24H = 0,
	TIME_12H,
};

// parameter struct as used by GetOsdConfigParam/SetOsdConfigParam
typedef struct
{
	/*00*/ u32 spdifMode : 1;       // 0=enabled, 1=disabled
	/*01*/ u32 screenType : 2;      // 0=4:3, 1=fullscreen, 2=16:9
	/*03*/ u32 videoOutput : 1;     // 0=rgb(scart), 1=component
	/*04*/ u32 japLanguage : 1;     // 0=japanese, 1=english(non-japanese)
	/*05*/ u32 ps1drvConfig : 8;    // Playstation driver settings.
	/*13*/ u32 version : 3;         // 0 = early Japanese OSD, 1 = OSD2, 2 = OSD2 with extended languages. Early kernels cannot retain the value set in this field (Hence always 0).
	/*16*/ u32 language : 5;        // LANGUAGE_??? value
	/*21*/ u32 timezoneOffset : 11; // timezone minutes offset from gmt
} ConfigParam;

// parameter struct as used by GetOsdConfigParam2/SetOsdConfigParam2
// (Not supported by unpatched, early kernels. Do NOT invoke GetOsdConfigParam2 or SetOsdConfigParam2 on one!)
typedef struct
{
	/*00*/ u16 reserved : 4;
	/*04*/ u16 daylightSaving : 1; // 0=standard(winter), 1=daylight savings(summer)
	/*05*/ u16 timeFormat : 1;     // 0=24 hour, 1=12 hour
	/*06*/ u16 dateFormat : 2;     // 0=YYYYMMDD, 1=MMDDYYYY, 2=DDMMYYYY

	//Only used if ConfigParam.version = 2
	/*08*/ u8 version;  //Set to 2
	/*16*/ u8 language; //The true language, unlike the one from ConfigParam
} Config2Param;

// get the language the ps2 is currently set to
//
// returns:	Language value (See OSD_LANGUAGES above)
int configGetLanguage(void);
// sets the default language of the ps2
//
// args:	Language value (See OSD_LANGUAGES above)
void configSetLanguage(int language);


// get the tv screen type the ps2 is setup for
//
// returns:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
int configGetTvScreenType(void);
// set the tv screen type
//
// args:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
void configSetTvScreenType(int screenType);


// gets the date display format
//
// returns:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
int configGetDateFormat(void);
// sets the date display format
//
// args:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
void configSetDateFormat(int dateFormat);


// gets the time display format
// (whether 24hour time or not)
//
// returns:	0 = 24hour
//			1 = 12hour
int configGetTimeFormat(void);
// sets the time display format
// (whether 24hour time or not)
//
// args:	0 = 24hour
//			1 = 12hour
void configSetTimeFormat(int timeFormat);

// get timezone
//
// returns: offset in minutes from GMT
int configGetTimezone(void);
// set timezone
//
// args:	offset in minutes from GMT
void configSetTimezone(int offset);

// checks whether the spdif is enabled or not
//
// returns:	1 = on
//			0 = off
int configIsSpdifEnabled(void);
// sets whether the spdif is enabled or not
//
// args:	1 = on
//			0 = off
void configSetSpdifEnabled(int enabled);

// checks whether daylight saving is currently set
//
// returns:	1 = on
//			0 = off
int configIsDaylightSavingEnabled(void);
// checks whether daylight saving is currently set
//
// returns:	1 = on
//			0 = off
void configSetDaylightSavingEnabled(int enabled);

// converts the time returned from the ps2's clock into GMT time
// (ps2 clock is in JST time)
void configConvertToGmtTime(sceCdCLOCK *time);

// converts the time returned from the ps2's clock into LOCAL time
// (ps2 clock is in JST time)
void configConvertToLocalTime(sceCdCLOCK *time);

// Internal functions.
int IsT10K(void);
int IsEarlyJap(ConfigParam config);
char *GetRomName(char *romname);

#ifdef __cplusplus
}
#endif

#endif // _CONFIG_H_
