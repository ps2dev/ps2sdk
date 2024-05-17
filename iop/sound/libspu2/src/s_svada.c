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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceADSRAttr(void)
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

void SpuSetVoiceADSRAttr(int v_num, u16 ar, u16 dr, u16 sr, u16 rr, u16 sl, int ar_mode, int sr_mode, int rr_mode)
{
	s16 v9;
	s16 v10;
	int v11;
	s16 v12;
	u16 v13;

	v9 = ((ar & 0x7F) << 8) | (16 * (dr & 0xF));
	v10 = ((sr & 0x7F) << 6) | (rr & 0x1F);
	v11 = 8 * v_num;
	_spu_RXX[512 * _spu_core + 3 + 8 * v_num] = v9 | (sl & 0xF) | ((ar_mode == SPU_VOICE_EXPIncN) << 15);
	switch ( sr_mode )
	{
		case SPU_VOICE_LINEARIncN:
			v12 = 0;
			break;
		case SPU_VOICE_EXPIncN:
			v12 = 0x8000;
			break;
		case SPU_VOICE_EXPDec:
			v12 = 0xC000;
			break;
		default:
			v12 = 0x4000;
			break;
	}
	v13 = v10 | v12;
	if ( rr_mode == SPU_VOICE_EXPDec )
		v13 = v10 | v12 | 0x20;
	_spu_RXX[512 * _spu_core + 4 + v11] = v13;
	_spu_wait_SpuSetVoiceADSRAttr();
}
