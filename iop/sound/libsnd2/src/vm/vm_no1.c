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

void vmNoiseOn(u8 vc)
{
	unsigned int right_vol_calc;
	libsnd2_sequence_struct_t *score_struct;
	unsigned int left_vol_calc;
	unsigned int left_vol_final;
	unsigned int right_vol_final;
	int v8;
	int v9;
	int v11;
	libsnd2_spu_voice_t *voice_struct;

	right_vol_calc = _svm_cur.m_voll * 0x3FFF * _svm_vh->mvol / 0x3F01 * _svm_cur.m_mvol * _svm_cur.m_vol / 0x3F01u;
	left_vol_calc = right_vol_calc;
	score_struct = NULL;
	if ( _svm_cur.m_seq_sep_no != 33 )
	{
		score_struct = &_ss_score[(_svm_cur.m_seq_sep_no & 0xFF)][(_svm_cur.m_seq_sep_no & 0xFF00) >> 8];
	}
	if ( score_struct != NULL )
	{
		left_vol_calc = right_vol_calc * (u16)score_struct->m_voll / 0x7F;
		right_vol_calc = right_vol_calc * (u16)score_struct->m_volr / 0x7F;
	}
	if ( (unsigned int)_svm_cur.m_pan >= 0x40 )
	{
		right_vol_final = right_vol_calc;
		left_vol_final = left_vol_calc * (127 - _svm_cur.m_pan) / 0x3F;
	}
	else
	{
		left_vol_final = left_vol_calc;
		right_vol_final = right_vol_calc * _svm_cur.m_pan / 0x3F;
	}
	if ( (unsigned int)_svm_cur.m_mpan >= 0x40 )
		left_vol_final = left_vol_final * (127 - _svm_cur.m_mpan) / 0x3F;
	else
		right_vol_final = right_vol_final * _svm_cur.m_mpan / 0x3F;
	if ( (unsigned int)_svm_cur.m_unk05 >= 0x40 )
		left_vol_final = left_vol_final * (127 - _svm_cur.m_unk05) / 0x3F;
	else
		right_vol_final = _svm_cur.m_unk05 * right_vol_final / 0x3F;
	if ( _svm_stereo_mono == 1 )
	{
		if ( left_vol_final >= right_vol_final )
			right_vol_final = left_vol_final;
		else
			left_vol_final = right_vol_final;
	}
	if ( score_struct != NULL )
	{
		left_vol_final = left_vol_final * left_vol_final / 0x3FFF;
		right_vol_final = right_vol_final * right_vol_final / 0x3FFF;
	}
	SpuSetNoiseClock((_svm_cur.m_note - _svm_cur.m_centre) & 0x3F);
	_svm_sreg_buf[vc].m_vol_left = left_vol_final;
	_svm_sreg_buf[vc].m_vol_right = right_vol_final;
	_svm_sreg_dirty[vc] |= 3;
	if ( vc >= 0x10u )
	{
		v8 = 0;
		v9 = 1 << (vc - 16);
	}
	else
	{
		v8 = 1 << vc;
		v9 = 0;
	}
	voice_struct = &_svm_voice[vc];
	voice_struct->m_pitch = 10;
	for ( v11 = 0; (s16)v11 < _SsVmMaxVoice; v11 += 1 )
	{
		if ( (_snd_vmask & (1 << v11)) == 0 )
		{
			libsnd2_spu_voice_t *voice_struct_1;

			voice_struct_1 = &_svm_voice[v11];
			voice_struct_1->m_unk1d &= 1u;
		}
	}
	voice_struct->m_unk1d = 2;
	_svm_okon1 |= v8;
	_svm_okon2 |= v9;
	_svm_okof1 &= ~_svm_okon1;
	_svm_okof2 &= ~_svm_okon2;
	if ( (_svm_cur.m_mode & 4) != 0 )
	{
		_svm_orev1 |= v8;
		_svm_orev2 |= v9;
	}
	else
	{
		_svm_orev1 &= ~(u16)v8;
		_svm_orev2 &= ~(u16)v9;
	}
	_svm_onos1 = v8;
	_svm_onos2 = v9;
}
