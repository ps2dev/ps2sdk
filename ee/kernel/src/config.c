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
#include <config.h>

// config for dev ps2 T-10000
typedef struct
{
	u16 timezone;
	u8  screenType;
	u8  dateFormat;
	u8  language;
	u8  spdif;
	u8  daylightSaving;
	u8  timeFormat;
}
T10KConfig;
T10KConfig t10KConfig = {0x21C, 0, 0, 0, 0, 0, 0};

// stores romname of ps2
char RomName[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


// bool IS_JAP_PS2(u32 config)
// test for whether the ps2 is a japanese model
// 
// args:	u32 config value from GetOsdConfigParam() syscall
// returns:	true if japanese, false if otherwise
#define IS_JAP_PS2(config)	(((config >> 13) & 0x07) == 0)


// gets the romname from the current ps2
// 14 chars - doesnt set a null terminator
// 
// args:	buffer to hold romname (14 chars long)
// returns:	pointer to buffer containing romname
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


// check whether ps2 is actually dev model T-10000
// 
// returns: 1 if T-10000
//			0 if normal ps2
s32  IsT10K(void)
{
	GetRomName(RomName);
	if((RomName[4] ^ 'T') < 1)
		return 1;
	return 0;
}


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
s32  configGetLanguage(void)
{
	u32 config = 0;
	
	if(IsT10K())
		return t10KConfig.language;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return (config>> 4) & 0x01;
	return (config>>16) & 0x1F;
}
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
void configSetLanguage(s32 language)
{
	u32 config = 0;
	
	// make sure language is valid
	if(language < 0 || language > 7)
		return;
	if(IsT10K())
		t10KConfig.language = language;
	
	// set language
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		config = (config&(~(0x01<< 4))) | ((language&0x01)<< 4);
	else
		config = (config&(~(0x1F<<16))) | ((language&0x1F)<<16);
	SetOsdConfigParam(&config);
}


// get the tv screen type the ps2 is setup for
// 
// returns:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
s32  configGetTvScreenType(void)
{
	u32 config = 0;

	if(IsT10K())
		return t10KConfig.screenType;
	
	GetOsdConfigParam(&config);
	return (config>> 1) & 0x03;
}
// set the tv screen type
// 
// args:	0 = 4:3
//			1 = fullscreen
//			2 = 16:9
void configSetTvScreenType(s32 screenType)
{
	u32 config = 0;

	// make sure screen type is valid
	if(screenType < 0 || screenType > 2)
		return;
	if(IsT10K())
		t10KConfig.screenType = screenType;
	
	// set screen type
	GetOsdConfigParam(&config);
	config = (config&(~(0x03<<1))) | ((screenType&0x03)<<1);
	SetOsdConfigParam(&config);
}


// gets the date display format
// 
// returns:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
s32  configGetDateFormat(void)
{
	u32 config  = 0;
	u32 config2 = 0;
	
	if(IsT10K())
		return t10KConfig.dateFormat;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return 0;
	GetOsdConfigParam2(&config2, 1, 1);
	return config2 >> 6;
}
// sets the date display format
// 
// args:	0 = yyyy/mm/dd
//			1 = mm/dd/yyyy
//			2 = dd/mm/yyyy
void configSetDateFormat(s32 dateFormat)
{
	u32 config  = 0;
	u32 config2 = 0;
	
	// make sure date format is valid
	if(dateFormat < 0 || dateFormat > 2)
		return;
	if(IsT10K())
		t10KConfig.dateFormat = dateFormat;
	
	// set date format
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return;
	GetOsdConfigParam2(&config2, 1, 1);
	config2 = (config2&(~(3<<6))) | ((dateFormat&3)<<6);
	SetOsdConfigParam2(&config2, 1, 1);
}


// gets the time display format
// (whether 24hour time or not)
// 
// returns:	0 = 24hour
//			1 = 12hour
s32  configGetTimeFormat(void)
{
	u32 config  = 0;
	u32 config2 = 0;
	
	if(IsT10K())
		return t10KConfig.timeFormat;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return 0;
	GetOsdConfigParam2(&config2, 1, 1);
	return (config2 >> 5) & 0x01;
}
// sets the time display format
// (whether 24hour time or not)
// 
// args:	0 = 24hour
//			1 = 12hour
void configSetTimeFormat(s32 timeFormat)
{
	u32 config  = 0;
	u32 config2 = 0;
	
	// make sure time format is valid
	if(timeFormat < 0 || timeFormat > 1)
		return;
	if(IsT10K())
		t10KConfig.timeFormat = timeFormat;
	
	// set time format
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return;
	GetOsdConfigParam2(&config2, 1, 1);
	config2 = (config2&(~(0x01<<5))) | ((timeFormat&0x01)<<5);
	SetOsdConfigParam2(&config2, 1, 1);
}


// get timezone
// 
// returns: offset in minutes from GMT
s32  configGetTimezone(void)
{
	u32 config = 0;

	if(IsT10K())
		return t10KConfig.timezone;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return 0x21C;
	return (config >> 21)&0x7FF;
}
// set timezone
// 
// args:	offset in minutes from GMT
void configSetTimezone(s32 offset)
{
	u32 config = 0;

	// set offset from GMT
	if(IsT10K())
		t10KConfig.timezone = offset;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return;
	config = (config&(~(0x7FF<<21))) | ((offset&0x7FF)<<21);
	SetOsdConfigParam(&config);
}


// checks whether the spdif is enabled or not
// 
// returns:	1 = on
//			0 = off
s32  configIsSpdifEnabled(void)
{
	u32 config = 0;
	
	if(IsT10K())
		return t10KConfig.spdif;
	
	GetOsdConfigParam(&config);
	return (config & 0x01)^0x01;
}
// sets whether the spdif is enabled or not
// 
// args:	1 = on
//			0 = off
void configSetSpdifEnabled(s32 enabled)
{
	u32 config = 0;
	
	if(IsT10K())
		t10KConfig.spdif = enabled;
	
	GetOsdConfigParam(&config);
	config = (config&(~0x01)) | (0x01^(enabled&0x01));
	SetOsdConfigParam(&config);
}


// checks whether daylight saving is currently set
// 
// returns:	1 = on
//			0 = off
s32  configIsDaylightSavingEnabled(void)
{
	u32 config = 0;
	u32 config2 = 0;
	
	if(IsT10K())
		return t10KConfig.daylightSaving;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return 0;
	GetOsdConfigParam2(&config2, 1, 1);
	
	return (config2 >> 4) & 0x01;
}
// checks whether daylight saving is currently set
// 
// returns:	1 = on
//			0 = off
void configSetDaylightSavingEnabled(s32 enabled)
{
	u32 config  = 0;
	u32 config2 = 0;
	
	if(IsT10K())
		t10KConfig.daylightSaving = enabled;
	
	GetOsdConfigParam(&config);
	if(IS_JAP_PS2(config))
		return;
	GetOsdConfigParam2(&config2, 1, 1);
	config2 = (config2&(~(0x01<<4))) | ((enabled&0x01)<<4);
	SetOsdConfigParam2(&config2, 1, 1);
	
}


