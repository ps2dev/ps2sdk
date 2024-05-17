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

void SsGetMVol(SndVolume *m_vol)
{
	SpuCommonAttr spu_attr;

	SpuGetCommonAttr(&spu_attr);
	m_vol->left = spu_attr.mvol.left / 129;
	m_vol->right = spu_attr.mvol.right / 129;
}
