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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <tamtypes.h>

// language values returned by: s32 getLanguage(void)
#define LANGUAGE_JAPANESE	0
#define LANGUAGE_ENGLISH	1
#define LANGUAGE_FRENCH		2
#define LANGUAGE_SPANISH	3
#define LANGUAGE_GERMAN		4
#define LANGUAGE_ITALIAN	5
#define LANGUAGE_DUTCH		6
#define LANGUAGE_PORTUGUESE	7

// tv screen values returned by: s32 getTvScreenType(void)
#define TV_SCREEN_43		0
#define TV_SCREEN_FULL		1
#define TV_SCREEN_169		2

// date format values returned by: s32 getDateFormat(void)
#define DATE_YYYYMMDD		0
#define DATE_MMDDYYYY		1
#define DATE_DDMMYYYY		2


#ifdef __cplusplus
extern "C" {
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
s32  configGetLanguage(void);
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
void configSetLanguage(s32 language);


// get the tv screen type the ps2 is setup for
// 
// returns:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
s32  configGetTvScreenType(void);
// set the tv screen type
// 
// args:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
void configSetTvScreenType(s32 screenType);


// gets the date display format
// 
// returns:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
s32  configGetDateFormat(void);
// sets the date display format
// 
// args:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
void configSetDateFormat(s32 dateFormat);


// gets the time display format
// (whether 24hour time or not)
// 
// returns:	0 = 24hour
//			1 = 12hour
s32  configGetTimeFormat(void);
// sets the time display format
// (whether 24hour time or not)
// 
// args:	0 = 24hour
//			1 = 12hour
void configSetTimeFormat(s32 timeFormat);


// get timezone
// 
// returns: offset in minutes from GMT
s32  configGetTimezone(void);
// set timezone
// 
// args:	offset in minutes from GMT
void configSetTimezone(s32 offset);


// checks whether the spdif is enabled or not
// 
// returns:	1 = on
//			0 = off
s32  configIsSpdifEnabled(void);
// sets whether the spdif is enabled or not
// 
// args:	1 = on
//			0 = off
void configSetSpdifEnabled(s32 enabled);


// checks whether daylight saving is currently set
// 
// returns:	1 = on
//			0 = off
s32  configIsDaylightSavingEnabled(void);
// checks whether daylight saving is currently set
// 
// returns:	1 = on
//			0 = off
void configSetDaylightSavingEnabled(s32 enabled);


#ifdef __cplusplus
}
#endif

#endif	// _CONFIG_H_

