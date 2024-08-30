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

unsigned int SpuSetTransferStartAddr(unsigned int addr)
{
	if ( addr - 0x5010 > 0x1FAFE8 )
		return 0;
	_spu_tsa[1] = addr >> 1;
	return 2 * (addr >> 1);
}
