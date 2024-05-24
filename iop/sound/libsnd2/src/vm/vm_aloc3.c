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

char SsBlockVoiceAllocation(void)
{
	if ( _snd_ev_flag == 1 )
		return -1;
	_snd_ev_flag = 1;
	return 1;
}

char SsUnBlockVoiceAllocation(void)
{
	if ( !_snd_ev_flag )
		return -1;
	_snd_ev_flag = 0;
	return 1;
}

int SsAllocateVoices(u8 voices, u8 priority)
{
	int v2;
	int v3;
	u16 v4;
	u16 v5;
	char v9;
	int m_priority;
	unsigned int m_key_stat;
	int v16;
	int v17;

	v3 = 0;
	for ( v2 = 0; (u8)v2 < (unsigned int)voices; v2 += 1 )
	{
		int m_unk02;
		char v7;
		char v8;

		v4 = -1;
		v5 = priority;
		m_unk02 = 0;
		v7 = 99;
		v8 = 99;
		for ( v9 = 0; (u8)v9 < _SsVmMaxVoice; v9 += 1 )
		{
			if ( ((1 << (u8)v9) & v3) == 0 )
			{
				libsnd2_spu_voice_t *voice_struct;

				voice_struct = &_svm_voice[(u8)v9];
				if ( !voice_struct->m_unk1d && !voice_struct->m_key_stat )
				{
					v8 = v9;
					break;
				}
				m_priority = voice_struct->m_priority;
				if ( m_priority >= v5 )
				{
					if ( m_priority == v5 )
					{
						m_key_stat = (u16)voice_struct->m_key_stat;
						if ( m_key_stat >= v4 )
						{
							if ( m_key_stat != v4 || m_unk02 >= voice_struct->m_unk02 )
								continue;
							m_unk02 = (u16)voice_struct->m_unk02;
						}
						else
						{
							m_unk02 = (u16)voice_struct->m_unk02;
							v4 = voice_struct->m_key_stat;
						}
						v7 = v9;
					}
				}
				else
				{
					v5 = voice_struct->m_priority;
					v4 = voice_struct->m_key_stat;
					m_unk02 = (u16)voice_struct->m_unk02;
					v7 = v9;
				}
			}
		}
		if ( v7 != 99 )
		{
			v3 |= 1 << v7;
		}
		else if ( v8 != 99 )
		{
			v3 |= 1 << v8;
		}
		else
		{
			return -1;
		}
	}
	v16 = voices;
	for ( v17 = 0; (u8)v17 < _SsVmMaxVoice; v17 += 1 )
	{
		libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[(u8)v17];
		if ( ((1 << v17) & v3) != 0 )
		{
			v16 -= 1;
			voice_struct->m_unk02 = (u8)v16;
			voice_struct->m_priority = priority;
			voice_struct->m_b_auto_vol = 0;
			voice_struct->m_b_auto_pan = 0;
			if ( voice_struct->m_unk1d == 2 )
				SpuSetNoiseVoice(SPU_OFF, 1 << v17);
			voice_struct->m_unk1d = 1;
		}
		else
		{
			voice_struct->m_unk02 += voices;
		}
	}
	return v3;
}
