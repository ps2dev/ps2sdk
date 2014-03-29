#include <errno.h>
#include <stdio.h>
#include <loadcore.h>
#include <thbase.h>
#include <irx.h>

#include "main.h"
#include "xfer.h"

IRX_ID("SMAP_ETH_driver", 0x00, 0x70);

extern struct irx_export_table _exp_smap;

extern const char VersionString[];

int _start(int argc, char *argv[]){
	int result;

	if(RegisterLibraryEntries(&_exp_smap)!=0){
		DEBUG_PRINTF("smap: module already loaded\n");
		return MODULE_NO_RESIDENT_END;
	}

	printf("SMAP (%s)\n", VersionString);

	if((result=smap_init(argc, argv))<0){
		DEBUG_PRINTF("smap: smap_init -> %d\n", result);
		ReleaseLibraryEntries(&_exp_smap);
		return MODULE_NO_RESIDENT_END;
	}

	return MODULE_RESIDENT_END;
}

