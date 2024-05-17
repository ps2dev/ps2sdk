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

void SpuGetVoiceVolumeAttr(int v_num, s16 *voll, s16 *volr, s16 *voll_mode, s16 *volr_mode)
{
	const vu16 *v5;
	u16 v6;
	u16 v7;
	s16 v8;
	s16 v10;

	v5 = &_spu_RXX[512 * _spu_core + 8 * (v_num & 0x1F)];
	v6 = *v5;
	v7 = v5[1];
	v8 = 0;
	if ( (v6 & 0x8000) != 0 )
	{
		switch ( v6 & 0xF000 )
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
		v6 &= ~0xF000;
	}
	v10 = 0;
	if ( (v7 & 0x8000) != 0 )
	{
		switch ( v7 & 0xF000 )
		{
			case 0x8000:
				v10 = SPU_VOICE_LINEARIncN;
				break;
			case 0x9000:
				v10 = SPU_VOICE_LINEARIncR;
				break;
			case 0xa000:
				v10 = SPU_VOICE_LINEARDecN;
				break;
			case 0xb000:
				v10 = SPU_VOICE_LINEARDecR;
				break;
			case 0xc000:
				v10 = SPU_VOICE_EXPIncN;
				break;
			case 0xd000:
				v10 = SPU_VOICE_EXPIncR;
				break;
			case 0xe000:
			case 0xf000:
				v10 = SPU_VOICE_EXPDec;
				break;
			default:
				break;
		}
		v7 &= ~0xF000;
	}
	*voll_mode = v8;
	*volr_mode = v10;
	if ( v6 < 0x4000u )
		*voll = v6;
	else
		*voll = v6 + 0x8000;
	if ( v7 < 0x4000u )
		*volr = v7;
	else
		*volr = v7 + 0x8000;
}
