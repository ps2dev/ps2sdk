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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceSR(void)
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

void SpuSetVoiceSR(int v_num, u16 sr)
{
	vu16 *v2;

	v2 = &_spu_RXX[512 * _spu_core + 8 * v_num];
	v2[4] = (v2[4] & 0x3F) | ((sr | 0x100) << 6);
	_spu_wait_SpuSetVoiceSR();
}

// libspu2/s_svrro

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceRR(void)
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

void SpuSetVoiceRR(int v_num, u16 rr)
{
	vu16 *v2;

	v2 = &_spu_RXX[512 * _spu_core + 8 * v_num];
	v2[4] = (v2[4] & ~0x3f) | rr;
	_spu_wait_SpuSetVoiceRR();
}
