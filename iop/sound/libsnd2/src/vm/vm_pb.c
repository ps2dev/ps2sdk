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

int _SsVmPBVoice(s16 vc, s16 seq_sep_num, s16 vab_id, s16 prog, s16 pitch)
{
	s16 pitch_converted;
	int bend_min;
	int bend_max;
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[vc];
	if ( voice_struct->m_seq_sep_no != seq_sep_num || voice_struct->m_vab_id != vab_id || voice_struct->m_prog != prog )
	{
		return 0;
	}
	pitch_converted = pitch - 64;
	bend_min = (u16)voice_struct->m_note;
	if ( pitch_converted <= 0 )
	{
		if ( pitch_converted >= 0 )
		{
			bend_max = 0;
		}
		else
		{
			int pbmin_tmp;

			pbmin_tmp = pitch_converted * _svm_tn[(u16)(voice_struct->m_tone + 16 * _svm_cur.m_fake_program)].pbmin;
			bend_min = bend_min + pbmin_tmp / 64 - 1;
			bend_max = 2 * (pbmin_tmp % 64) + 127;
		}
	}
	else
	{
		int pbmax_tmp;

		pbmax_tmp = pitch_converted * _svm_tn[(u16)(voice_struct->m_tone + 16 * _svm_cur.m_fake_program)].pbmax;
		bend_min += pbmax_tmp / 63;
		bend_max = 2 * (pbmax_tmp % 63);
	}
	_svm_cur.m_voice_idx = vc;
	_svm_cur.m_tone = voice_struct->m_tone;
	_svm_sreg_buf[vc].m_pitch = note2pitch2(bend_min, bend_max);
	_svm_sreg_dirty[vc] |= 4;
	return 1;
}

int _SsVmPitchBend(s16 seq_sep_no, int vab_id, int prog, s16 pitch)
{
	int voice_i_tmp;
	int voice_offs_tmp;

	_SsVmVSetUp(vab_id, prog);
	_svm_cur.m_seq_sep_no = seq_sep_no;
	voice_offs_tmp = 0;
	for ( voice_i_tmp = 0; (s16)voice_i_tmp < _SsVmMaxVoice; voice_i_tmp += 1 )
	{
		voice_offs_tmp += (s16)_SsVmPBVoice(voice_i_tmp, seq_sep_no, vab_id, prog, pitch);
	}
	return voice_offs_tmp;
}
