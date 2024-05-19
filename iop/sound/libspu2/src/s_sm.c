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

int SpuSetMute(int on_off)
{
	vu16 *v2;

	v2 = &_spu_RXX[512 * _spu_core];
	switch ( on_off )
	{
		case SPU_OFF:
			v2[205] |= 0x4000;
			break;
		case SPU_ON:
			v2[205] &= ~0x4000;
			break;
		default:
			break;
	}
	return on_off;
}
