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

void SpuGetCommonAttr(SpuCommonAttr *attr)
{
	s16 v2;
	const vu16 *v3;
	u16 v4;
	u16 v5;
	s16 v6;
	const vu16 *v11;
	const vu16 *v13;

	v2 = 0;
	v3 = &_spu_RXX[20 * _spu_core];
	v4 = v3[944];
	v5 = v3[945];
	v6 = 0;
	if ( (v4 & 0x8000) != 0 )
	{
		switch ( v4 & 0xF000 )
		{
			case 0x8000:
				v2 = SPU_VOICE_LINEARIncN;
				break;
			case 0x9000:
				v2 = SPU_VOICE_LINEARIncR;
				break;
			case 0xa000:
				v2 = SPU_VOICE_LINEARDecN;
				break;
			case 0xb000:
				v2 = SPU_VOICE_LINEARDecR;
				break;
			case 0xc000:
				v2 = SPU_VOICE_EXPIncN;
				break;
			case 0xd000:
				v2 = SPU_VOICE_EXPIncR;
				break;
			case 0xe000:
			case 0xf000:
				v2 = SPU_VOICE_EXPDec;
				break;
			default:
				break;
		}
		v4 &= ~0xF000;
	}
	if ( (v5 & 0x8000) != 0 )
	{
		switch ( v5 & 0xF000 )
		{
			case 0x8000:
				v6 = SPU_VOICE_LINEARIncN;
				break;
			case 0x9000:
				v6 = SPU_VOICE_LINEARIncR;
				break;
			case 0xa000:
				v6 = SPU_VOICE_LINEARDecN;
				break;
			case 0xb000:
				v6 = SPU_VOICE_LINEARDecR;
				break;
			case 0xc000:
				v6 = SPU_VOICE_EXPIncN;
				break;
			case 0xd000:
				v6 = SPU_VOICE_EXPIncR;
				break;
			case 0xe000:
			case 0xf000:
				v6 = SPU_VOICE_EXPDec;
				break;
			default:
				break;
		}
		v5 &= ~0xF000;
	}
	if ( v4 < 0x4000u )
		attr->mvol.left = v4;
	else
		attr->mvol.left = v4 + 0x8000;
	if ( v5 < 0x4000u )
		attr->mvol.right = v5;
	else
		attr->mvol.right = v5 + 0x8000;
	attr->mvolmode.right = v6;
	attr->mvolmode.left = v2;
	v11 = &_spu_RXX[20 * _spu_core];
	attr->mvolx.left = v11[972];
	attr->mvolx.right = v11[973];
	attr->cd.volume.left = v11[968];
	attr->cd.volume.right = v11[969];
	attr->cd.reverb = (_spu_RXX[512 * _spu_core + 205] & 4) != 0;
	attr->cd.mix = (_spu_RXX[512 * _spu_core + 205] & 1) != 0;
	v13 = &_spu_RXX[20 * _spu_core];
	attr->ext.volume.left = v13[970];
	attr->ext.volume.right = v13[971];
	attr->ext.reverb = (_spu_RXX[512 * _spu_core + 205] & 8) != 0;
	attr->ext.mix = (_spu_RXX[512 * _spu_core + 205] & 2) != 0;
}
