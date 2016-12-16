/*
	libkrnlupd	- Kernel updates library.

	Contains updates for the Playstation 2's "Protokernel" EE kernel.

	The only known consoles to have a "Protokernel" are the SCPH-10000 and SCPH-15000. Both contain either boot ROM v1.00 or v1.01.

	Note that these kernels are not necessarily buggy, but were based on an older set of specifications. This file contains patches that will "modernize" these kernels until a hard reset.
	(Code was based on the official Sony "libosd" patch)
*/

#include <kernel.h>
#include <syscallnr.h>
#include <osd_config.h>

#include "libkrnlupd.h"

int kCopy(u32 *dest, const u32 *src, int size){
	unsigned int i;

	if(size>>2){
		for(i=0; i<size; i+=4,dest++,src++){
			*dest=*src;
		}
	}

	return 0;
}

int PatchIsNeeded(void){
	ConfigParam original_config, config;

	GetOsdConfigParam(&original_config);
	config=original_config;
	config.version=1;	//Protokernels cannot retain values set in this field.
	SetOsdConfigParam(&config);
	GetOsdConfigParam(&config);
	SetOsdConfigParam(&original_config);

	return(config.version<1);
}
