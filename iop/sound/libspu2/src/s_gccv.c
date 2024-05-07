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

void SpuGetCommonCDVolume(s16 *cd_left, s16 *cd_right)
{
	*cd_left = _spu_RXX[20 * _spu_core + 968];
	*cd_right = _spu_RXX[20 * _spu_core + 969];
}
