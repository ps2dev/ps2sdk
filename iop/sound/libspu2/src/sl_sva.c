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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuLSetVoiceAttr(void)
{
	int i;
	int v1;

	v1 = 1;
	for ( i = 0; i < 2; i += 1 )
	{
		v1 *= 13;
		__asm__ __volatile__("" : "+g"(v1) : :);
	}
}

void SpuLSetVoiceAttr(int num, SpuLVoiceAttr *arg_list)
{
	int v3;
	unsigned int v9;
	int v10;
	s16 v11;
	int v12;
	int v13;
	s16 v14;
	int v15;
	unsigned int v16;
	s16 v17;
	unsigned int v18;
	unsigned int v19;
	s16 v20;
	unsigned int v22;
	s16 v23;
	unsigned int v25;

	for ( v3 = 0; v3 < num; v3 += 1 )
	{
		int attr_mask;
		int voiceNum;
		int v7;
		int v8;

		attr_mask = arg_list[v3].attr.mask;
		if ( attr_mask == 0 )
		{
			attr_mask = 0xFFFFFFFF;
		}
		voiceNum = arg_list[v3].voiceNum;
		v7 = 8 * voiceNum;
		v8 = 6 * voiceNum;
		if ( (attr_mask & SPU_VOICE_PITCH) != 0 )
			_spu_RXX[512 * _spu_core + 2 + v7] = arg_list[v3].attr.pitch;
		if ( (attr_mask & SPU_VOICE_SAMPLE_NOTE) != 0 )
			_spu_voice_centerNote[_spu_core][voiceNum] = arg_list[v3].attr.sample_note;
		if ( (attr_mask & SPU_VOICE_NOTE) != 0 )
		{
			v9 = (u16)_spu_voice_centerNote[_spu_core][voiceNum];
			_spu_RXX[512 * _spu_core + 2 + v7] =
				_spu_note2pitch(v9 >> 8, (u8)v9, ((arg_list[v3].attr.note >> 8)) & 0xFF, (u8)arg_list[v3].attr.note);
		}
		if ( (attr_mask & SPU_VOICE_VOLL) != 0 )
		{
			v10 = 0;
			v11 = arg_list[v3].attr.volume.left & ~0x8000;
			if ( (attr_mask & SPU_VOICE_VOLMODEL) != 0 )
			{
				switch ( arg_list[v3].attr.volmode.left )
				{
					case SPU_VOICE_LINEARIncN:
						v10 = 0x8000;
						break;
					case SPU_VOICE_LINEARIncR:
						v10 = 0x9000;
						break;
					case SPU_VOICE_LINEARDecN:
						v10 = 0xa000;
						break;
					case SPU_VOICE_LINEARDecR:
						v10 = 0xb000;
						break;
					case SPU_VOICE_EXPIncN:
						v10 = 0xc000;
						break;
					case SPU_VOICE_EXPIncR:
						v10 = 0xd000;
						break;
					case SPU_VOICE_EXPDec:
						v10 = 0xe000;
						break;
					default:
						break;
				}
			}
			if ( v10 )
			{
				v12 = (s16)arg_list[v3].attr.volume.left;
				if ( v12 < 128 )
				{
					if ( v12 < 0 )
						v11 = 0;
				}
				else
				{
					v11 = 127;
				}
			}
			_spu_RXX[512 * _spu_core + v7] = v11 | v10;
		}
		if ( (attr_mask & SPU_VOICE_VOLR) != 0 )
		{
			v13 = 0;
			v14 = arg_list[v3].attr.volume.right & ~0x8000;
			if ( (attr_mask & SPU_VOICE_VOLMODER) != 0 )
			{
				switch ( arg_list[v3].attr.volmode.right )
				{
					case SPU_VOICE_LINEARIncN:
						v13 = 0x8000;
						break;
					case SPU_VOICE_LINEARIncR:
						v13 = 0x9000;
						break;
					case SPU_VOICE_LINEARDecN:
						v13 = 0xa000;
						break;
					case SPU_VOICE_LINEARDecR:
						v13 = 0xb000;
						break;
					case SPU_VOICE_EXPIncN:
						v13 = 0xc000;
						break;
					case SPU_VOICE_EXPIncR:
						v13 = 0xd000;
						break;
					case SPU_VOICE_EXPDec:
						v13 = 0xe000;
						break;
					default:
						break;
				}
			}
			if ( v13 )
			{
				v15 = (s16)arg_list[v3].attr.volume.right;
				if ( v15 < 128 )
				{
					if ( v15 < 0 )
						v14 = 0;
				}
				else
				{
					v14 = 127;
				}
			}
			_spu_RXX[512 * _spu_core + 1 + v7] = v14 | v13;
		}
		if ( (attr_mask & SPU_VOICE_WDSA) != 0 )
			_spu_FsetRXX(v8 + 224, (arg_list[v3].attr.addr >> 4) << 4, 1);
		if ( (attr_mask & SPU_VOICE_LSAX) != 0 )
			_spu_FsetRXX(v8 + 226, (arg_list[v3].attr.loop_addr >> 4) << 4, 1);
		if ( (attr_mask & SPU_VOICE_ADSR_ADSR1) != 0 )
			_spu_RXX[512 * _spu_core + 3 + v7] = arg_list[v3].attr.adsr1;
		if ( (attr_mask & SPU_VOICE_ADSR_ADSR2) != 0 )
			_spu_RXX[512 * _spu_core + 4 + v7] = arg_list[v3].attr.adsr2;
		if ( (attr_mask & SPU_VOICE_ADSR_AR) != 0 )
		{
			v16 = arg_list[v3].attr.ar;
			if ( v16 >= 0x80 )
				v16 = 127;
			v17 = 0;
			if ( ((attr_mask & SPU_VOICE_ADSR_AMODE) != 0) && arg_list[v3].attr.a_mode == SPU_VOICE_EXPIncN )
				v17 = 128;
			_spu_RXX[512 * _spu_core + 3 + v7] = (u8)_spu_RXX[512 * _spu_core + 3 + v7] | (u16)(((u16)v16 | (u16)v17) << 8);
		}
		if ( (attr_mask & SPU_VOICE_ADSR_DR) != 0 )
		{
			v18 = arg_list[v3].attr.dr;
			if ( v18 >= 0x10 )
				v18 = 15;
			_spu_RXX[512 * _spu_core + 3 + v7] = (_spu_RXX[512 * _spu_core + 3 + v7] & ~0xf0) | (16 * v18);
		}
		if ( (attr_mask & SPU_VOICE_ADSR_SR) != 0 )
		{
			v19 = arg_list[v3].attr.sr;
			if ( v19 >= 0x80 )
				v19 = 127;
			v20 = 256;
			if ( (attr_mask & SPU_VOICE_ADSR_SMODE) != 0 )
			{
				switch ( arg_list[v3].attr.s_mode )
				{
					case SPU_VOICE_LINEARIncN:
						v20 = 0;
						break;
					case SPU_VOICE_EXPIncN:
						v20 = 512;
						break;
					case SPU_VOICE_EXPDec:
						v20 = 768;
						break;
					default:
						break;
				}
			}
			_spu_RXX[512 * _spu_core + 4 + v7] = (_spu_RXX[512 * _spu_core + 4 + v7] & 0x3F) | (((u16)v19 | (u16)v20) << 6);
		}
		if ( (attr_mask & SPU_VOICE_ADSR_RR) != 0 )
		{
			v22 = arg_list[v3].attr.rr;
			if ( v22 >= 0x20 )
				v22 = 31;
			v23 = 0;
			if ( (attr_mask & SPU_VOICE_ADSR_RMODE) != 0 )
			{
				if ( arg_list[v3].attr.r_mode == SPU_VOICE_EXPDec )
					v23 = 32;
			}
			_spu_RXX[512 * _spu_core + 4 + v7] = (_spu_RXX[512 * _spu_core + 4 + v7] & ~0x3f) | v22 | v23;
		}
		if ( (attr_mask & SPU_VOICE_ADSR_SL) != 0 )
		{
			v25 = arg_list[v3].attr.sl;
			if ( v25 >= 0x10 )
				v25 = 15;
			_spu_RXX[512 * _spu_core + 3 + v7] = (_spu_RXX[512 * _spu_core + 3 + v7] & ~0xF) | v25;
		}
	}
	_spu_wait_SpuLSetVoiceAttr();
}
