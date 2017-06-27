/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <syscallnr.h>
#include <osd_config.h>

#include "osd.h"
#include "ExecPS2.h"

struct SyscallPatchData{
	unsigned int syscall;
	void *function;
};

static const struct SyscallPatchData SyscallPatchData[]={
	{ __NR_SetOsdConfigParam, &SetOsdConfigParam},
	{ __NR_GetOsdConfigParam, &GetOsdConfigParam},
	{ __NR_SetOsdConfigParam2, &SetOsdConfigParam2},
	{ __NR_GetOsdConfigParam2, &GetOsdConfigParam2},
	{ 0xFFFFC402, &ExecPS2Patch},
};

SystemConfiguration_t SystemConfiguration = { .EEGS=0x40};

int _start(int syscall) __attribute__((section(".start")));

int _start(int syscall){
	unsigned int i;
	int result;

	InitSystemConfig(&SystemConfiguration, 0x26);

	for(i=0; i<5; i++){
		if(SyscallPatchData[i].syscall==syscall){
			if(syscall==0xFFFFC402){
				result=((unsigned int)SyscallPatchData[i].function>>2&0x03FFFFFF)|0x0C000000;	//Creates a JAL instruction to the function.
			}
			else result=(unsigned int)SyscallPatchData[i].function;

			return result;
		}
	}

	return 0;
}
