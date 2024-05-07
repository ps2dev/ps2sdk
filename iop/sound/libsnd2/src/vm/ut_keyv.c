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

s16 SsUtKeyOnV(s16 vc, s16 vab_id, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr)
{
	const ProgAtr *pProg;
	const VagAtr *pVag;
	libsnd2_spu_voice_t *voice_struct;

	if ( _snd_ev_flag == 1 )
		return -1;
	_snd_ev_flag = 1;
	if ( (u16)vc >= 0x18u || _SsVmVSetUp(vab_id, prog) )
	{
		_snd_ev_flag = 0;
		return -1;
	}
	_svm_cur.m_seq_sep_no = 33;
	_svm_cur.m_note = note;
	_svm_cur.m_fine = fine;
	_svm_cur.m_tone = tone;
	if ( voll == volr )
	{
		_svm_cur.m_unk05 = 64;
		_svm_cur.m_voll = voll;
	}
	else
	{
		int volr_lshift_6;

		volr_lshift_6 = volr << 6;
		if ( volr >= voll )
		{
			int voll_lshift_6;

			voll_lshift_6 = voll << 6;
			if ( !volr )
				__builtin_trap();
			if ( volr == -1 && (unsigned int)voll_lshift_6 == 0x80000000 )
				__builtin_trap();
			_svm_cur.m_voll = volr;
			_svm_cur.m_unk05 = 127 - voll_lshift_6 / volr;
		}
		else
		{
			if ( !voll )
				__builtin_trap();
			if ( voll == -1 && (unsigned int)volr_lshift_6 == 0x80000000 )
				__builtin_trap();
			_svm_cur.m_voll = voll;
			_svm_cur.m_unk05 = volr_lshift_6 / voll;
		}
	}
	pProg = &_svm_pg[prog];
	_svm_cur.m_mvol = pProg->mvol;
	_svm_cur.m_mpan = pProg->mpan;
	_svm_cur.m_sep_sep_no_tonecount = pProg->tones;
	pVag = &_svm_tn[16 * _svm_cur.m_fake_program + tone];
	_svm_cur.m_prior = pVag->prior;
	_svm_cur.m_vag_idx2 = pVag->vag;
	_svm_cur.m_vol = pVag->vol;
	_svm_cur.m_pan = pVag->pan;
	_svm_cur.m_centre = pVag->center;
	_svm_cur.m_shift = pVag->shift;
	_svm_cur.m_mode = pVag->mode;
	if ( _svm_cur.m_vag_idx2 == 0 )
	{
		_snd_ev_flag = 0;
		return -1;
	}
	_svm_cur.m_voice_idx = vc;
	voice_struct = &_svm_voice[vc];
	voice_struct->m_seq_sep_no = 33;
	voice_struct->m_vab_id = vab_id;
	voice_struct->m_prog = prog;
	voice_struct->m_fake_program = _svm_cur.m_fake_program;
	voice_struct->m_vag_idx = _svm_cur.m_vag_idx2;
	voice_struct->m_note = note;
	voice_struct->m_unk1d = 1;
	voice_struct->m_unk02 = 0;
	voice_struct->m_tone = _svm_cur.m_tone;
	voice_struct->m_voll2 = _svm_cur.m_voll;
	_SsVmDoAllocate();
	if ( _svm_cur.m_vag_idx2 == 255 )
	{
		vmNoiseOn(vc);
	}
	else
	{
		_SsVmKeyOnNow(1, note2pitch2(note, fine));
	}
	_snd_ev_flag = 0;
	return vc;
}

s16 SsUtKeyOffV(s16 vc)
{
	if ( _snd_ev_flag == 1 )
		return -1;
	_snd_ev_flag = 1;
	if ( (u16)vc >= 0x18u )
	{
		_snd_ev_flag = 0;
		return -1;
	}
	_svm_cur.m_voice_idx = vc;
	_SsVmKeyOffNow();
	return 0;
}
