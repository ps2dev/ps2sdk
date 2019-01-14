#include <errno.h>
#include <stdio.h>
#include <loadcore.h>
#include <thbase.h>
#include <irx.h>
#include <netman.h>

#include "main.h"
#include "xfer.h"

IRX_ID("SMAP_driver", 0x2, 0x19);

//While the header of the export table is small, the large size of the export table (as a whole) places it in data instead of sdata.
extern struct irx_export_table _exp_smap __attribute__((section("data")));

int _start(int argc, char *argv[]){
	int result;

	if(RegisterLibraryEntries(&_exp_smap)!=0){
		DEBUG_PRINTF("smap: module already loaded\n");
		return MODULE_NO_RESIDENT_END;
	}

	DisplayBanner();

/*	This code was present in SMAP, but cannot be implemented with the default IOP kernel due to MODLOAD missing these functions.
	It may be necessary to prevent SMAP from linking with an old DEV9 module.
	if((ModuleID=SearchModuleByName("dev9"))<0){
		sceInetPrintf("smap: dev9 module not found\n");
		return MODULE_NO_RESIDENT_END;
	}
	if(ReferModuleStatus(ModuleID, &ModStatus)<0){
		sceInetPrintf("smap: can't get dev9 module status\n");
		return MODULE_NO_RESIDENT_END;
	}

	if(ModStatus.version<0x204){
		sceInetPrintf("smap: dev9 module version must be 2.4 or later\n");
		return MODULE_NO_RESIDENT_END;
	} */

	if((result=smap_init(argc, argv))<0){
		DEBUG_PRINTF("smap: smap_init -> %d\n", result);
		ReleaseLibraryEntries(&_exp_smap);
		return MODULE_NO_RESIDENT_END;
	}

	return MODULE_RESIDENT_END;
}

