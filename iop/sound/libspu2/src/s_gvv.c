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

void SpuGetVoiceVolume(int v_num, s16 *voll, s16 *volr)
{
	const vu16 *v3;
	u16 v4;
	u16 v5;

	v3 = &_spu_RXX[512 * _spu_core + 8 * (v_num & 0x1F)];
	v4 = v3[1];
	v5 = v3[0];
	if ( v5 < 0x4000 )
		*voll = v5;
	else
		*voll = v5 + 0x8000;
	if ( v4 < 0x4000u )
		*volr = v4;
	else
		*volr = v4 + 0x8000;
}
