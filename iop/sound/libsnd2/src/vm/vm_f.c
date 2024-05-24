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

u16 _svm_okon1;
int _svm_envx_ptr;
unsigned int _svm_envx_hist[16];
libsnd2_spu_voice_t _svm_voice[24];
u16 _svm_okof1;
u16 _svm_okof2;
u16 _svm_okon2;
libsnd2_auto_vol_pan_callback _autovol;
libsnd2_auto_vol_pan_callback _autopan;
char _svm_sreg_dirty[24];
libsnd2_reg_buffer_struct_t _svm_sreg_buf[24];
libsnd2_reg_buffer_struct_2_t _svm_sreg_buf2[24];
u16 _svm_orev2;
u16 _svm_orev1;
u16 _svm_onos2;
u16 _svm_onos1;

void __attribute__((optimize("no-unroll-loops"))) wait1fsa(void)
{
	int i;
	int curdum;

	curdum = 13;
	for ( i = 0; i < 1000; i += 1 )
	{
		curdum *= 3;
		__asm__ __volatile__("" : "+g"(curdum) : :);
	}
}

void DumpSpu(void)
{
	int cur_i_1;
	int cur_i_2;

	for ( cur_i_1 = 0; (u16)cur_i_1 < 0x1A2u; cur_i_1 += 1 )
	{
		int curdata_1;

		curdata_1 = *(u16 *)(2 * (u16)cur_i_1 + (SpuGetCore() << 10) + 0xbf900000);
		printf("  0x%3x  0x%4x \n", (u16)cur_i_1, curdata_1);
	}
	for ( cur_i_2 = 944; (u16)cur_i_2 < 0x3C4u; cur_i_2 += 1 )
	{
		int curdata_2;

		curdata_2 = *(u16 *)(2 * (u16)cur_i_2 + 40 * SpuGetCore() + 0xbf900000);
		printf("  0x%3x  0x%4x \n", (u16)cur_i_2, curdata_2);
	}
	printf(" ------------------------------------\n");
}

void DumpVoice(void)
{
	u16 i;
	u16 cur_i_1;
	u16 cur_j_1;

	for ( i = 0; i < 0x10u; i += 1 )
	{
		if ( (((int)*((vu16 *)0xBF9001A0) >> i) & 1) != 0 )
		{
			printf("voice = %d\n", i);
			for ( cur_i_1 = 0; cur_i_1 < 8u; cur_i_1 += 1 )
			{
				printf("  0x%3x  0x%4x \n", cur_i_1, *(u16 *)(2 * cur_i_1 + 16 * i + 0xbf900000));
			}
			for ( cur_j_1 = 224; cur_j_1 < 0xE6u; cur_j_1 += 1 )
			{
				printf("  0x%3x  0x%4x \n", cur_j_1, *(u16 *)(2 * cur_j_1 + 12 * i + 0xbf900000));
			}
			printf(" ------------------------------------\n");
		}
	}
}

void DumpVoice2(void)
{
	u16 i;
	u16 cur_i_1;
	u16 cur_j_1;

	for ( i = 0; i < 0x10u; i += 1 )
	{
		if ( (((int)_svm_okon1 >> i) & 1) != 0 )
		{
			if ( i >= 5u )
			{
				*(u16 *)(16 * i + 0xbf900000) = 0;
				*(u16 *)(16 * i + 0xbf900002) = 0;
			}
			printf("voice = %d\n", i);
			for ( cur_i_1 = 0; cur_i_1 < 8u; cur_i_1 += 1 )
			{
				printf("  0x%3x  0x%4x \n", cur_i_1, *(u16 *)(2 * cur_i_1 + 16 * i + 0xbf900000));
			}
			for ( cur_j_1 = 224; cur_j_1 < 0xE6u; cur_j_1 += 1 )
			{
				printf("  0x%3x  0x%4x \n", cur_j_1, *(u16 *)(2 * cur_j_1 + 12 * i + 0xbf900000));
			}
			printf(" ------------------------------------\n");
		}
	}
}

