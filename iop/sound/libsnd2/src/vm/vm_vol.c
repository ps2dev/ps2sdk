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

int _SsVmSetVol(s16 seq_sep_no, s16 vab_id, s16 prog, s16 voll, s16 volr)
{
	int v7;
	u8 volr_tmp;
	libsnd2_sequence_struct_t *score_struct;
	int v13;
	int v20;
	const VagAtr *vag_attr_ptr;
	unsigned int volcalc;
	unsigned int left_vol;
	unsigned int pan;
	unsigned int right_vol;
	unsigned int mpan;
	unsigned int vol_selfmul;

	v7 = 0;
	volr_tmp = volr;
	score_struct = &_ss_score[(u8)seq_sep_no][(seq_sep_no & 0xFF00) >> 8];
	_SsVmVSetUp(vab_id, prog);
	_svm_cur.m_seq_sep_no = seq_sep_no;
	if ( !volr )
		volr_tmp = 1;
	if ( !voll )
		voll = 1;
	for ( v13 = 0; (s16)v13 < _SsVmMaxVoice; v13 += 1 )
	{
		if ( (_snd_vmask & (1 << v13)) == 0 )
		{
			const libsnd2_spu_voice_t *voice_struct;

			voice_struct = &_svm_voice[(s16)v13];
			if (
				voice_struct->m_seq_sep_no == seq_sep_no && voice_struct->m_prog == prog && voice_struct->m_vab_id == vab_id )
			{
				v20 = score_struct->m_vol[score_struct->m_channel_idx];
				if ( (v20 != 0) || (v20 != (u16)voll) )
				{
					vag_attr_ptr = &_svm_tn[16 * voice_struct->m_fake_program + voice_struct->m_tone];
					volcalc = voice_struct->m_voll1 * (u16)voll / 127 * 0x3FFF * _svm_vh->mvol / 0x3F01 * _svm_pg[prog].mvol
									* vag_attr_ptr->vol;
					left_vol = volcalc / 0x3F01 * (u16)score_struct->m_voll / 0x7F;
					pan = vag_attr_ptr->pan;
					right_vol = volcalc / 0x3F01 * (u16)score_struct->m_volr / 0x7F;
					if ( pan >= 0x40 )
						left_vol = left_vol * (127 - pan) / 0x3F;
					else
						right_vol = right_vol * pan / 0x3F;
					mpan = _svm_pg[voice_struct->m_prog].mpan;
					if ( mpan >= 0x40 )
						left_vol = left_vol * (127 - mpan) / 0x3F;
					else
						right_vol = right_vol * _svm_pg[voice_struct->m_prog].mpan / 0x3F;
					if ( volr_tmp >= 0x40u )
						left_vol = left_vol * (127 - volr_tmp) / 0x3F;
					else
						right_vol = right_vol * volr_tmp / 0x3F;
					vol_selfmul = left_vol * left_vol;
					if ( _svm_stereo_mono == 1 )
					{
						if ( left_vol >= right_vol )
							right_vol = left_vol;
						else
							left_vol = right_vol;
						vol_selfmul = left_vol * left_vol;
					}
					v7 += 1;
					_svm_sreg_dirty[(s16)v13] |= 3u;
					_svm_sreg_buf[(s16)v13].m_vol_left = vol_selfmul / 0x3FFF;
					_svm_sreg_buf[(s16)v13].m_vol_right = right_vol * right_vol / 0x3FFF;
				}
				if ( v20 != 0 )
				{
					score_struct->m_vol[score_struct->m_channel_idx] = 1;
				}
			}
		}
	}
	return v7;
}
