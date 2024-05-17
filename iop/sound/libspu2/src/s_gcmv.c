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

void SpuGetCommonMasterVolume(s16 *mvol_left, s16 *mvol_right)
{
	const vu16 *v2;
	u16 v3;
	u16 v4;

	v2 = &_spu_RXX[20 * _spu_core];
	v3 = v2[944];
	v4 = v2[945];
	if ( (v3 & 0x8000) != 0 )
		v3 &= ~0xF000;
	if ( (v2[945] & 0x8000) != 0 )
		v4 &= ~0xF000;
	if ( v3 < 0x4000u )
		*mvol_left = v3;
	else
		*mvol_left = v3 + 0x8000;
	if ( v4 < 0x4000u )
		*mvol_right = v4;
	else
		*mvol_right = v4 + 0x8000;
}
