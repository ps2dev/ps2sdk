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

void _SsContNrpn2(s16 sep_no, s16 seq_no, u8 control_value)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	if ( control_value == 20 )
	{
		score_struct->m_unk1B = control_value;
		score_struct->m_unk1C = 1;
		score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
		score_struct->m_unk08 = score_struct->m_seq_ptr;
		return;
	}
	if ( control_value != 30 )
	{
		score_struct->m_unk1B = control_value;
		score_struct->m_unk1F += 1;
		score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
		return;
	}
	score_struct->m_unk1B = control_value;
	if ( score_struct->m_unk1D == 0 )
	{
		score_struct->m_unk15 = 0;
		score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
		return;
	}
	if ( score_struct->m_unk1D >= 0x7Fu )
	{
		_SsReadDeltaValue(sep_no, seq_no);
		score_struct->m_delta_value = 0;
		score_struct->m_seq_ptr = score_struct->m_unk08;
	}
	else
	{
		score_struct->m_unk1D -= 1;
		score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
		if ( score_struct->m_unk1D )
			score_struct->m_seq_ptr = score_struct->m_unk08;
		else
			score_struct->m_unk15 = 0;
	}
}