void _SsVmFlush(void)
{
	int v1;
	signed int v4;
	int v12;
	int mask;
	SpuVoiceAttr voice_attr;

	_svm_envx_ptr = ((u8)_svm_envx_ptr + 1) & 0xF;
	_svm_envx_hist[_svm_envx_ptr] = 0;
	for ( v1 = 0; v1 < _SsVmMaxVoice; v1 += 1 )
	{
		libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[v1];
		SpuGetVoiceEnvelope(v1, &(voice_struct->m_key_stat));
		if ( !voice_struct->m_key_stat )
			_svm_envx_hist[_svm_envx_ptr] |= 1 << v1;
	}
	if ( !_svm_auto_kof_mode )
	{
		int voiceBits;
		int v7;

		voiceBits = -1;
		for ( v4 = 0; v4 < 15; v4 += 1 )
		{
			voiceBits &= _svm_envx_hist[v4];
		}
		for ( v7 = 0; v7 < _SsVmMaxVoice; v7 += 1 )
		{
			libsnd2_spu_voice_t *voice_struct;

			voice_struct = &_svm_voice[v7];
			if ( (voiceBits & (1 << v7)) != 0 )
			{
				if ( voice_struct->m_unk1d == 2 )
				{
					int mask_1;
					int mask2;

					mask_1 = 1 << v7;
					mask2 = 0;
					if ( v7 >= 16 )
					{
						mask_1 = 0;
						mask2 = 1 << (v7 - 16);
					}
					SpuSetNoiseVoice(SPU_OFF, ((u8)mask2 << 16) | (s16)mask_1);
				}
				voice_struct->m_unk1d = 0;
			}
		}
	}
	_svm_okon1 &= ~_svm_okof1;
	_svm_okon2 &= ~_svm_okof2;
	for ( v4 = 0; v4 < 24; v4 += 1 )
	{
		const libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[v4];
		if ( voice_struct->m_b_auto_vol )
			_autovol(v4);
		if ( voice_struct->m_b_auto_pan )
			_autopan(v4);
	}
	for ( v12 = 0; v12 < 24; v12 += 1 )
	{
		voice_attr.mask = 0;
		voice_attr.voice = 1 << v12;
		if ( (_svm_sreg_dirty[v12] & 1) != 0 )
		{
			voice_attr.mask |= SPU_VOICE_VOLL | SPU_VOICE_VOLR;
			voice_attr.volume.left = _svm_sreg_buf[v12].m_vol_left;
			voice_attr.volume.right = _svm_sreg_buf[v12].m_vol_right;
			if ( voice_attr.volume.left == 614 )
				voice_attr.volume.left = 615;
			if ( voice_attr.volume.right == 614 )
				voice_attr.volume.right = 615;
		}
		if ( (_svm_sreg_dirty[v12] & 4) != 0 )
		{
			voice_attr.mask |= SPU_VOICE_PITCH;
			voice_attr.pitch = _svm_sreg_buf[v12].m_pitch;
		}
		if ( (_svm_sreg_dirty[v12] & 8) != 0 )
		{
			voice_attr.mask |= SPU_VOICE_WDSA;
			voice_attr.addr = (_svm_sreg_buf2[v12].m_vag_spu_addr << 4) + (_svm_sreg_buf2[v12].m_vab_spu_offset << 20);
		}
		if ( (_svm_sreg_dirty[v12] & 0x10) != 0 )
		{
			voice_attr.mask |= SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2;
			voice_attr.adsr1 = _svm_sreg_buf[v12].m_adsr1;
			voice_attr.adsr2 = _svm_sreg_buf[v12].m_adsr2;
		}
		if ( voice_attr.mask )
			SpuSetVoiceAttr(&voice_attr);
		_svm_sreg_dirty[v12] = 0;
	}
	SpuSetKey(SPU_OFF, ((u8)_svm_okof2 << 16) | _svm_okof1);
	SpuSetKey(SPU_ON, ((u8)_svm_okon2 << 16) | _svm_okon1);
	mask = 0xFFFFFF >> (24 - _SsVmMaxVoice);
	// cppcheck-suppress badBitmaskCheck
	SpuSetReverbVoice(SPU_BIT, ((((u32)_svm_orev2 << 16) | _svm_orev1) & mask) | (SpuGetReverbVoice() & ~mask));
	SpuSetNoiseVoice(SPU_BIT, ((((u32)_svm_onos2 << 16) | _svm_onos1) & mask) | (SpuGetNoiseVoice() & ~mask));
	_svm_okof1 = 0;
	_svm_okof2 = 0;
	_svm_okon1 = 0;
	_svm_okon2 = 0;
	_svm_onos1 = 0;
	_svm_onos2 = 0;
}
