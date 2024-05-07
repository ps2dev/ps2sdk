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

int SpuSetReverb(int on_off)
{
	vu16 *v1;

	switch ( on_off )
	{
		case SPU_OFF:
			v1 = &_spu_RXX[512 * _spu_core];
			_spu_rev_flag = SPU_OFF;
			v1[205] = v1[205] & ~0x80;
			break;
		case SPU_ON:
			v1 = &_spu_RXX[512 * _spu_core];
			_spu_rev_flag = SPU_ON;
			v1[205] = v1[205] | 0x80;
			break;
		default:
			break;
	}
	return _spu_rev_flag;
}
