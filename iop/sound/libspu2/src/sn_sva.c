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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuNSetVoiceAttr(void)
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

void SpuNSetVoiceAttr(int v_num, SpuVoiceAttr *arg)
{
	int v3;
	unsigned int attr_mask;
	int v5;

	v3 = 8 * v_num;
	attr_mask = arg->mask;
	if ( attr_mask == 0 )
	{
		attr_mask = 0xFFFFFFFF;
	}
	v5 = 6 * v_num;
	if ( (attr_mask & SPU_VOICE_PITCH) != 0 )
		_spu_RXX[512 * _spu_core + 2 + v3] = arg->pitch;
	if ( (attr_mask & SPU_VOICE_SAMPLE_NOTE) != 0 )
		_spu_voice_centerNote[_spu_core][v_num] = arg->sample_note;
	if ( (attr_mask & SPU_VOICE_NOTE) != 0 )
		_spu_RXX[512 * _spu_core + 2 + v3] = _spu_note2pitch(
			(_spu_voice_centerNote[_spu_core][v_num] >> 8) & 0xFF,
			(u8)_spu_voice_centerNote[_spu_core][v_num],
			(arg->note >> 8) & 0xFF,
			(u8)arg->note);
	if ( (attr_mask & SPU_VOICE_VOLL) != 0 )
	{
		int v6;
		s16 v7;

		v6 = 0;
		v7 = arg->volume.left & ~0x8000;
		if ( (attr_mask & SPU_VOICE_VOLMODEL) != 0 )
		{
			switch ( arg->volmode.left )
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
		}
		if ( v6 )
		{
			int left;

			left = arg->volume.left;
			if ( left < 128 )
			{
				if ( left < 0 )
					v7 = 0;
			}
			else
			{
				v7 = 127;
			}
		}
		_spu_RXX[512 * _spu_core + v3] = v7 | v6;
	}
	if ( (attr_mask & SPU_VOICE_VOLR) != 0 )
	{
		int v9;
		s16 v10;

		v9 = 0;
		v10 = arg->volume.right & ~0x8000;
		if ( (attr_mask & SPU_VOICE_VOLMODER) != 0 )
		{
			switch ( arg->volmode.right )
			{
				case SPU_VOICE_LINEARIncN:
					v9 = 0x8000;
					break;
				case SPU_VOICE_LINEARIncR:
					v9 = 0x9000;
					break;
				case SPU_VOICE_LINEARDecN:
					v9 = 0xa000;
					break;
				case SPU_VOICE_LINEARDecR:
					v9 = 0xb000;
					break;
				case SPU_VOICE_EXPIncN:
					v9 = 0xc000;
					break;
				case SPU_VOICE_EXPIncR:
					v9 = 0xd000;
					break;
				case SPU_VOICE_EXPDec:
					v9 = 0xe000;
					break;
				default:
					break;
			}
		}
		if ( v9 )
		{
			int right;

			right = arg->volume.right;
			if ( right < 128 )
			{
				if ( right < 0 )
					v10 = 0;
			}
			else
			{
				v10 = 127;
			}
		}
		_spu_RXX[512 * _spu_core + 1 + v3] = v10 | v9;
	}
	if ( (attr_mask & SPU_VOICE_WDSA) != 0 )
		_spu_FsetRXX(v5 + 224, (arg->addr >> 4) << 4, 1);
	if ( (attr_mask & SPU_VOICE_LSAX) != 0 )
		_spu_FsetRXX(v5 + 226, (arg->loop_addr >> 4) << 4, 1);
	if ( (attr_mask & SPU_VOICE_ADSR_ADSR1) != 0 )
		_spu_RXX[512 * _spu_core + 3 + v3] = arg->adsr1;
	if ( (attr_mask & SPU_VOICE_ADSR_ADSR2) != 0 )
		_spu_RXX[512 * _spu_core + 4 + v3] = arg->adsr2;
	if ( (attr_mask & SPU_VOICE_ADSR_AR) != 0 )
	{
		unsigned int ar;
		s16 v13;

		ar = arg->ar;
		if ( ar >= 0x80 )
			ar = 127;
		v13 = 0;
		if ( ((attr_mask & SPU_VOICE_ADSR_AMODE) != 0) && arg->a_mode == SPU_VOICE_EXPIncN )
			v13 = 128;
		_spu_RXX[512 * _spu_core + 3 + v3] = (u8)_spu_RXX[512 * _spu_core + 3 + v3] | (u16)(((u16)ar | (u16)v13) << 8);
	}
	if ( (attr_mask & SPU_VOICE_ADSR_DR) != 0 )
	{
		unsigned int dr;

		dr = arg->dr;
		if ( dr >= 0x10 )
			dr = 15;
		_spu_RXX[512 * _spu_core + 3 + v3] = (_spu_RXX[512 * _spu_core + 3 + v3] & ~0xf0) | (16 * dr);
	}
	if ( (attr_mask & SPU_VOICE_ADSR_SR) != 0 )
	{
		unsigned int sr;
		s16 v16;

		sr = arg->sr;
		if ( sr >= 0x80 )
			sr = 127;
		v16 = 256;
		if ( (attr_mask & SPU_VOICE_ADSR_SMODE) != 0 )
		{
			switch ( arg->s_mode )
			{
				case SPU_VOICE_LINEARIncN:
					v16 = 0;
					break;
				case SPU_VOICE_EXPIncN:
					v16 = 512;
					break;
				case SPU_VOICE_EXPDec:
					v16 = 768;
					break;
				default:
					break;
			}
		}
		_spu_RXX[512 * _spu_core + 4 + v3] = (_spu_RXX[512 * _spu_core + 4 + v3] & 0x3F) | (((u16)sr | (u16)v16) << 6);
	}
	if ( (attr_mask & SPU_VOICE_ADSR_RR) != 0 )
	{
		unsigned int rr;
		s16 v19;

		rr = arg->rr;
		if ( rr >= 0x20 )
			rr = 31;
		v19 = 0;
		if ( (attr_mask & SPU_VOICE_ADSR_RMODE) != 0 )
		{
			if ( arg->r_mode == SPU_VOICE_EXPDec )
				v19 = 32;
		}
		_spu_RXX[512 * _spu_core + 4 + v3] = (_spu_RXX[512 * _spu_core + 4 + v3] & ~0x3f) | rr | v19;
	}
	if ( (attr_mask & SPU_VOICE_ADSR_SL) != 0 )
	{
		unsigned int sl;

		sl = arg->sl;
		if ( sl >= 0x10 )
			sl = 15;
		_spu_RXX[512 * _spu_core + 3 + v3] = (_spu_RXX[512 * _spu_core + 3 + v3] & ~0xF) | sl;
	}
	_spu_wait_SpuNSetVoiceAttr();
}
