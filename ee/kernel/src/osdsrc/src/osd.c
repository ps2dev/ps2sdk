/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <tamtypes.h>
#include <osd_config.h>

#include "osd.h"

extern SystemConfiguration_t SystemConfiguration;

void InitSystemConfig(SystemConfiguration_t *SysConf, int SysConfLen){
	unsigned int SysConfRegAddr, i;
	vu8 *ptr;

	if((SysConfRegAddr=*(vu32*)0xbc0003c0)!=0){
		ptr=(vu8 *)(0xbc000000+SysConfRegAddr+0xF);

		for(i=0; i<SysConfLen; i++) SysConf->data[i]=ptr[i];
	}

	if((SysConf->EEGS>>6&7)==0){
		SysConf->EEGS&=0xFFFF02FFFFFFFFFF;
	}
}

static ConfigParam OSDConfig;

void SetOsdConfigParam(ConfigParam* config){
	OSDConfig.spdifMode=config->spdifMode;
	OSDConfig.screenType=config->screenType;
	OSDConfig.videoOutput=config->videoOutput;
	OSDConfig.japLanguage=config->japLanguage;
	OSDConfig.ps1drvConfig=config->ps1drvConfig;
	OSDConfig.version=config->version;
	OSDConfig.language=config->language;
	OSDConfig.timezoneOffset=config->timezoneOffset;
}

void GetOsdConfigParam(ConfigParam* config){
	config->spdifMode=OSDConfig.spdifMode;
	config->screenType=OSDConfig.screenType;
	config->videoOutput=OSDConfig.videoOutput;
	config->japLanguage=OSDConfig.japLanguage;
	config->ps1drvConfig=OSDConfig.ps1drvConfig;
	config->version=OSDConfig.version;
	config->language=OSDConfig.language;
	config->timezoneOffset=OSDConfig.timezoneOffset;
}

static u8 OSDConfig2[128];

void SetOsdConfigParam2(void* config, int size, int offset){
	unsigned int AmountToWrite, WriteEnd, i;
	u8 *ptr;

	ptr=config;
	if((WriteEnd=offset+size)>=0x81){
		if(offset<0x80){
			AmountToWrite=0x80-offset;
		}
		else{
			offset=0x80;
			AmountToWrite=0;
		}

		WriteEnd=AmountToWrite+offset;
	}

	for(i=0; offset<WriteEnd; i++,offset++){
		OSDConfig2[offset]=ptr[i];
	}
}

int GetOsdConfigParam2(void* config, int size, int offset){
	unsigned int AmountToRead, ReadEnd, i;
	u8 *ptr;

	ptr=config;
	if((ReadEnd=offset+size)>=0x81){
		if(offset<0x80){
			AmountToRead=0x80-offset;
		}
		else{
			offset=0x80;
			AmountToRead=0;
		}
	}
	else AmountToRead=size;

	ReadEnd=AmountToRead+offset;
	for(i=0; offset<ReadEnd; i++,offset++){
		ptr[i]=OSDConfig2[offset];
	}

	return(((SystemConfiguration.EEGS>>6&7)!=0)?(SystemConfiguration.EEGS>>44&0xF):0);
}

