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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceARAttr(void)
{
	int i;
	int v1;

	v1 = 1;
	for ( i = 0; i < 2; i += 1 )
	{
		v1 *= 13;
		__asm__ __volatile__("" : "+g"(v1) : :);
	}
}

void SpuSetVoiceARAttr(int v_num, u16 ar, int ar_mode)
{
	vu16 *v3;

	v3 = &_spu_RXX[512 * _spu_core + 8 * v_num];
	v3[3] = (u8)v3[3] | (u16)((ar | ((ar_mode == SPU_VOICE_EXPIncN) << 7)) << 8);
	_spu_wait_SpuSetVoiceARAttr();
}
