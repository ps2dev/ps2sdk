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

void SpuGetVoiceVolumeX(int v_num, s16 *voll_x, s16 *volr_x)
{
	int v3;

	v3 = 8 * v_num;
	*voll_x = _spu_RXX[512 * _spu_core + 6 + v3];
	*volr_x = _spu_RXX[512 * _spu_core + 7 + v3];
}
