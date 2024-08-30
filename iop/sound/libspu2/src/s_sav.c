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

unsigned int _SpuSetAnyVoice(int on_off_flags, unsigned int voice_bits, int word_idx1, int word_idx2)
{
	// Unofficial: Fixed _spu_RQ and _spu_RQmask access offsets
	int p_register_1;
	int p_register_2;
	unsigned int ret_bits;

	if ( (_spu_env & 1) != 0 )
	{
		p_register_1 = _spu_RQ[word_idx1 - 188];
		p_register_2 = (u8)_spu_RQ[word_idx2 - 188];
	}
	else
	{
		p_register_1 = _spu_RXX[512 * _spu_core + word_idx1];
		p_register_2 = (u8)_spu_RXX[512 * _spu_core + word_idx2];
	}
	ret_bits = p_register_1 | (p_register_2 << 16);
	switch ( on_off_flags )
	{
		case SPU_OFF:
			if ( (_spu_env & 1) != 0 )
			{
				_spu_RQ[word_idx1 - 188] &= ~(u16)voice_bits;
				_spu_RQ[word_idx2 - 188] &= ~((voice_bits >> 16) & 0xFF);
				_spu_RQmask |= 1 << ((word_idx1 - 190) >> 1);
				if ( (1 << ((word_idx1 - 190) >> 1)) == 16 )
				{
					_spu_RQmask |= 8;
				}
			}
			else
			{
				_spu_RXX[512 * _spu_core + word_idx1] &= ~(u16)voice_bits;
				_spu_RXX[512 * _spu_core + word_idx2] &= ~((voice_bits >> 16 & 0xFF));
			}
			ret_bits &= ~(voice_bits & 0xFFFFFF);
			break;
		case SPU_ON:
			if ( (_spu_env & 1) != 0 )
			{
				_spu_RQ[word_idx1 - 188] |= voice_bits;
				_spu_RQ[word_idx2 - 188] |= (voice_bits >> 16) & 0xFF;
				_spu_RQmask |= 1 << ((word_idx1 - 190) >> 1);
				if ( (1 << ((word_idx1 - 190) >> 1)) == 16 )
				{
					_spu_RQmask |= 8;
				}
			}
			else
			{
				_spu_RXX[512 * _spu_core + word_idx1] |= voice_bits;
				_spu_RXX[512 * _spu_core + word_idx2] |= (voice_bits >> 16) & 0xFF;
			}
			ret_bits |= voice_bits & 0xFFFFFF;
			break;
		case SPU_BIT:
			if ( (_spu_env & 1) != 0 )
			{
				_spu_RQ[word_idx1 - 188] = voice_bits;
				_spu_RQ[word_idx2 - 188] = (voice_bits >> 16) & 0xFF;
				_spu_RQmask |= 1 << ((word_idx1 - 190) >> 1);
				if ( (1 << ((word_idx1 - 190) >> 1)) == 16 )
				{
					_spu_RQmask |= 8;
				}
			}
			else
			{
				_spu_RXX[512 * _spu_core + word_idx1] = voice_bits;
				_spu_RXX[512 * _spu_core + word_idx2] = (voice_bits >> 16) & 0xFF;
			}
			ret_bits = voice_bits & 0xFFFFFF;
			break;
		default:
			break;
	}
	return ret_bits & 0xFFFFFF;
}
