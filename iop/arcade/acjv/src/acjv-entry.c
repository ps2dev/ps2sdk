/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acjv_internal.h"

#define MODNAME "Arcade_JV"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 8a8acf790934d8eb0dffdd8b57c06374
// Known titles:
// NM00005
// NM00008
// Path strings:
// /usr/local/sys246/psalm-0.1.3/jv-iop-0.1.4/src/
// TODO: diff with module text hash cda3a00a245ec5ea8c478da1b1743e3b

extern struct irx_export_table _exp_acjv;

#define acJvEntry _start

int acJvEntry(int argc, char **argv)
{
	int ret;

	ret = acJvModuleStart(argc, argv);
	// cppcheck-suppress knownConditionTrueFalse
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acjv) != 0 )
		return -16;
	return 0;
}
