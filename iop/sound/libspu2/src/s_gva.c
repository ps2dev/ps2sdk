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

void SpuGetVoiceAttr(SpuVoiceAttr *arg)
{
	int v2;
	int v3;
	int v6;
	const vu16 *v7;
	u16 v8;
	u16 v9;
	s16 v10;
	s16 v12;
	const vu16 *v16;
	const vu16 *v21;

	v2 = -1;
	for ( v3 = 0; v3 < 24; v3 += 1 )
	{
		if ( (arg->voice & (1 << v3)) != 0 )
		{
			v2 = v3;
			break;
		}
	}
	v6 = 8 * v2;
	if ( v2 != -1 )
	{
		int v17;
		unsigned int v22;
		unsigned int v23;
		int v24;
		int v26;
		int v27;

		v7 = &_spu_RXX[512 * _spu_core + v6];
		v8 = *v7;
		v9 = v7[1];
		v10 = 0;
		if ( (v8 & 0x8000) != 0 )
		{
			switch ( v8 & 0xF000 )
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
			v8 &= ~0xF000;
		}
		v12 = 0;
		if ( (v9 & 0x8000) != 0 )
		{
			switch ( v9 & 0xF000 )
			{
				case 0x8000:
					v12 = SPU_VOICE_LINEARIncN;
					break;
				case 0x9000:
					v12 = SPU_VOICE_LINEARIncR;
					break;
				case 0xa000:
					v12 = SPU_VOICE_LINEARDecN;
					break;
				case 0xb000:
					v12 = SPU_VOICE_LINEARDecR;
					break;
				case 0xc000:
					v12 = SPU_VOICE_EXPIncN;
					break;
				case 0xd000:
					v12 = SPU_VOICE_EXPIncR;
					break;
				case 0xe000:
				case 0xf000:
					v12 = SPU_VOICE_EXPDec;
					break;
				default:
					break;
			}
			v9 &= ~0xF000;
		}
		if ( v8 < 0x4000u )
			arg->volume.left = v8;
		else
			arg->volume.left = v8 + 0x8000;
		if ( v9 < 0x4000u )
			arg->volume.right = v9;
		else
			arg->volume.right = v9 + 0x8000;
		arg->volmode.left = v10;
		arg->volmode.right = v12;
		v16 = &_spu_RXX[512 * _spu_core + v6];
		arg->volumex.left = v16[6];
		arg->volumex.right = v16[7];
		arg->pitch = v16[2];
		v17 = _spu_pitch2note(
			(_spu_voice_centerNote[_spu_core][v2] >> 8) & 0xFF, (u8)_spu_voice_centerNote[_spu_core][v2], arg->pitch);
		if ( v17 < 0 )
			arg->note = 0;
		else
			arg->note = v17;
		arg->sample_note = _spu_voice_centerNote[_spu_core][v2];
		arg->envx = _spu_RXX[512 * _spu_core + v6 + 5];
		arg->addr = _spu_MGFgetRXX2(224);
		arg->loop_addr = _spu_MGFgetRXX2(226);
		v21 = &_spu_RXX[512 * _spu_core + v6];
		v22 = v21[3];
		v23 = v21[4];
		v24 = SPU_VOICE_EXPIncN;
		if ( (v22 & 0x8000) == 0 )
			v24 = SPU_VOICE_LINEARIncN;
		arg->a_mode = v24;
		switch ( v23 & 0xE000 )
		{
			case 0xc000:
				v26 = SPU_VOICE_EXPDec;
				break;
			case 0x8000:
				v26 = SPU_VOICE_EXPIncN;
				break;
			case 0x4000:
				v26 = SPU_VOICE_LINEARDecN;
				break;
			default:
				v26 = SPU_VOICE_LINEARIncN;
				break;
		}
		arg->s_mode = v26;
		v27 = SPU_VOICE_EXPDec;
		if ( (v23 & 0x20) == 0 )
			v27 = SPU_VOICE_LINEARDecN;
		arg->r_mode = v27;
		arg->ar = (v22 >> 8) & 0x3F;
		arg->dr = (u8)(v22 & 0xF0) >> 4;
		arg->sr = (v23 >> 6) & 0x7F;
		arg->rr = v23 & 0x1F;
		arg->sl = v22 & 0xF;
		arg->adsr1 = v22;
		arg->adsr2 = v23;
	}
}
