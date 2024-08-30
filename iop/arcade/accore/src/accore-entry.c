/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accore_internal.h"

#define MODNAME "Arcade_Core"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// cf99d72936452b3070371cc2c3cbe883
// Known titles:
// NM00004
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/core-iop-0.1.6/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 10cd5c69591445ffdde8468d75dd031b

extern struct irx_export_table _exp_accore;

#define acCoreEntry _start

int acCoreEntry(int argc, char **argv)
{
	unsigned int v4;
	acCoreInit *v5;
	int ret_v4;
	acCoreInit inits[3];

	v4 = 0;
	v5 = inits;
	inits[0] = acDev9ModuleStart;
	inits[1] = acIntrModuleStart;
	inits[2] = acDmaModuleStart;
	while ( 1 )
	{
		int ret;
		int v7;

		ret = (*v5)(argc, argv);
		v7 = ret;
		if ( ret < 0 )
		{
			printf("accore:init_start:%d: error %d\n", (int)v4, ret);
			ret_v4 = v7;
			break;
		}
		++v4;
		++v5;
		if ( v4 >= 3 )
		{
			ret_v4 = 0;
			break;
		}
	}
	if ( ret_v4 < 0 )
	{
		return ret_v4;
	}
	if ( RegisterLibraryEntries(&_exp_accore) != 0 )
		return -16;
	return 0;
}
