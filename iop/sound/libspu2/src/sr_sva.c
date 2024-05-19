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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuRSetVoiceAttr(void)
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

int SpuRSetVoiceAttr(int min_, int max_, SpuVoiceAttr *arg)
{
	unsigned int attr_mask;
	int v5;
	int v8;
	int i;
	int v12;
	int v13;
	s16 v14;
	int left;
	int v16;
	s16 v17;
	int right;
	unsigned int ar;
	s16 v20;
	unsigned int dr;
	unsigned int sr;
	s16 v23;
	unsigned int rr;
	s16 v26;
	unsigned int sl;

	attr_mask = arg->mask;
	if ( attr_mask == 0 )
	{
		attr_mask = 0xFFFFFFFF;
	}
	v5 = max_;
	if ( min_ < 0 )
		min_ = 0;
	if ( min_ >= 24 )
		return SPU_INVALID_ARGS;
	if ( max_ >= 24 )
		v5 = 23;
	if ( v5 < 0 )
	{
		return SPU_INVALID_ARGS;
	}
	v8 = v5 + 1;
	if ( v5 < min_ )
		return SPU_INVALID_ARGS;
	for ( i = min_; i < v8; i += 1 )
	{
		if ( (arg->voice & (1 << i)) != 0 )
		{
			int v11;

			v11 = 8 * i;
			v12 = 2 * (i * 2 + i);
			if ( (attr_mask & SPU_VOICE_PITCH) != 0 )
				_spu_RXX[512 * _spu_core + 2 + v11] = arg->pitch;
			if ( (attr_mask & SPU_VOICE_SAMPLE_NOTE) != 0 )
				_spu_voice_centerNote[_spu_core][i] = arg->sample_note;
			if ( (attr_mask & SPU_VOICE_NOTE) != 0 )
				_spu_RXX[512 * _spu_core + 2 + v11] = _spu_note2pitch(
					(_spu_voice_centerNote[_spu_core][i] >> 8) & 0xFF,
					(u8)_spu_voice_centerNote[_spu_core][i],
					(arg->note >> 8) & 0xFF,
					(u8)arg->note);
			if ( (attr_mask & SPU_VOICE_VOLL) != 0 )
			{
				v13 = 0;
				v14 = arg->volume.left & ~0x8000;
				if ( (attr_mask & SPU_VOICE_VOLMODEL) != 0 )
				{
					switch ( arg->volmode.left )
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
					left = arg->volume.left;
					if ( left < 128 )
					{
						if ( left < 0 )
							v14 = 0;
					}
					else
					{
						v14 = 127;
					}
				}
				_spu_RXX[512 * _spu_core + v11] = v14 | v13;
			}
			if ( (attr_mask & SPU_VOICE_VOLR) != 0 )
			{
				v16 = 0;
				v17 = arg->volume.right & ~0x8000;
				if ( (attr_mask & SPU_VOICE_VOLMODER) != 0 )
				{
					switch ( arg->volmode.right )
					{
						case SPU_VOICE_LINEARIncN:
							v16 = 0x8000;
							break;
						case SPU_VOICE_LINEARIncR:
							v16 = 0x9000;
							break;
						case SPU_VOICE_LINEARDecN:
							v16 = 0xa000;
							break;
						case SPU_VOICE_LINEARDecR:
							v16 = 0xb000;
							break;
						case SPU_VOICE_EXPIncN:
							v16 = 0xc000;
							break;
						case SPU_VOICE_EXPIncR:
							v16 = 0xd000;
							break;
						case SPU_VOICE_EXPDec:
							v16 = 0xe000;
							break;
						default:
							break;
					}
				}
				if ( v16 )
				{
					right = arg->volume.right;
					if ( right < 128 )
					{
						if ( right < 0 )
							v17 = 0;
					}
					else
					{
						v17 = 127;
					}
				}
				_spu_RXX[512 * _spu_core + 1 + v11] = v17 | v16;
			}
			if ( (attr_mask & SPU_VOICE_WDSA) != 0 )
				_spu_FsetRXX(v12 + 224, (arg->addr >> 4) << 4, 1);
			if ( (attr_mask & SPU_VOICE_LSAX) != 0 )
				_spu_FsetRXX(v12 + 226, (arg->loop_addr >> 4) << 4, 1);
			if ( (attr_mask & SPU_VOICE_ADSR_ADSR1) != 0 )
				_spu_RXX[512 * _spu_core + 3 + v11] = arg->adsr1;
			if ( (attr_mask & SPU_VOICE_ADSR_ADSR2) != 0 )
				_spu_RXX[512 * _spu_core + 4 + v11] = arg->adsr2;
			if ( (attr_mask & SPU_VOICE_ADSR_AR) != 0 )
			{
				ar = arg->ar;
				if ( ar >= 0x80 )
					ar = 127;
				v20 = 0;
				if ( ((attr_mask & SPU_VOICE_ADSR_AMODE) != 0) && arg->a_mode == SPU_VOICE_EXPIncN )
					v20 = 128;
				_spu_RXX[512 * _spu_core + 3 + v11] =
					(u8)_spu_RXX[512 * _spu_core + 3 + v11] | (u16)(((u16)ar | (u16)v20) << 8);
			}
			if ( (attr_mask & SPU_VOICE_ADSR_DR) != 0 )
			{
				dr = arg->dr;
				if ( dr >= 0x10 )
					dr = 15;
				_spu_RXX[512 * _spu_core + 3 + v11] = (_spu_RXX[512 * _spu_core + 3 + v11] & ~0xf0) | (16 * dr);
			}
			if ( (attr_mask & SPU_VOICE_ADSR_SR) != 0 )
			{
				sr = arg->sr;
				if ( sr >= 0x80 )
					sr = 127;
				v23 = 256;
				if ( (attr_mask & SPU_VOICE_ADSR_SMODE) != 0 )
				{
					switch ( arg->s_mode )
					{
						case SPU_VOICE_LINEARIncN:
							v23 = 0;
							break;
						case SPU_VOICE_EXPIncN:
							v23 = 512;
							break;
						case SPU_VOICE_EXPDec:
							v23 = 768;
							break;
						default:
							break;
					}
				}
				_spu_RXX[512 * _spu_core + 4 + v11] =
					(_spu_RXX[512 * _spu_core + 4 + v11] & 0x3F) | (((u16)sr | (u16)v23) << 6);
			}
			if ( (attr_mask & SPU_VOICE_ADSR_RR) != 0 )
			{
				rr = arg->rr;
				if ( rr >= 0x20 )
					rr = 31;
				v26 = 0;
				if ( (attr_mask & SPU_VOICE_ADSR_RMODE) != 0 )
				{
					if ( arg->r_mode == SPU_VOICE_EXPDec )
						v26 = 32;
				}
				_spu_RXX[512 * _spu_core + 4 + v11] = (_spu_RXX[512 * _spu_core + 4 + v11] & ~0x3f) | rr | v26;
			}
			if ( (attr_mask & SPU_VOICE_ADSR_SL) != 0 )
			{
				sl = arg->sl;
				if ( sl >= 0x10 )
					sl = 15;
				_spu_RXX[512 * _spu_core + 3 + v11] = (_spu_RXX[512 * _spu_core + 3 + v11] & ~0xF) | sl;
			}
		}
	}
	_spu_wait_SpuRSetVoiceAttr();
	return SPU_SUCCESS;
}
