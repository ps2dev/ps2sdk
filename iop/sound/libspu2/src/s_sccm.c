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

void SpuSetCommonCDMix(int cd_mix)
{
	vu16 *v1;
	u16 v2;

	v1 = &_spu_RXX[512 * _spu_core];
	if ( cd_mix )
		v2 = v1[205] | 1;
	else
		v2 = v1[205] & ~1;
	v1[205] = v2;
}
