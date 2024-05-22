/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acflash_internal.h"

#define MODNAME "Arcade_Flash_Memory"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 83c44942d316812471a189fd2015bbc9
// Known titles:
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/flash-iop-0.1.2/src/
// TODO: diff with module text hash 1437cb87136e2564482cd3f609ab4023

extern struct irx_export_table _exp_acflash;

#define acFlashEntry _start

int acFlashEntry(int argc, char **argv)
{
	int ret;

	ret = acFlashModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries((struct irx_export_table *)&_exp_acflash) != 0 )
		return -16;
	return 0;
}
