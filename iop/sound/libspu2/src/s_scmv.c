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

void SpuSetCommonMasterVolume(s16 mvol_left, s16 mvol_right)
{
	vu16 *v2;

	v2 = &_spu_RXX[20 * _spu_core];
	v2[944] = mvol_left & ~0x8000;
	v2[945] = mvol_right & ~0x8000;
}
