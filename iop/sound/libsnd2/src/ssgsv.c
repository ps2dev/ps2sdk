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

void SsGetSerialVol(char s_num, SndVolume *s_vol)
{
	SpuCommonAttr spu_attr;

	SpuGetCommonAttr(&spu_attr);
	switch ( s_num )
	{
		case SS_SERIAL_A:
			s_vol->left = spu_attr.cd.volume.left / 258;
			s_vol->right = (u16)spu_attr.cd.volume.right / 258;
			break;
		case SS_SERIAL_B:
			s_vol->left = spu_attr.ext.volume.left / 258;
			s_vol->right = (u16)spu_attr.ext.volume.right / 258;
			break;
		default:
			break;
	}
}
