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

void _SsContNrpn1(s16 sep_no, s16 seq_no, u8 control_value)
{
	libsnd2_sequence_struct_t *score_struct;
	int m_unk1B;

	score_struct = &_ss_score[sep_no][seq_no];
	m_unk1B = score_struct->m_unk1B;
	if ( m_unk1B == 40 )
	{
		libsnd2_ss_mark_callback_proc pFunc;

		pFunc = _SsMarkCallback[sep_no].m_entries[seq_no];
		if ( pFunc )
			pFunc(sep_no, seq_no, control_value);
		m_unk1B = score_struct->m_unk1B;
	}
	if ( !(m_unk1B == 30 || m_unk1B == 20) && (m_unk1B != 40) )
	{
		score_struct->m_fn_idx = control_value;
		score_struct->m_unk1C = 0;
		score_struct->m_unk1F += 1;
	}
	score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
}
