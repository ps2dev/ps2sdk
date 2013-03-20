#include <tamtypes.h>
#include <osd_config.h>

#include "osd.h"

extern unsigned char SystemConfiguration[];

void InitSystemConfig(unsigned char *SysConf, int SysConfLen){
	unsigned int SysConfRegAddr, i;
	volatile unsigned char *ptr;
	u64 *config;

	if((SysConfRegAddr=*(volatile unsigned int*)0xbc0003c0)!=0){
		ptr=(unsigned char *)(0xbc000000+SysConfRegAddr+0xF);

		for(i=0; i<SysConfLen; i++) SystemConfiguration[i]=ptr[i];
	}

	config=(u64*)SysConf;
	if((*config>>6&7)==0){
		*config&=0xFFFF02FFFFFFFFFF;
	}
}

static ConfigParam OSDConfig;

void SetOsdConfigParam(ConfigParam* config){
	OSDConfig.spdifMode=config->spdifMode;
	OSDConfig.screenType=config->screenType;
	OSDConfig.videoOutput=config->videoOutput;
	OSDConfig.japLanguage=config->japLanguage;
	OSDConfig.ps1drvConfig=config->ps1drvConfig;
	OSDConfig.region=config->region;
	OSDConfig.language=config->language;
	OSDConfig.timezoneOffset=config->timezoneOffset;
}

void GetOsdConfigParam(ConfigParam* config){
	config->spdifMode=OSDConfig.spdifMode;
	config->screenType=OSDConfig.screenType;
	config->videoOutput=OSDConfig.videoOutput;
	config->japLanguage=OSDConfig.japLanguage;
	config->ps1drvConfig=OSDConfig.ps1drvConfig;
	config->region=OSDConfig.region;
	config->language=OSDConfig.language;
	config->timezoneOffset=OSDConfig.timezoneOffset;
}

static unsigned char OSDConfig2[128];

void SetOsdConfigParam2(void* config, int size, int offset){
	unsigned int AmountToWrite, WriteEnd, i;
	unsigned char *ptr;

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
	unsigned char *ptr;

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

	return(((*(u64*)SystemConfiguration>>6&7)!=0)?*(u64*)SystemConfiguration>>44&0xF:0);
}

