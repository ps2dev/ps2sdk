#include <kernel.h>
#include <loadfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "HDDSupport.h"

static volatile unsigned int HDDLoadStatArea[4] ALIGNED(16);
static unsigned char IsHddSupportEnabled=0, HddSupportStatus=0;

void CheckHDDUpdate(int device, const char *SysExecFolder){
	char CmdStr[34], ModulePath[40];

	sprintf(ModulePath, "mc%u:/%s/dev9.irx", device, SysExecFolder);
	if(SifLoadModule(ModulePath, 0, NULL)>=0){
		sprintf(ModulePath, "mc%u:/%s/atad.irx", device, SysExecFolder);
		if(SifLoadModule(ModulePath, 0, NULL)>=0){
			strcpy(CmdStr, "-osd");
			strcpy(&CmdStr[5], "0x00100000");
			strcpy(&CmdStr[16], "-stat");
			sprintf(&CmdStr[22], "%p", HDDLoadStatArea);
			CmdStr[sizeof(CmdStr)-1]='\0';

			sprintf(ModulePath, "mc%u:/%s/hddload.irx", device, SysExecFolder);
			if(SifLoadModule(ModulePath, sizeof(CmdStr), CmdStr)>=0){
				IsHddSupportEnabled = 1;
			}
		}
	}
}

int GetHddSupportEnabledStatus(void){
	return IsHddSupportEnabled;
}

int GetHddUpdateStatus(void){
	return HddSupportStatus;
}

void DetermineHDDLoadStatus(void){
	int result;

	if((result=*(int *)UNCACHED_SEG(HDDLoadStatArea))==1){
		HddSupportStatus = 1;
	}
	else if(result<0){
		IsHddSupportEnabled = 0;
	}
}
