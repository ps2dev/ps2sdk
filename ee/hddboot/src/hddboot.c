#include <kernel.h>
#include <loadfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "hdd_boot.h"

static volatile unsigned int HDDLoadStatArea[4] ALIGNED(16);
static unsigned char IsHddSupportEnabled=0, HddSupportStatus=0;
char CmdStr[34];

static void construct_params() {
	memset(&CmdStr, 0, sizeof(CmdStr));
	strcpy(CmdStr, "-osd");
	strcpy(&CmdStr[5], "0x00100000");
	strcpy(&CmdStr[16], "-stat");
	sprintf(&CmdStr[22], "%p", HDDLoadStatArea);
	CmdStr[sizeof(CmdStr)-1]='\0';
}

int BootHDDLoadBuffer(void* irx, unsigned int size, int* ret){
	int id, rett;
	construct_params();
	id = SifExecModuleBuffer(irx, size, sizeof(CmdStr), CmdStr, &rett);
	if (id > 0 && rett != 1) IsHddSupportEnabled = 1;
	if (ret != NULL) *ret = rett;
	return id;
}

int BootHDDLoadFile(char* path, int* ret){
	int id, rett;
	construct_params();
	id = SifLoadStartModule(path, sizeof(CmdStr), CmdStr, &rett);
	if (id > 0 && rett != 1) IsHddSupportEnabled = 1;
	if (ret != NULL) *ret = rett;
	return id;
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
