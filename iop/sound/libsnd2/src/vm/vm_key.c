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

int _SsVmKeyOn(int seq_sep_no, s16 vab_id, s16 prog, s16 note, s16 voll, s16 unknown27)
{
	int on_keys_mask_1;
	libsnd2_sequence_struct_t *score_struct;
	int on_keys_mask_2;
	const ProgAtr *prog_attr_Ptr;
	u8 selected_vag_count;
	const VagAtr *vag_attr_ptr;
	int v16;
	u8 vag_nums[128];
	u8 vag_index_nums[128];

	score_struct = NULL;
	if ( seq_sep_no != 33 )
	{
		score_struct = &_ss_score[(u8)seq_sep_no][(seq_sep_no & 0xFF00) >> 8];
	}
	on_keys_mask_1 = 0;
	on_keys_mask_2 = -1;
	if ( _SsVmVSetUp(vab_id, prog) != 0 )
	{
		return on_keys_mask_2;
	}
	_svm_cur.m_seq_sep_no = seq_sep_no;
	_svm_cur.m_note = note;
	_svm_cur.m_fine = 0;
	if ( score_struct != NULL )
		_svm_cur.m_voll = (u16)voll * score_struct->m_vol[score_struct->m_channel_idx] / 127;
	else
		_svm_cur.m_voll = voll;
	_svm_cur.m_unk05 = unknown27;
	prog_attr_Ptr = &_svm_pg[prog];
	_svm_cur.m_mvol = prog_attr_Ptr->mvol;
	_svm_cur.m_mpan = prog_attr_Ptr->mpan;
	_svm_cur.m_sep_sep_no_tonecount = prog_attr_Ptr->tones;
	if ( _svm_cur.m_fake_program >= (int)_svm_vh->ps )
	{
		return -1;
	}
	if ( !voll )
		return _SsVmKeyOff(seq_sep_no, vab_id, prog, note);
	selected_vag_count = _SsVmSelectToneAndVag(vag_index_nums, vag_nums);
	for ( v16 = 0; (u8)v16 < (unsigned int)selected_vag_count; v16 += 1 )
	{
		_svm_cur.m_vag_idx2 = (u8)vag_nums[(u8)v16];
		_svm_cur.m_tone = vag_index_nums[(u8)v16];
		vag_attr_ptr = &_svm_tn[(u16)(_svm_cur.m_tone + 16 * _svm_cur.m_fake_program)];
		_svm_cur.m_prior = vag_attr_ptr->prior;
		_svm_cur.m_vol = vag_attr_ptr->vol;
		_svm_cur.m_pan = vag_attr_ptr->pan;
		_svm_cur.m_centre = vag_attr_ptr->center;
		_svm_cur.m_shift = vag_attr_ptr->shift;
		_svm_cur.m_mode = vag_attr_ptr->mode;
		_svm_cur.m_voice_idx = (u8)_SsVmAlloc();
		if ( _svm_cur.m_voice_idx >= _SsVmMaxVoice )
		{
			on_keys_mask_1 = -1;
		}
		else
		{
			libsnd2_spu_voice_t *voice_struct;

			voice_struct = &_svm_voice[_svm_cur.m_voice_idx];
			voice_struct->m_unk1d = 1;
			voice_struct->m_unk02 = 0;
			voice_struct->m_seq_sep_no = seq_sep_no;
			voice_struct->m_vab_id = _svm_cur.m_vab_id;
			voice_struct->m_fake_program = _svm_cur.m_fake_program;
			voice_struct->m_prog = prog;
			if ( score_struct != NULL )
			{
				voice_struct->m_voll1 = voll;
				voice_struct->m_channel_idx = score_struct->m_channel_idx;
			}
			voice_struct->m_pan = unknown27;
			voice_struct->m_voll2 = _svm_cur.m_voll;
			voice_struct->m_tone = _svm_cur.m_tone;
			voice_struct->m_note = note;
			voice_struct->m_priority = _svm_cur.m_prior;
			voice_struct->m_vag_idx = _svm_cur.m_vag_idx2;
			_SsVmDoAllocate();
			if ( _svm_cur.m_vag_idx2 == 255 )
			{
				vmNoiseOn(_svm_cur.m_voice_idx);
			}
			else
			{
				_SsVmKeyOnNow(selected_vag_count, note2pitch());
			}
			on_keys_mask_1 |= 1 << (_svm_cur.m_voice_idx & 0xFF);
		}
	}
	return on_keys_mask_1;
}

int _SsVmKeyOff(int seq_sep_no, s16 vab_id, s16 prog, s16 note)
{
	int v4;
	int v5;

	v5 = 0;
	for ( v4 = 0; (u8)v4 < _SsVmMaxVoice; v4 += 1 )
	{
		const libsnd2_spu_voice_t *voice_struct;

		voice_struct = &_svm_voice[v4];
		if ( !((_snd_vmask & (1 << v4)) != 0 || voice_struct->m_note != note || voice_struct->m_prog != prog
					 || voice_struct->m_seq_sep_no != (s16)seq_sep_no || voice_struct->m_vab_id != vab_id) )
		{
			v5 += 1;
			if ( voice_struct->m_vag_idx == 255 )
			{
				vmNoiseOff(v4);
			}
			else
			{
				_svm_cur.m_voice_idx = (u8)v4;
				_SsVmKeyOffNow();
			}
		}
	}
	return v5;
}

int _SsVmSeKeyOn(s16 vab_id, s16 prog, u16 note, int pitch, u16 voll, u16 volr)
{
	s16 unknown27;
	u16 voll_;

	(void)pitch;

	if ( voll == volr )
	{
		unknown27 = 64;
		voll_ = voll;
	}
	else if ( volr >= (unsigned int)voll )
	{
		voll_ = volr;
		unknown27 = 127 - (voll << 6) / volr;
	}
	else
	{
		voll_ = voll;
		unknown27 = (volr << 6) / voll;
	}
	return _SsVmKeyOn(33, vab_id, prog, note, voll_, unknown27);
}

int _SsVmSeKeyOff(s16 vab_id, s16 prog, s16 note)
{
	return _SsVmKeyOff(33, vab_id, prog, note);
}

void KeyOnCheck(void)
{
	;
}
