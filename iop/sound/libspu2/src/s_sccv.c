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

void SpuSetCommonCDVolume(s16 cd_left, s16 cd_right)
{
	vu16 *v2;

	v2 = &_spu_RXX[20 * _spu_core];
	v2[968] = cd_left;
	v2[969] = cd_right;
}
