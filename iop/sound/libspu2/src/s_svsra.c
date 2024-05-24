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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceSRAttr(void)
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

void SpuSetVoiceSRAttr(int v_num, u16 sr, int sr_mode)
{
	s16 v3;
	int v4;

	v4 = 8 * v_num;
	switch ( sr_mode )
	{
		case SPU_VOICE_LINEARIncN:
			v3 = 0;
			break;
		case SPU_VOICE_EXPIncN:
			v3 = 512;
			break;
		case SPU_VOICE_EXPDec:
			v3 = 768;
			break;
		default:
			v3 = 256;
			break;
	}
	_spu_RXX[512 * _spu_core + 4 + v4] = (_spu_RXX[512 * _spu_core + 4 + v4] & 0x3F) | ((sr | (u16)v3) << 6);
	_spu_wait_SpuSetVoiceSRAttr();
}
