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

void SpuSetCommonAttr(SpuCommonAttr *attr)
{
	s16 left_1;
	unsigned int mask;
	s16 right_1;
	int mov_left_part1;
	int mov_right_part1;
	vu16 *regstmp1;
	u16 attrtmp1;
	vu16 *regstmp2;
	u16 attrtmp2;
	vu16 *regstmp3;
	u16 attrtmp3;

	left_1 = 0;
	mask = attr->mask;
	right_1 = 0;
	mov_left_part1 = 0;
	mov_right_part1 = 0;
	if ( mask == 0 )
	{
		mask = 0xFFFFFFFF;
	}
	if ( (mask & SPU_COMMON_MVOLL) != 0 )
	{
		if ( (mask & SPU_COMMON_MVOLMODEL) != 0 )
		{
			switch ( attr->mvolmode.left )
			{
				case SPU_VOICE_LINEARIncN:
					mov_left_part1 = 0x8000;
					break;
				case SPU_VOICE_LINEARIncR:
					mov_left_part1 = 0x9000;
					break;
				case SPU_VOICE_LINEARDecN:
					mov_left_part1 = 0xA000;
					break;
				case SPU_VOICE_LINEARDecR:
					mov_left_part1 = 0xB000;
					break;
				case SPU_VOICE_EXPIncN:
					mov_left_part1 = 0xC000;
					break;
				case SPU_VOICE_EXPIncR:
					mov_left_part1 = 0xD000;
					break;
				case SPU_VOICE_EXPDec:
					mov_left_part1 = 0xE000;
					break;
				default:
					left_1 = attr->mvol.left;
					break;
			}
		}
		if ( mov_left_part1 )
		{
			int left_2;

			left_2 = attr->mvol.left;
			left_1 = 127;
			if ( left_2 < 128 )
			{
				left_1 = 0;
				if ( left_2 >= 0 )
					left_1 = attr->mvol.left;
			}
		}
		_spu_RXX[20 * _spu_core + 944] = (left_1 & ~0x8000) | mov_left_part1;
	}
	if ( (mask & SPU_COMMON_MVOLR) != 0 )
	{
		s16 right_masked;

		if ( (mask & SPU_COMMON_MVOLMODER) != 0 )
		{
			switch ( attr->mvolmode.right )
			{
				case SPU_VOICE_LINEARIncN:
					mov_right_part1 = 0x8000;
					break;
				case SPU_VOICE_LINEARIncR:
					mov_right_part1 = 0x9000;
					break;
				case SPU_VOICE_LINEARDecN:
					mov_right_part1 = 0xa000;
					break;
				case SPU_VOICE_LINEARDecR:
					mov_right_part1 = 0xb000;
					break;
				case SPU_VOICE_EXPIncN:
					mov_right_part1 = 0xc000;
					break;
				case SPU_VOICE_EXPIncR:
					mov_right_part1 = 0xd000;
					break;
				case SPU_VOICE_EXPDec:
					mov_right_part1 = 0xe000;
					break;
				default:
					right_1 = attr->mvol.right;
					break;
			}
		}
		right_masked = right_1 & ~0x8000;
		if ( mov_right_part1 )
		{
			int right_2;
			s16 right_3;

			right_2 = attr->mvol.right;
			right_3 = 127;
			if ( right_2 < 128 )
			{
				right_3 = 0;
				if ( right_2 >= 0 )
					right_3 = attr->mvol.right;
			}
			right_masked = right_3 & ~0x8000;
		}
		_spu_RXX[20 * _spu_core + 945] = right_masked | mov_right_part1;
	}
	if ( (mask & SPU_COMMON_CDVOLL) != 0 )
		_spu_RXX[20 * _spu_core + 968] = attr->cd.volume.left;
	if ( (mask & SPU_COMMON_CDVOLR) != 0 )
		_spu_RXX[20 * _spu_core + 969] = attr->cd.volume.right;
	if ( (mask & SPU_COMMON_EXTVOLL) != 0 )
		_spu_RXX[20 * _spu_core + 970] = attr->ext.volume.left;
	if ( (mask & SPU_COMMON_EXTVOLR) != 0 )
		_spu_RXX[20 * _spu_core + 971] = attr->ext.volume.right;
	if ( (mask & SPU_COMMON_CDREV) != 0 )
	{
		regstmp1 = &_spu_RXX[512 * _spu_core];
		if ( attr->cd.reverb == SPU_ON )
			attrtmp1 = regstmp1[205] | 4;
		else
			attrtmp1 = regstmp1[205] & ~4;
		regstmp1[205] = attrtmp1;
	}
	if ( (mask & SPU_COMMON_CDMIX) != 0 )
	{
		regstmp2 = &_spu_RXX[512 * _spu_core];
		if ( attr->cd.mix == SPU_ON )
			attrtmp2 = regstmp2[205] | 1;
		else
			attrtmp2 = regstmp2[205] & ~1;
		regstmp2[205] = attrtmp2;
	}
	if ( (mask & SPU_COMMON_EXTREV) != 0 )
	{
		regstmp3 = &_spu_RXX[512 * _spu_core];
		if ( attr->ext.reverb == SPU_ON )
			attrtmp3 = regstmp3[205] | 8;
		else
			attrtmp3 = regstmp3[205] & ~8;
		regstmp3[205] = attrtmp3;
	}
	if ( (mask & SPU_COMMON_EXTMIX) != 0 )
	{
		if ( attr->ext.mix == SPU_ON )
			_spu_RXX[512 * _spu_core + 205] |= 2u;
		else
			_spu_RXX[512 * _spu_core + 205] &= ~2u;
	}
}
