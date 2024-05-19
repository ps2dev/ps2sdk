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

void _SsSndCrescendo(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	score_struct->m_unkA0 += 1;
	if ( score_struct->m_unk9C >= score_struct->m_unkA0 )
	{
		int mulfield_tmp;
		int new_field_4a;
		int vol_inc_by;

		mulfield_tmp = score_struct->m_unk48 * score_struct->m_unkA0;
		new_field_4a = mulfield_tmp / score_struct->m_unk9C;
		if ( score_struct->m_unk9C == -1 && (unsigned int)mulfield_tmp == 0x80000000 )
			__builtin_trap();
		vol_inc_by = new_field_4a - score_struct->m_unk4A;
		if ( new_field_4a != score_struct->m_unk4A )
		{
			int voll_clamped;
			int volr_clamped;
			s16 seq_left_vol;
			s16 seq_right_vol;

			score_struct->m_unk4A = new_field_4a;
			_SsVmGetSeqVol(sep_no | (seq_no << 8), &seq_left_vol, &seq_right_vol);
			voll_clamped = (u16)seq_left_vol + vol_inc_by;
			if ( voll_clamped >= 128 )
				voll_clamped = 127;
			if ( voll_clamped < 0 )
				voll_clamped = 0;
			volr_clamped = (u16)seq_right_vol + vol_inc_by;
			if ( volr_clamped >= 128 )
				volr_clamped = 127;
			if ( volr_clamped < 0 )
				volr_clamped = 0;
			_SsVmSetSeqVol(sep_no | (seq_no << 8), voll_clamped, volr_clamped);
			if ( (voll_clamped == 127 && volr_clamped == 127) || (voll_clamped == 0 && volr_clamped == 0) )
			{
				score_struct->m_flags &= ~0x10u;
			}
		}
	}
	else
	{
		score_struct->m_flags &= ~0x10u;
	}
	_SsVmGetSeqVol(sep_no | (seq_no << 8), &score_struct->m_unk5C, &score_struct->m_unk5E);
}
