/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

void SsSetMVol(s16 voll, s16 volr)
{
	SpuCommonAttr spu_attr;

	spu_attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
	spu_attr.mvol.left = 129 * voll;
	spu_attr.mvol.right = 129 * volr;
	SpuSetCommonAttr(&spu_attr);
}
