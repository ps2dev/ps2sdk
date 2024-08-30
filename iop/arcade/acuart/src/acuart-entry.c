/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acuart_internal.h"

#define MODNAME "UART_driver"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// efa5399dc7ebb43087db51b82e865f64
// Known titles:
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/uart-iop-0.1.3/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 7ceac5800a29a7c32cde41d0b7faa64d

extern struct irx_export_table _exp_acuart;

#define acUartEntry _start

int acUartEntry(int argc, char **argv)
{
	int ret;

	ret = acUartModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_acuart) != 0 )
		return -16;
	return 0;
}
