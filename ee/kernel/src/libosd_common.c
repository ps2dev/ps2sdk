/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * libosd support functions
 *
 */

#include <kernel.h>
#include <syscallnr.h>
#include <osd_config.h>

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
