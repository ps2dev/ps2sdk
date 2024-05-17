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

void SsGetRVol(SndVolume *r_vol)
{
	SpuReverbAttr reverb_attr;

	SpuGetReverbModeParam(&reverb_attr);
	r_vol->left = (s16)(127 * reverb_attr.depth.left) / 0x7FFF;
	r_vol->right = 127 * reverb_attr.depth.right / 0x7FFF;
}
