/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

void _SsVmNoiseOnWithAdsr(s32 voll, s32 volr, s32 arg2, s32 arg3)
{
	int voice_idx_tmp;

	_svm_cur.m_prior = 127;
	voice_idx_tmp = (u8)_SsVmAlloc();
	_svm_cur.m_voice_idx = voice_idx_tmp;
	if ( voice_idx_tmp < _SsVmMaxVoice )
		vmNoiseOn2(_svm_cur.m_voice_idx, voll, volr, arg2, arg3);
}

void _SsVmNoiseOff(void)
{
	int v0;

	for ( v0 = 0; (s16)v0 < _SsVmMaxVoice; v0 += 1 )
	{
		if ( (_snd_vmask & (1 << v0)) == 0 )
		{
			const libsnd2_spu_voice_t *voice_struct;

			voice_struct = &_svm_voice[v0];
			if ( voice_struct->m_unk1d == 2 )
			{
				vmNoiseOff(v0);
			}
		}
	}
}

void _SsVmNoiseOn(u16 voll, u16 volr)
{
	_SsVmNoiseOnWithAdsr(voll, volr, 0x80ff, 0x5fc8);
}
