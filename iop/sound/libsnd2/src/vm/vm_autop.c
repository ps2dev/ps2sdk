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

void SeAutoPan(s16 vc, s16 start_pan, s16 end_pan, s16 delta_time)
{
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[vc];
	if ( start_pan == end_pan )
		return;
	voice_struct->m_b_auto_pan = 1;
	voice_struct->m_auto_pan_start = start_pan;
	voice_struct->m_auto_pan_end = end_pan;
	if (
		((start_pan - end_pan >= 0) && (start_pan - end_pan < delta_time))
		|| ((start_pan - end_pan < 0) && (end_pan - start_pan < delta_time)) )
	{
		int v8;

		if ( end_pan >= start_pan )
		{
			int v12;
			int v13;

			v12 = delta_time;
			v13 = start_pan - end_pan;
			if ( v13 == -1 && (unsigned int)v12 == 0x80000000 )
				__builtin_trap();
			voice_struct->m_auto_pan_amount = 1;
			v8 = -(v12 / v13);
		}
		else
		{
			int v9;

			v8 = delta_time;
			v9 = start_pan - end_pan;
			if ( v9 == -1 && (unsigned int)v8 == 0x80000000 )
				__builtin_trap();
			v8 = v8 / v9;
			voice_struct->m_auto_pan_amount = -1;
		}
		voice_struct->m_auto_pan_dt1 = v8;
		voice_struct->m_auto_pan_dt2 = v8;
	}
	else
	{
		int v14;

		v14 = start_pan - end_pan;
		if ( !delta_time )
			__builtin_trap();
		if ( delta_time == -1 && (unsigned int)v14 == 0x80000000 )
			__builtin_trap();
		voice_struct->m_auto_pan_dt1 = 0;
		voice_struct->m_auto_pan_dt2 = 0;
		voice_struct->m_auto_pan_amount = -(s16)(v14 / delta_time);
	}
}

void SetAutoPan(int vc)
{
	int m_auto_pan_dt2;
	int v5;
	int m_auto_pan_amount;
	char m_auto_pan_start;
	const VagAtr *pVag;
	int pan;
	unsigned int v15;
	unsigned int v16;
	unsigned int v18;
	unsigned int v19;
	int mpan;
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[(s16)vc];
	m_auto_pan_dt2 = (u16)voice_struct->m_auto_pan_dt2;
	voice_struct->m_auto_pan_dt2 = m_auto_pan_dt2 - 1;
	if ( m_auto_pan_dt2 > 0 )
		return;
	v5 = (u16)voice_struct->m_auto_pan_start + (u16)voice_struct->m_auto_pan_amount;
	voice_struct->m_auto_pan_start = v5;
	m_auto_pan_amount = voice_struct->m_auto_pan_amount;
	if ( m_auto_pan_amount <= 0 )
	{
		if ( m_auto_pan_amount < 0 && (s16)v5 <= voice_struct->m_auto_pan_end )
		{
			voice_struct->m_auto_pan_start = voice_struct->m_auto_pan_end;
			voice_struct->m_b_auto_pan = 0;
		}
	}
	else if ( (s16)v5 >= voice_struct->m_auto_pan_end )
	{
		voice_struct->m_auto_pan_start = voice_struct->m_auto_pan_end;
		voice_struct->m_b_auto_pan = 0;
	}
	m_auto_pan_start = voice_struct->m_auto_pan_start;
	if ( _svm_cur.m_voice_idx == (s16)vc )
		_svm_cur.m_unk05 = voice_struct->m_auto_pan_start;
	pVag = &_svm_tn[(u16)voice_struct->m_tone + (voice_struct->m_fake_program * 16)];
	pan = (char)pVag->pan;
	v15 = voice_struct->m_voll2 * 0x3FFF * _svm_vh->mvol / 0x3F01 * _svm_pg[voice_struct->m_prog].mvol * pVag->vol;
	v16 = v15 / 0x3F01;
	if ( pan >= 64 )
	{
		v19 = v15 / 0x3F01;
		v18 = (v16 * (127 - pan)) >> 6;
	}
	else
	{
		unsigned int v17;

		v17 = v16 * pan;
		v18 = v15 / 0x3F01;
		v19 = v17 >> 6;
	}
	mpan = (char)_svm_pg[voice_struct->m_prog].mpan;
	if ( mpan >= 64 )
	{
		int v22;

		v22 = (u16)v18 * (127 - mpan);
		v18 = (unsigned int)v22 >> 6;
		if ( v22 < 0 )
			v18 = (unsigned int)(v22 + 63) >> 6;
	}
	else
	{
		int v21;

		v21 = (u16)v19 * mpan;
		v19 = (unsigned int)v21 >> 6;
		if ( v21 < 0 )
			v19 = (unsigned int)(v21 + 63) >> 6;
	}
	if ( m_auto_pan_start >= 64 )
	{
		int v24;

		v24 = (u16)v18 * (127 - m_auto_pan_start);
		v18 = (unsigned int)v24 >> 6;
		if ( v24 < 0 )
			v18 = (unsigned int)(v24 + 63) >> 6;
	}
	else
	{
		int v23;

		v23 = (u16)v19 * m_auto_pan_start;
		v19 = (unsigned int)v23 >> 6;
		if ( v23 < 0 )
			v19 = (unsigned int)(v23 + 63) >> 6;
	}
	if ( _svm_stereo_mono == 1 )
	{
		if ( (u16)v18 >= (unsigned int)(u16)v19 )
			v19 = v18;
		else
			v18 = v19;
	}
	voice_struct->m_auto_pan_dt2 = voice_struct->m_auto_pan_dt1;
	_svm_sreg_buf[vc].m_vol_left = v18;
	_svm_sreg_buf[vc].m_vol_right = v19;
	_svm_sreg_dirty[(s16)vc] |= 3;
}
