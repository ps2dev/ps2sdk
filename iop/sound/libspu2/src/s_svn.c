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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceNote(void)
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

void SpuSetVoiceNote(int v_num, u16 note)
{
	unsigned int v4;

	v4 = (u16)_spu_voice_centerNote[_spu_core][v_num];
	_spu_RXX[512 * _spu_core + 2 + 8 * v_num] = _spu_note2pitch(v4 >> 8, (u8)v4, (u16)(note & 0xFF00) >> 8, (u8)note);
	_spu_wait_SpuSetVoiceNote();
}
