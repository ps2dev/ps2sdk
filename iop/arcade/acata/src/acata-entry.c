/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acata_internal.h"

#define MODNAME "ATA/ATAPI_driver"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// b4ae71c954a7435710d4be18af0c2ba0
// Known titles:
// NM00048
// NM00052
// Path strings:
// /home/kyota/psalm2hd/psalm-0.1.3/ata-iop-0.1.8/src/
// /home/kyota/psalm2hd/psalm-0.1.3/core-hdr-0.1.3/src/util/

extern struct irx_export_table _exp_acata;

#define acAtaEntry _start

int acAtaEntry(int argc, char **argv)
{
	int ret;

	ret = acAtaModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acata) != 0 )
		return -16;
	return 0;
}
