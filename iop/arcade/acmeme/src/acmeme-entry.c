/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acmeme_internal.h"

#define MODNAME "Arcade_Memory_Access/EE"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 6059ca4ccd258e0d6b07a7da46c41c19
// Known titles:
// NM00004
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/mem-iop-0.1.4/src/
// TODO: diff with module text hash 26954e9302f9499714676e156a60cdde

extern struct irx_export_table _exp_acmeme;

#define acMemeEntry _start

int acMemeEntry(int argc, char **argv)
{
	int ret;

	ret = acMemeModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acmeme) != 0 )
		return -16;
	return 0;
}
