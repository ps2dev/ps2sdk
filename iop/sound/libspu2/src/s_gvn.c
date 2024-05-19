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

void SpuGetVoiceNote(int v_num, u16 *note)
{
	unsigned int v3;
	int v4;

	v3 = (u16)_spu_voice_centerNote[_spu_core][v_num];
	v4 = _spu_pitch2note(v3 >> 8, (u8)v3, _spu_RXX[512 * _spu_core + 2 + 8 * (v_num & 0x1F)]);
	if ( v4 < 0 )
		*note = 0;
	else
		*note = v4;
}
