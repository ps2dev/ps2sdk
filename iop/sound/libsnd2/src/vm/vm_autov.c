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

void SeAutoVol(s16 vc, s16 start_vol, s16 end_vol, s16 delta_time)
{
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[vc];
	if ( start_vol == end_vol )
		return;
	voice_struct->m_b_auto_vol = 1;
	voice_struct->m_auto_vol_start = start_vol;
	voice_struct->m_auto_vol_end = end_vol;
	if (
		((start_vol - end_vol >= 0) && (start_vol - end_vol < delta_time))
		|| ((start_vol - end_vol < 0) && (end_vol - start_vol < delta_time)) )
	{
		int v8;

		if ( end_vol >= start_vol )
		{
			int v12;
			int v13;

			v12 = delta_time;
			v13 = start_vol - end_vol;
			if ( v13 == -1 && (unsigned int)v12 == 0x80000000 )
				__builtin_trap();
			voice_struct->m_auto_vol_amount = 1;
			v8 = -(v12 / v13);
		}
		else
		{
			int v9;

			v8 = delta_time;
			v9 = start_vol - end_vol;
			if ( v9 == -1 && (unsigned int)v8 == 0x80000000 )
				__builtin_trap();
			voice_struct->m_auto_vol_amount = -1;
			v8 = v8 / v9;
		}
		voice_struct->m_auto_vol_dt1 = v8;
		voice_struct->m_auto_vol_dt2 = v8;
	}
	else
	{
		int v14;

		v14 = start_vol - end_vol;
		if ( !delta_time )
			__builtin_trap();
		if ( delta_time == -1 && (unsigned int)v14 == 0x80000000 )
			__builtin_trap();
		voice_struct->m_auto_vol_dt1 = 0;
		voice_struct->m_auto_vol_dt2 = 0;
		voice_struct->m_auto_vol_amount = -(s16)(v14 / delta_time);
	}
}

void SetAutoVol(int vc)
{
	int m_auto_vol_dt2;
	s16 v5;
	int m_auto_vol_amount;
	s16 m_auto_vol_start;
	unsigned int v9;
	unsigned int v10;
	unsigned int v11;
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[(s16)vc];
	m_auto_vol_dt2 = (u16)voice_struct->m_auto_vol_dt2;
	voice_struct->m_auto_vol_dt2 = m_auto_vol_dt2 - 1;
	if ( m_auto_vol_dt2 > 0 )
		return;
	v5 = voice_struct->m_auto_vol_start + voice_struct->m_auto_vol_amount;
	voice_struct->m_auto_vol_start = v5;
	m_auto_vol_amount = voice_struct->m_auto_vol_amount;
	if ( m_auto_vol_amount > 0 )
	{
		if ( voice_struct->m_auto_vol_end <= v5 )
		{
			voice_struct->m_auto_vol_start = voice_struct->m_auto_vol_end;
			voice_struct->m_b_auto_vol = 0;
		}
	}
	if ( m_auto_vol_amount < 0 )
	{
		if ( voice_struct->m_auto_vol_end >= v5 )
		{
			voice_struct->m_auto_vol_start = voice_struct->m_auto_vol_end;
			voice_struct->m_b_auto_vol = 0;
		}
	}
	m_auto_vol_start = voice_struct->m_auto_vol_start;
	_svm_cur.m_voll = m_auto_vol_start;
	v9 = m_auto_vol_start * 0x3FFF * _svm_vh->mvol / 0x3F01 * _svm_cur.m_mvol * _svm_cur.m_vol / 0x3F01u;
	if ( (u8)_svm_cur.m_pan >= 0x40u )
	{
		v11 = v9;
		v10 = (v9 * (127 - (u8)_svm_cur.m_pan)) >> 6;
	}
	else
	{
		v10 = v9;
		v11 = (v9 * (u8)_svm_cur.m_pan) >> 6;
	}
	if ( (u8)_svm_cur.m_mpan >= 0x40u )
	{
		int v13;

		v13 = (u16)v10 * (127 - (u8)_svm_cur.m_mpan);
		v10 = (unsigned int)v13 >> 6;
		if ( v13 < 0 )
			v10 = (unsigned int)(v13 + 63) >> 6;
	}
	else
	{
		int v12;

		v12 = (u16)v11 * (u8)_svm_cur.m_mpan;
		v11 = (unsigned int)v12 >> 6;
		if ( v12 < 0 )
			v11 = (unsigned int)(v12 + 63) >> 6;
	}
	if ( (u8)_svm_cur.m_unk05 >= 0x40u )
	{
		int v15;

		v15 = (u16)v10 * (127 - (u8)_svm_cur.m_unk05);
		v10 = (unsigned int)v15 >> 6;
		if ( v15 < 0 )
			v10 = (unsigned int)(v15 + 63) >> 6;
	}
	else
	{
		int v14;

		v14 = (u16)v11 * (u8)_svm_cur.m_unk05;
		v11 = (unsigned int)v14 >> 6;
		if ( v14 < 0 )
			v11 = (unsigned int)(v14 + 63) >> 6;
	}
	if ( _svm_stereo_mono == 1 )
	{
		if ( (u16)v10 >= (unsigned int)(u16)v11 )
			v11 = v10;
		else
			v10 = v11;
	}
	voice_struct->m_auto_vol_dt2 = voice_struct->m_auto_vol_dt1;
	_svm_sreg_buf[vc].m_vol_left = v10;
	_svm_sreg_buf[vc].m_vol_right = v11;
	_svm_sreg_dirty[(s16)vc] |= 3;
}
