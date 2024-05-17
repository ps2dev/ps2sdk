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

s16 _SsVmAlloc(void)
{
	u8 voice_to_alloc_idx;
	u16 lowest_key_stat;
	char match_counter;
	int lowest_unk02;
	char lowest_match;
	u8 i_cur_1;
	u16 lowest_prior;
	int m_priority;
	unsigned int m_key_stat;
	int m_unk02;

	voice_to_alloc_idx = 99;
	lowest_key_stat = -1;
	match_counter = 0;
	lowest_unk02 = 0;
	lowest_match = 99;
	lowest_prior = _svm_cur.m_prior;
	for ( i_cur_1 = 0; i_cur_1 < _SsVmMaxVoice; i_cur_1 += 1 )
	{
		libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[i_cur_1];
		if ( (_snd_vmask & (1 << i_cur_1)) == 0 )
		{
			if ( !voice_struct->m_unk1d )
			{
				if ( !voice_struct->m_key_stat )
				{
					voice_to_alloc_idx = i_cur_1;
					break;
				}
			}
			m_priority = voice_struct->m_priority;
			if ( m_priority >= lowest_prior )
			{
				if ( m_priority == lowest_prior )
				{
					m_key_stat = (u16)voice_struct->m_key_stat;
					match_counter += 1;
					if ( m_key_stat >= lowest_key_stat )
					{
						if ( m_key_stat == lowest_key_stat )
						{
							m_unk02 = voice_struct->m_unk02;
							if ( lowest_unk02 < m_unk02 )
							{
								lowest_unk02 = (u16)voice_struct->m_unk02;
								lowest_match = i_cur_1;
							}
						}
					}
					else
					{
						lowest_unk02 = (u16)voice_struct->m_unk02;
						lowest_key_stat = voice_struct->m_key_stat;
						lowest_match = i_cur_1;
					}
				}
			}
			else
			{
				lowest_prior = voice_struct->m_priority;
				lowest_match = i_cur_1;
				lowest_key_stat = voice_struct->m_key_stat;
				lowest_unk02 = (u16)voice_struct->m_unk02;
				match_counter = 1;
			}
		}
	}
	if ( voice_to_alloc_idx == 99 )
	{
		voice_to_alloc_idx = lowest_match;
		if ( !match_counter )
			voice_to_alloc_idx = _SsVmMaxVoice;
	}
	if ( voice_to_alloc_idx < _SsVmMaxVoice )
	{
		int v16;
		libsnd2_spu_voice_t *voice_struct;

		for ( v16 = 0; (u8)v16 < _SsVmMaxVoice; v16 += 1 )
		{
			libsnd2_spu_voice_t *voice_struct_1;

			voice_struct_1 = &_svm_voice[(u8)v16];
			if ( (_snd_vmask & (1 << (u8)v16)) == 0 )
				voice_struct_1->m_unk02 += 1;
		}
		voice_struct = &_svm_voice[voice_to_alloc_idx];
		voice_struct->m_unk02 = 0;
		voice_struct->m_b_auto_pan = 0;
		voice_struct->m_b_auto_vol = 0;
		voice_struct->m_priority = _svm_cur.m_prior;
	}
	return voice_to_alloc_idx;
}
