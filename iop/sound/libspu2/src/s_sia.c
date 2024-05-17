/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libspu2_internal.h"

unsigned int SpuSetIRQAddr(unsigned int addr)
{
	if ( addr > 0x1FFFF8 )
		return 0;
	_spu_FsetRXX(206, addr, 1);
#ifdef LIB_OSD_100
	return 2 * addr;
#else
	// Added in OSDSND 110U (but not in libspu2 1600)
	return 2 * (addr >> 1);
#endif
}
