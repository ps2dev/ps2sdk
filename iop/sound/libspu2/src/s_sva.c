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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceAttr(void)
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

void SpuSetVoiceAttr(SpuVoiceAttr *arg)
{
	int voice_num;
	unsigned int attr_mask;
	int regtmp;
	int vol_left_upper;
	s16 vol_left_clamped;
	int vol_left;
	int vol_right_upper;
	s16 vol_right_clamped;
	int vol_right;
	unsigned int attr_ar;
	s16 adsr_ar_part;
	unsigned int adsr_dr_part;
	unsigned int adsr_sr_part;
	s16 converted_s_mode;
	unsigned int adsr_rr_part;
	s16 attr_r_mode_converted;
	unsigned int attr_sl;

	attr_mask = arg->mask;
	if ( attr_mask == 0 )
	{
		attr_mask = 0xFFFFFFFF;
	}
	for ( voice_num = 0; voice_num < 24; voice_num += 1 )
	{
		if ( (arg->voice & (1 << voice_num)) != 0 )
		{
			int converted_voice_num;

			converted_voice_num = 8 * voice_num;
			regtmp = 2 * (voice_num * 2 + voice_num);
			if ( (attr_mask & SPU_VOICE_PITCH) != 0 )
				_spu_RXX[512 * _spu_core + 2 + converted_voice_num] = arg->pitch;
			if ( (attr_mask & SPU_VOICE_SAMPLE_NOTE) != 0 )
				_spu_voice_centerNote[_spu_core][voice_num] = arg->sample_note;
			if ( (attr_mask & SPU_VOICE_NOTE) != 0 )
				_spu_RXX[512 * _spu_core + 2 + converted_voice_num] = _spu_note2pitch(
					(_spu_voice_centerNote[_spu_core][voice_num] >> 8) & 0xFF,
					(u8)_spu_voice_centerNote[_spu_core][voice_num],
					(arg->note >> 8) & 0xFF,
					(u8)arg->note);
			if ( (attr_mask & SPU_VOICE_VOLL) != 0 )
			{
				vol_left_upper = 0;
				vol_left_clamped = arg->volume.left & ~0x8000;
				if ( (attr_mask & SPU_VOICE_VOLMODEL) != 0 )
				{
					switch ( arg->volmode.left )
					{
						case SPU_VOICE_LINEARIncN:
							vol_left_upper = 0x8000;
							break;
						case SPU_VOICE_LINEARIncR:
							vol_left_upper = 0x9000;
							break;
						case SPU_VOICE_LINEARDecN:
							vol_left_upper = 0xa000;
							break;
						case SPU_VOICE_LINEARDecR:
							vol_left_upper = 0xb000;
							break;
						case SPU_VOICE_EXPIncN:
							vol_left_upper = 0xc000;
							break;
						case SPU_VOICE_EXPIncR:
							vol_left_upper = 0xd000;
							break;
						case SPU_VOICE_EXPDec:
							vol_left_upper = 0xe000;
							break;
						default:
							break;
					}
				}
				if ( vol_left_upper )
				{
					vol_left = arg->volume.left;
					if ( vol_left < 128 )
					{
						if ( vol_left < 0 )
							vol_left_clamped = 0;
					}
					else
					{
						vol_left_clamped = 127;
					}
				}
				_spu_RXX[512 * _spu_core + converted_voice_num] = vol_left_clamped | vol_left_upper;
			}
			if ( (attr_mask & SPU_VOICE_VOLR) != 0 )
			{
				vol_right_upper = 0;
				vol_right_clamped = arg->volume.right & ~0x8000;
				if ( (attr_mask & SPU_VOICE_VOLMODER) != 0 )
				{
					switch ( arg->volmode.right )
					{
						case SPU_VOICE_LINEARIncN:
							vol_right_upper = 0x8000;
							break;
						case SPU_VOICE_LINEARIncR:
							vol_right_upper = 0x9000;
							break;
						case SPU_VOICE_LINEARDecN:
							vol_right_upper = 0xa000;
							break;
						case SPU_VOICE_LINEARDecR:
							vol_right_upper = 0xb000;
							break;
						case SPU_VOICE_EXPIncN:
							vol_right_upper = 0xc000;
							break;
						case SPU_VOICE_EXPIncR:
							vol_right_upper = 0xd000;
							break;
						case SPU_VOICE_EXPDec:
							vol_right_upper = 0xe000;
							break;
						default:
							break;
					}
				}
				if ( vol_right_upper )
				{
					vol_right = arg->volume.right;
					if ( vol_right < 128 )
					{
						if ( vol_right < 0 )
							vol_right_clamped = 0;
					}
					else
					{
						vol_right_clamped = 127;
					}
				}
				_spu_RXX[512 * _spu_core + 1 + converted_voice_num] = vol_right_clamped | vol_right_upper;
			}
			if ( (attr_mask & SPU_VOICE_WDSA) != 0 )
				_spu_FsetRXX(regtmp + 224, (arg->addr >> 4) << 4, 1);
			if ( (attr_mask & SPU_VOICE_LSAX) != 0 )
				_spu_FsetRXX(regtmp + 226, (arg->loop_addr >> 4) << 4, 1);
			if ( (attr_mask & SPU_VOICE_ADSR_ADSR1) != 0 )
				_spu_RXX[512 * _spu_core + 3 + converted_voice_num] = arg->adsr1;
			if ( (attr_mask & SPU_VOICE_ADSR_ADSR2) != 0 )
				_spu_RXX[512 * _spu_core + 4 + converted_voice_num] = arg->adsr2;
			if ( (attr_mask & SPU_VOICE_ADSR_AR) != 0 )
			{
				attr_ar = arg->ar;
				if ( attr_ar >= 0x80 )
					attr_ar = 127;
				adsr_ar_part = 0;
				if ( ((attr_mask & SPU_VOICE_ADSR_AMODE) != 0) && arg->a_mode == SPU_VOICE_EXPIncN )
					adsr_ar_part = 128;
				_spu_RXX[512 * _spu_core + 3 + converted_voice_num] =
					(u8)_spu_RXX[512 * _spu_core + 3 + converted_voice_num] | (u16)(((u16)attr_ar | (u16)adsr_ar_part) << 8);
			}
			if ( (attr_mask & SPU_VOICE_ADSR_DR) != 0 )
			{
				adsr_dr_part = arg->dr;
				if ( adsr_dr_part >= 0x10 )
					adsr_dr_part = 15;
				_spu_RXX[512 * _spu_core + 3 + converted_voice_num] =
					(_spu_RXX[512 * _spu_core + 3 + converted_voice_num] & ~0xf0) | (16 * adsr_dr_part);
			}
			if ( (attr_mask & SPU_VOICE_ADSR_SR) != 0 )
			{
				adsr_sr_part = arg->sr;
				if ( adsr_sr_part >= 0x80 )
					adsr_sr_part = 127;
				converted_s_mode = 256;
				if ( (attr_mask & SPU_VOICE_ADSR_SMODE) != 0 )
				{
					switch ( arg->s_mode )
					{
						case SPU_VOICE_LINEARIncN:
							converted_s_mode = 0;
							break;
						case SPU_VOICE_EXPIncN:
							converted_s_mode = 512;
							break;
						case SPU_VOICE_EXPDec:
							converted_s_mode = 768;
							break;
						default:
							break;
					}
				}
				_spu_RXX[512 * _spu_core + 4 + converted_voice_num] =
					(_spu_RXX[512 * _spu_core + 4 + converted_voice_num] & 0x3F)
					| (((u16)adsr_sr_part | (u16)converted_s_mode) << 6);
			}
			if ( (attr_mask & SPU_VOICE_ADSR_RR) != 0 )
			{
				adsr_rr_part = arg->rr;
				if ( adsr_rr_part >= 0x20 )
					adsr_rr_part = 31;
				attr_r_mode_converted = 0;
				if ( (attr_mask & SPU_VOICE_ADSR_RMODE) != 0 )
				{
					if ( arg->r_mode == SPU_VOICE_EXPDec )
						attr_r_mode_converted = 32;
				}
				_spu_RXX[512 * _spu_core + 4 + converted_voice_num] =
					(_spu_RXX[512 * _spu_core + 4 + converted_voice_num] & ~0x3f) | adsr_rr_part | attr_r_mode_converted;
			}
			if ( (attr_mask & SPU_VOICE_ADSR_SL) != 0 )
			{
				attr_sl = arg->sl;
				if ( attr_sl >= 0x10 )
					attr_sl = 15;
				_spu_RXX[512 * _spu_core + 3 + converted_voice_num] =
					(_spu_RXX[512 * _spu_core + 3 + converted_voice_num] & ~0xF) | attr_sl;
			}
		}
	}
	_spu_wait_SpuSetVoiceAttr();
}
