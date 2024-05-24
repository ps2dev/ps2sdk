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

void SpuSetKey(int on_off, unsigned int voice_bit)
{
	unsigned int voice_bit_tmp;

	voice_bit_tmp = voice_bit & 0xFFFFFF;
	switch ( on_off )
	{
		case SPU_OFF:
			if ( (_spu_env & 1) != 0 )
			{
				_spu_RQ[2] = voice_bit_tmp & 0xFFFF;
				_spu_RQ[3] = (voice_bit_tmp >> 16) & 0xFFFF;
				_spu_RQmask |= 1u;
				_spu_RQvoice &= ~voice_bit_tmp;
				if ( (_spu_RQ[0] & (u16)voice_bit_tmp) != 0 )
					_spu_RQ[0] &= ~(u16)voice_bit_tmp;
				if ( (_spu_RQ[1] & ((voice_bit_tmp >> 16) & 0xFFFF)) != 0 )
					_spu_RQ[1] &= ~((voice_bit_tmp >> 16) & 0xFFFF);
			}
			else
			{
				_spu_RXX[512 * _spu_core + 210] = voice_bit_tmp & 0xFFFF;
				_spu_RXX[512 * _spu_core + 211] = (voice_bit_tmp >> 16) & 0xFFFF;
				_spu_keystat[_spu_core] &= ~voice_bit_tmp;
			}
			break;
		case SPU_ON:
			if ( (_spu_env & 1) != 0 )
			{
				_spu_RQ[0] = voice_bit_tmp & 0xFFFF;
				_spu_RQ[1] = (voice_bit_tmp >> 16) & 0xFFFF;
				_spu_RQmask |= 1u;
				_spu_RQvoice |= voice_bit_tmp;
				if ( (_spu_RQ[2] & (u16)voice_bit_tmp) != 0 )
					_spu_RQ[2] &= ~(u16)voice_bit_tmp;
				if ( (_spu_RQ[3] & ((voice_bit_tmp >> 16) & 0xFFFF)) != 0 )
					_spu_RQ[3] &= ~((voice_bit_tmp >> 16) & 0xFFFF);
			}
			else
			{
				_spu_RXX[512 * _spu_core + 208] = voice_bit_tmp & 0xFFFF;
				_spu_RXX[512 * _spu_core + 209] = (voice_bit_tmp >> 16) & 0xFFFF;
				_spu_keystat[_spu_core] |= voice_bit_tmp;
			}
			break;
		default:
			break;
	}
}
