/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accdvde_internal.h"

#define MODNAME "CD/DVD_Compatible/EE"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 3adc67543b57ea59a55923c48202b7a8
// Known titles:
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/cdvd-iop-0.1.9/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 13ec89dd2db83c3fe3c667251c260f77

extern struct irx_export_table _exp_accdvde;

#define acCdvdeEntry _start

int acCdvdeEntry(int argc, char **argv)
{
	int ret;

	ret = acCdvdeModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_accdvde) != 0 )
		return -16;
	return 0;
}
