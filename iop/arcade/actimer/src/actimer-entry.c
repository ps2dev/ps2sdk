/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "actimer_internal.h"

#define MODNAME "simple_timer"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 6d5f9b91cc1ce01e895297ea9f8ae6b4
// Known titles:
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/timer-iop-0.1.4/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 0f77412b636a0393ccd6848c86ce5011

extern struct irx_export_table _exp_actimer;

#define acTimerEntry _start

int acTimerEntry(int argc, char **argv)
{
	int ret;

	ret = acTimerModuleStart(argc, argv);
	if ( ret < 0 )
	{
		return ret;
	}
	if ( RegisterLibraryEntries(&_exp_actimer) != 0 )
		return -16;
	return 0;
}
