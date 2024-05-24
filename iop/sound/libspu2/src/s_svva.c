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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceVolumeAttr(void)
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

void SpuSetVoiceVolumeAttr(int v_num, s16 voll, s16 volr, s16 voll_mode, s16 volr_mode)
{
	s16 v5;
	s16 v6;
	int v7;
	s16 v8;
	s16 v9;

	v5 = voll & ~0x8000;
	v6 = 0;
	v7 = 8 * v_num;
	switch ( voll_mode )
	{
		case SPU_VOICE_LINEARIncN:
			v6 = 0x8000;
			break;
		case SPU_VOICE_LINEARIncR:
			v6 = 0x9000;
			break;
		case SPU_VOICE_LINEARDecN:
			v6 = 0xA000;
			break;
		case SPU_VOICE_LINEARDecR:
			v6 = 0xB000;
			break;
		case SPU_VOICE_EXPIncN:
			v6 = 0xC000;
			break;
		case SPU_VOICE_EXPIncR:
			v6 = 0xD000;
			break;
		case SPU_VOICE_EXPDec:
			v6 = 0xE000;
			break;
		default:
			break;
	}
	v8 = volr & ~0x8000;
	v9 = 0;
	_spu_RXX[512 * _spu_core + v7] = v5 | v6;
	switch ( volr_mode )
	{
		case SPU_VOICE_LINEARIncN:
			v9 = 0x8000;
			break;
		case SPU_VOICE_LINEARIncR:
			v9 = 0x9000;
			break;
		case SPU_VOICE_LINEARDecN:
			v9 = 0xA000;
			break;
		case SPU_VOICE_LINEARDecR:
			v9 = 0xB000;
			break;
		case SPU_VOICE_EXPIncN:
			v9 = 0xC000;
			break;
		case SPU_VOICE_EXPIncR:
			v9 = 0xD000;
			break;
		case SPU_VOICE_EXPDec:
			v9 = 0xE000;
			break;
		default:
			break;
	}
	_spu_RXX[512 * _spu_core + 1 + v7] = v8 | v9;
	_spu_wait_SpuSetVoiceVolumeAttr();
}
