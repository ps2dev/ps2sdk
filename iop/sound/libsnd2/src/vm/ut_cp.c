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

s16 SsUtChangePitch(s16 vc, s16 vab_id, s16 prog, s16 old_note, s16 old_fine, s16 new_note, s16 new_fine)
{
	int m_vab_id;
	int m_prog;
	const libsnd2_spu_voice_t *voice_struct;

	(void)old_fine;

	if ( (u16)vc >= 0x18u )
		return -1;
	voice_struct = &_svm_voice[vc];
	m_vab_id = voice_struct->m_vab_id;
	if ( m_vab_id != vab_id )
	{
		return -1;
	}
	m_prog = voice_struct->m_prog;
	if ( m_prog != prog )
	{
		return -1;
	}
	if ( voice_struct->m_note != old_note )
	{
		return -1;
	}
	_SsVmVSetUp(m_vab_id, m_prog);
	_svm_cur.m_seq_sep_no = 33;
	_svm_cur.m_voice_idx = vc;
	_svm_cur.m_tone = voice_struct->m_tone;
	_svm_sreg_buf[vc].m_pitch = note2pitch2(new_note, new_fine);
	_svm_sreg_dirty[vc] |= 4u;
	return 0;
}
