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

void SpuSetCommonMasterVolumeAttr(s16 mvol_left, s16 mvol_right, s16 mvolmode_left, s16 mvolmode_right)
{
	s16 v4;
	s16 v5;
	int v6;
	int v7;

	v4 = mvol_left;
	v5 = mvol_right;
	v6 = 0;
	v7 = 0;
	switch ( mvolmode_left )
	{
		case SPU_VOICE_LINEARIncN:
			v6 = 0x8000;
			break;
		case SPU_VOICE_LINEARIncR:
			v6 = 0x9000;
			break;
		case SPU_VOICE_LINEARDecN:
			v6 = 0xa000;
			break;
		case SPU_VOICE_LINEARDecR:
			v6 = 0xb000;
			break;
		case SPU_VOICE_EXPIncN:
			v6 = 0xc000;
			break;
		case SPU_VOICE_EXPIncR:
			v6 = 0xd000;
			break;
		case SPU_VOICE_EXPDec:
			v6 = 0xe000;
			break;
		default:
			break;
	}
	if ( v6 )
	{
		v4 = 127;
		if ( mvol_left < 128 )
		{
			v4 = mvol_left;
			if ( mvol_left < 0 )
				v4 = 0;
		}
	}
	_spu_RXX[20 * _spu_core + 944] = (v4 & ~0x8000) | v6;
	switch ( mvolmode_right )
	{
		case SPU_VOICE_LINEARIncN:
			v7 = 0x8000;
			break;
		case SPU_VOICE_LINEARIncR:
			v7 = 0x9000;
			break;
		case SPU_VOICE_LINEARDecN:
			v7 = 0xa000;
			break;
		case SPU_VOICE_LINEARDecR:
			v7 = 0xb000;
			break;
		case SPU_VOICE_EXPIncN:
			v7 = 0xc000;
			break;
		case SPU_VOICE_EXPIncR:
			v7 = 0xd000;
			break;
		case SPU_VOICE_EXPDec:
			v7 = 0xe000;
			break;
		default:
			break;
	}
	if ( v7 )
	{
		v5 = 127;
		if ( mvol_right < 128 )
		{
			v5 = mvol_right;
			if ( mvol_right < 0 )
				v5 = 0;
		}
	}
	_spu_RXX[20 * _spu_core + 945] = (v5 & ~0x8000) | v7;
}
