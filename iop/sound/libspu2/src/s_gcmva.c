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

void SpuGetCommonMasterVolumeAttr(s16 *mvol_left, s16 *mvol_right, s16 *mvolmode_left, s16 *mvolmode_right)
{
	s16 v4;
	const vu16 *v5;
	u16 v6;
	u16 v7;
	s16 v8;

	v4 = 0;
	v5 = &_spu_RXX[20 * _spu_core];
	v6 = v5[944];
	v7 = v5[945];
	v8 = 0;
	if ( (v6 & 0x8000) != 0 )
	{
		switch ( v6 & 0xF000 )
		{
			case 0x8000:
				v4 = SPU_VOICE_LINEARIncN;
				break;
			case 0x9000:
				v4 = SPU_VOICE_LINEARIncR;
				break;
			case 0xa000:
				v4 = SPU_VOICE_LINEARDecN;
				break;
			case 0xb000:
				v4 = SPU_VOICE_LINEARDecR;
				break;
			case 0xc000:
				v4 = SPU_VOICE_EXPIncN;
				break;
			case 0xd000:
				v4 = SPU_VOICE_EXPIncR;
				break;
			case 0xe000:
			case 0xf000:
				v4 = SPU_VOICE_EXPDec;
				break;
			default:
				break;
		}
		v6 &= ~0xF000;
	}
	v8 = 0;
	if ( (v7 & 0x8000) != 0 )
	{
		switch ( v7 & 0xF000 )
		{
			case 0x8000:
				v8 = SPU_VOICE_LINEARIncN;
				break;
			case 0x9000:
				v8 = SPU_VOICE_LINEARIncR;
				break;
			case 0xa000:
				v8 = SPU_VOICE_LINEARDecN;
				break;
			case 0xb000:
				v8 = SPU_VOICE_LINEARDecR;
				break;
			case 0xc000:
				v8 = SPU_VOICE_EXPIncN;
				break;
			case 0xd000:
				v8 = SPU_VOICE_EXPIncR;
				break;
			case 0xe000:
			case 0xf000:
				v8 = SPU_VOICE_EXPDec;
				break;
			default:
				break;
		}
		v7 &= ~0xF000;
	}
	*mvolmode_left = v4;
	*mvolmode_right = v8;
	if ( v6 < 0x4000u )
		*mvol_left = v6;
	else
		*mvol_left = v6 + 0x8000;
	if ( v7 < 0x4000u )
		*mvol_right = v7;
	else
		*mvol_right = v7 + 0x8000;
}
