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

void SePitchBend(u8 vc, s16 arg1)
{
	char v8;
	int tone;
	int note;
	int pbend;
	const libsnd2_spu_voice_t *voice_struct;

	if ( vc >= 0x18 )
	{
		return;
	}
	voice_struct = &_svm_voice[vc];
	_svm_cur.m_fake_program = voice_struct->m_fake_program;
	v8 = voice_struct->m_tone;
	_svm_cur.m_voice_idx = vc;
	_svm_cur.m_tone = v8;
	tone = v8 + ((u8)_svm_cur.m_fake_program * 16);
	if ( arg1 < 0 )
	{
		int v13;

		v13 = arg1 * _svm_tn[tone].pbmin;
		pbend = v13 / 127 + 127;
		note = voice_struct->m_note + v13 / 127 - 1;
	}
	else
	{
		int v10;

		v10 = arg1 * _svm_tn[tone].pbmax;
		note = voice_struct->m_note + v10 / 127;
		pbend = v10 % 127;
	}
	_svm_sreg_buf[vc].m_pitch = note2pitch2(note, pbend);
	_svm_sreg_dirty[vc] |= 4;
}
