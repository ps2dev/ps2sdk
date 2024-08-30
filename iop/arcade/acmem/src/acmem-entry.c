/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acmem_internal.h"

#define MODNAME "Arcade_Memory_Access"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 697bf204c8fe9f91d78cf327c2247f48
// Known titles:
// NM00004
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/mem-iop-0.1.4/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 7a8be932b87c8e6ef19a48318cce1e0b

extern struct irx_export_table _exp_acmem;

#define acMemEntry _start

int acMemEntry(int argc, char **argv)
{
	int ret;

	ret = acMemModuleStart(argc, argv);
	// cppcheck-suppress knownConditionTrueFalse
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acmem) != 0 )
		return -16;
	return 0;
}
