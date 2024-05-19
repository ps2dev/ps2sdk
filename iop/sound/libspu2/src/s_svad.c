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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceADSR(void)
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

void SpuSetVoiceADSR(int v_num, u16 ar, u16 dr, u16 sr, u16 rr, u16 sl)
{
	vu16 *v6;

	v6 = &_spu_RXX[512 * _spu_core + 8 * v_num];
	v6[3] = ((ar & 0x7F) << 8) | (16 * (dr & 0xF)) | (sl & 0xF);
	v6[4] = ((sr & 0x7F) << 6) | (rr & 0x1F) | 0x4000;
	_spu_wait_SpuSetVoiceADSR();
}
