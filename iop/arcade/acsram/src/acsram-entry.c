/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acsram_internal.h"

#define MODNAME "Arcade_SRAM"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 9678b9c8dededca0a021cc70826d49d4
// Known titles:
// NM00004
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/sram-iop-0.1.4/src/
// TODO: diff with module text hash 747b6206ea65e1fe9b9944b74e7e1b51

extern struct irx_export_table _exp_acsram;

#define acSramEntry _start

int acSramEntry(int argc, char **argv)
{
	int ret;

	ret = acSramModuleStart(argc, argv);
	// cppcheck-suppress knownConditionTrueFalse
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acsram) != 0 )
		return -16;
	return 0;
}
