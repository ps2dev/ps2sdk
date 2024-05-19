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

void SpuNGetVoiceAttr(int v_num, SpuVoiceAttr *arg)
{
	s16 v3;
	int v4;
	const vu16 *v5;
	u16 v6;
	u16 v7;
	s16 v10;
	const vu16 *v14;
	int v15;
	const vu16 *v19;
	unsigned int v20;
	unsigned int v21;
	int v22;
	int v24;
	int v25;

	v3 = 0;
	v4 = 8 * v_num;
	v5 = &_spu_RXX[512 * _spu_core + 8 * v_num];
	v6 = *v5;
	v7 = v5[1];
	if ( (v6 & 0x8000) != 0 )
	{
		switch ( v6 & 0xF000 )
		{
			case 0x8000:
				v3 = SPU_VOICE_LINEARIncN;
				break;
			case 0x9000:
				v3 = SPU_VOICE_LINEARIncR;
				break;
			case 0xa000:
				v3 = SPU_VOICE_LINEARDecN;
				break;
			case 0xb000:
				v3 = SPU_VOICE_LINEARDecR;
				break;
			case 0xc000:
				v3 = SPU_VOICE_EXPIncN;
				break;
			case 0xd000:
				v3 = SPU_VOICE_EXPIncR;
				break;
			case 0xe000:
			case 0xf000:
				v3 = SPU_VOICE_EXPDec;
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
	if ( v6 < 0x4000u )
		arg->volume.left = v6;
	else
		arg->volume.left = v6 + 0x8000;
	if ( v7 < 0x4000u )
		arg->volume.right = v7;
	else
		arg->volume.right = v7 + 0x8000;
	arg->volmode.left = v3;
	arg->volmode.right = v10;
	v14 = &_spu_RXX[512 * _spu_core + v4];
	arg->volumex.left = v14[6];
	arg->volumex.right = v14[7];
	arg->pitch = v14[2];
	v15 = _spu_pitch2note(
		(_spu_voice_centerNote[_spu_core][v_num] >> 8) & 0xFF, (u8)_spu_voice_centerNote[_spu_core][v_num], arg->pitch);
	if ( v15 < 0 )
		arg->note = 0;
	else
		arg->note = v15;
	arg->sample_note = _spu_voice_centerNote[_spu_core][v_num];
	arg->envx = _spu_RXX[512 * _spu_core + v4 + 5];
	arg->addr = _spu_MGFgetRXX2(224);
	arg->loop_addr = _spu_MGFgetRXX2(226);
	v19 = &_spu_RXX[512 * _spu_core + v4];
	v20 = v19[3];
	v21 = v19[4];
	v22 = SPU_VOICE_EXPIncN;
	if ( (v20 & 0x8000) == 0 )
		v22 = SPU_VOICE_LINEARIncN;
	arg->a_mode = v22;
	switch ( v21 & 0xE000 )
	{
		case 0xc000:
			v24 = SPU_VOICE_EXPDec;
			break;
		case 0x8000:
			v24 = SPU_VOICE_EXPIncN;
			break;
		case 0x4000:
			v24 = SPU_VOICE_LINEARDecN;
			break;
		default:
			v24 = SPU_VOICE_LINEARIncN;
			break;
	}
	arg->s_mode = v24;
	v25 = SPU_VOICE_EXPDec;
	if ( (v21 & 0x20) == 0 )
		v25 = SPU_VOICE_LINEARDecN;
	arg->r_mode = v25;
	arg->ar = (v20 >> 8) & 0x3F;
	arg->dr = (u8)(v20 & 0xF0) >> 4;
	arg->sr = (v21 >> 6) & 0x7F;
	arg->rr = v21 & 0x1F;
	arg->sl = v20 & 0xF;
	arg->adsr1 = v20;
	arg->adsr2 = v21;
}
