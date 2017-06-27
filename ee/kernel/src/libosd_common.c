/**
 * @file
 * libosd support functions
 *
 */

#include <kernel.h>
#include <syscallnr.h>
#include <osd_config.h>

#include "libosd.h"

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
