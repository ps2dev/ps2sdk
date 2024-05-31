/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acram_internal.h"

#define MODNAME "Arcade_Ext._Memory"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// b60694e19a187a25b87084cdcd939e1c
// Known titles:
// NM00004
// NM00006
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/ram-iop-0.1.4/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 3b4ef6456a23411e40d6e542b89ae0af

extern struct irx_export_table _exp_acram;

#define acRamEntry _start

int acRamEntry(int argc, char **argv)
{
	int ret;

	ret = acRamModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acram) != 0 )
		return -16;
	return 0;
}
