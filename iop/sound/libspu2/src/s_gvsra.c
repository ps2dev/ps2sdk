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

void SpuGetVoiceSRAttr(int v_num, u16 *sr, int *sr_mode)
{
	u16 v3;

	v3 = _spu_RXX[512 * _spu_core + 4 + 8 * (v_num & 0x1F)];
	*sr = (v3 >> 6) & 0x7F;
	switch ( v3 & 0xE000 )
	{
		case 0xc000:
			*sr_mode = SPU_VOICE_EXPDec;
			break;
		case 0x8000:
			*sr_mode = SPU_VOICE_EXPIncN;
			break;
		case 0x4000:
			*sr_mode = SPU_VOICE_LINEARDecN;
			break;
		default:
			*sr_mode = SPU_VOICE_LINEARIncN;
			break;
	}
}
