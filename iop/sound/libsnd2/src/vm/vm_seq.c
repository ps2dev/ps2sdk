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

void _SsVmSetSeqVol(s16 seq_sep_num, s16 voll, s16 volr)
{
	libsnd2_sequence_struct_t *score_struct;
	int v5;
	int m_vab_id;
	const VagAtr *vag_attr_ptr;
	unsigned int vol_factor;
	unsigned int voll_val;
	unsigned int pan;
	unsigned int volr_val;
	unsigned int mpan;
	unsigned int m_pan;

	score_struct = &_ss_score[(u8)seq_sep_num][(seq_sep_num & 0xFF00) >> 8];
	score_struct->m_voll = voll;
	score_struct->m_volr = volr;
	if ( (u16)voll >= 0x7Fu )
		score_struct->m_voll = 127;
	if ( (u16)score_struct->m_volr >= 0x7Fu )
		score_struct->m_volr = 127;
	for ( v5 = 0; (s16)v5 < _SsVmMaxVoice; v5 += 1 )
	{
		if ( (_snd_vmask & (1 << v5)) == 0 )
		{
			libsnd2_spu_voice_t *voice_struct;

			voice_struct = &_svm_voice[(s16)v5];
			if ( voice_struct->m_seq_sep_no == seq_sep_num )
			{
				m_vab_id = voice_struct->m_vab_id;
				if ( m_vab_id == (char)score_struct->m_vab_id )
				{
					_SsVmVSetUp(m_vab_id, voice_struct->m_fake_program);
					vag_attr_ptr = &_svm_tn[16 * voice_struct->m_fake_program + voice_struct->m_tone];
					vol_factor = _svm_vh->mvol * 0x3FFF
										 * (voice_struct->m_voll1 * score_struct->m_vol[voice_struct->m_channel_idx] / 127) / 0x3F01
										 * _svm_pg[voice_struct->m_prog].mvol * vag_attr_ptr->vol;
					voll_val = vol_factor / 0x3F01 * (u16)score_struct->m_voll / 0x7F;
					pan = vag_attr_ptr->pan;
					volr_val = vol_factor / 0x3F01 * (u16)score_struct->m_volr / 0x7F;
					if ( pan >= 0x40 )
						voll_val = voll_val * (127 - pan) / 0x3F;
					else
						volr_val = volr_val * pan / 0x3F;
					mpan = _svm_pg[voice_struct->m_prog].mpan;
					if ( mpan >= 0x40 )
						voll_val = (int)((u16)voll_val * (127 - mpan)) / 63;
					else
						volr_val = (unsigned int)((u64)(2181570691LL * (int)((u16)volr_val * mpan)) >> 32) >> 5;
					m_pan = (u8)voice_struct->m_pan;
					if ( m_pan >= 0x40 )
						voll_val = (int)((u16)voll_val * (127 - m_pan)) / 63;
					else
						volr_val = (unsigned int)((u64)(2181570691LL * (int)((u16)volr_val * m_pan)) >> 32) >> 5;
					if ( _svm_stereo_mono == 1 )
					{
						if ( (u16)voll_val >= (unsigned int)(u16)volr_val )
							volr_val = voll_val & 0xFFFF;
						else
							voll_val = volr_val & 0xFFFF;
					}
					_svm_sreg_dirty[(s16)v5] |= 3u;
					_svm_sreg_buf[(s16)v5].m_vol_left = (u16)voll_val * (u16)voll_val / 0x3FFF;
					_svm_sreg_buf[(s16)v5].m_vol_right = (s16)(volr_val * volr_val) / 0x3FFF;
				}
			}
		}
	}
}

void _SsVmGetSeqVol(s16 seq_sep_no, s16 *voll_ptr, s16 *volr_ptr)
{
	const libsnd2_sequence_struct_t *score_struct;

	_svm_cur.m_seq_sep_no = seq_sep_no;
	score_struct = &_ss_score[(u8)seq_sep_no][(seq_sep_no & 0xFF00) >> 8];
	*voll_ptr = score_struct->m_voll;
	*volr_ptr = score_struct->m_volr;
}

int _SsVmGetSeqLVol(s16 seq_sep_no)
{
	const libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[(u8)seq_sep_no][(seq_sep_no & 0xFF00) >> 8];
	_svm_cur.m_seq_sep_no = seq_sep_no;
	return score_struct->m_voll;
}

int _SsVmGetSeqRVol(s16 seq_sep_no)
{
	const libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[(u8)seq_sep_no][(seq_sep_no & 0xFF00) >> 8];
	_svm_cur.m_seq_sep_no = seq_sep_no;
	return score_struct->m_volr;
}

void _SsVmSeqKeyOff(s16 seq_sep_no)
{
	u8 cur_voice_tmp;

	for ( cur_voice_tmp = 0; cur_voice_tmp < _SsVmMaxVoice; cur_voice_tmp += 1 )
	{
		const libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[cur_voice_tmp];
		if ( (_snd_vmask & (1 << cur_voice_tmp)) == 0 && voice_struct->m_seq_sep_no == seq_sep_no )
		{
			_svm_cur.m_voice_idx = cur_voice_tmp;
			_SsVmKeyOffNow();
		}
	}
}
