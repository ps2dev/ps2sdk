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

void _SsVmKeyOnNow(s16 vag_count, s16 pitch)
{
	u16 m_voice_idx;
	unsigned int left;
	libsnd2_sequence_struct_t *score_struct;
	unsigned int left_tmp2;
	unsigned int right_tmp;
	unsigned int left_tmp;
	int bits_lower;
	int bits_upper;
	libsnd2_spu_voice_t *voice_struct;

	(void)vag_count;

	m_voice_idx = _svm_cur.m_voice_idx;
	left = _svm_cur.m_voll * 0x3FFF * _svm_vh->mvol / 0x3F01 * _svm_cur.m_mvol * _svm_cur.m_vol / 0x3F01u;
	left_tmp2 = left;
	score_struct = NULL;
	if ( _svm_cur.m_seq_sep_no != 33 )
	{
		score_struct = &_ss_score[(_svm_cur.m_seq_sep_no & 0xFF)][(_svm_cur.m_seq_sep_no & 0xFF00) >> 8];
	}
	if ( score_struct != NULL )
	{
		left_tmp2 = left * (u16)score_struct->m_voll / 0x7F;
		left = left * (u16)score_struct->m_volr / 0x7F;
	}
	if ( (u8)_svm_cur.m_pan >= 0x40u )
	{
		left_tmp = left;
		right_tmp = left_tmp2 * (127 - (u8)_svm_cur.m_pan) / 0x3F;
	}
	else
	{
		right_tmp = left_tmp2;
		left_tmp = left * (u8)_svm_cur.m_pan / 0x3F;
	}
	if ( (u8)_svm_cur.m_mpan >= 0x40u )
		right_tmp = right_tmp * (127 - (u8)_svm_cur.m_mpan) / 0x3F;
	else
		left_tmp = left_tmp * (u8)_svm_cur.m_mpan / 0x3F;
	if ( (u8)_svm_cur.m_unk05 >= 0x40u )
		right_tmp = right_tmp * (127 - (u8)_svm_cur.m_unk05) / 0x3F;
	else
		left_tmp = left_tmp * (u8)_svm_cur.m_unk05 / 0x3F;
	if ( _svm_stereo_mono == 1 )
	{
		if ( right_tmp >= left_tmp )
			left_tmp = right_tmp;
		else
			right_tmp = left_tmp;
	}
	if ( score_struct != NULL )
	{
		right_tmp = right_tmp * right_tmp / 0x3FFF;
		left_tmp = left_tmp * left_tmp / 0x3FFF;
	}
	_svm_sreg_buf[m_voice_idx].m_pitch = pitch;
	_svm_sreg_buf[m_voice_idx].m_vol_left = right_tmp;
	_svm_sreg_buf[m_voice_idx].m_vol_right = left_tmp;
	_svm_sreg_dirty[_svm_cur.m_voice_idx] |= 7u;
	voice_struct = &_svm_voice[_svm_cur.m_voice_idx];
	voice_struct->m_pitch = pitch;
	if ( _svm_cur.m_voice_idx >= 16 )
	{
		bits_lower = 0;
		bits_upper = 1 << ((_svm_cur.m_voice_idx & 0xFF) - 16);
	}
	else
	{
		bits_lower = 1 << (_svm_cur.m_voice_idx & 0xFF);
		bits_upper = 0;
	}
	if ( (_svm_cur.m_mode & 4) != 0 )
	{
		_svm_orev1 |= bits_lower;
		_svm_orev2 |= bits_upper;
	}
	else
	{
		_svm_orev1 &= ~(u16)bits_lower;
		_svm_orev2 &= ~(u16)bits_upper;
	}
	_svm_onos1 &= ~(u16)bits_lower;
	_svm_okon2 |= bits_upper;
	_svm_onos2 &= ~(u16)bits_upper;
	_svm_okon1 |= bits_lower;
	_svm_okof1 &= ~_svm_okon1;
	_svm_okof2 &= ~_svm_okon2;
}
