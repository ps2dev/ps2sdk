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

int SpuReserveReverbWorkArea(int on_off)
{
	switch ( on_off )
	{
		case SPU_ON:
			_spu_rev_reserve_wa = SPU_ON;
			break;
		case SPU_OFF:
		default:
			_spu_rev_reserve_wa = SPU_OFF;
			break;
	}
	return _spu_rev_reserve_wa;
}
