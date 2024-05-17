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

void SsSetRVol(s16 voll, s16 volr)
{
	SpuReverbAttr reverb_attr;

	reverb_attr.mask = SPU_REV_DEPTHL | SPU_REV_DEPTHR;
	reverb_attr.depth.left = (s16)(0x7FFF * voll) / 127;
	reverb_attr.depth.right = (s16)(0x7FFF * volr) / 127;
	SpuSetReverbDepth(&reverb_attr);
}
