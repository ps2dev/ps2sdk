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

void SpuGetVoiceADSR(int v_num, u16 *ar, u16 *dr, u16 *sr, u16 *rr, u16 *sl)
{
	const vu16 *v6;
	u16 v7;
	unsigned int v8;

	v6 = &_spu_RXX[512 * _spu_core + 8 * (v_num & 0x1F)];
	v7 = v6[3];
	v8 = v6[4];
	*ar = (v7 >> 8) & 0x3F;
	*dr = (u8)(v7 & 0xF0) >> 4;
	*sr = (v8 >> 6) & 0x7F;
	*rr = v8 & 0x1F;
	*sl = v7 & 0xF;
}
