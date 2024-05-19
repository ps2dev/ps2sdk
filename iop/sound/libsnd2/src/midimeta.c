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

void _SsGetMetaEvent(s16 sep_no, s16 seq_no, u8 meta_event)
{
	libsnd2_sequence_struct_t *score_struct;
	u8 v4;
	u8 v6;
	int v7;
	unsigned int v8;
	int v9;
	int v10;
	unsigned int v11;

	(void)meta_event;

	score_struct = &_ss_score[sep_no][seq_no];
	v4 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v6 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v7 = 60000000 / ((v4 << 16) | (v6 << 8) | *(score_struct->m_seq_ptr));
	score_struct->m_seq_ptr += 1;
	v8 = score_struct->m_resolution_of_quarter_note * v7;
	v9 = VBLANK_MINUS;
	v10 = 16 * VBLANK_MINUS;
	score_struct->m_unk94 = v7;
	v11 = 4 * (v10 - v9);
	if ( 10 * v8 < v11 )
	{
		unsigned int v13;

		if ( !v8 )
			__builtin_trap();
		v13 = 600 * v9 / v8;
		score_struct->m_unk52 = v13;
		score_struct->m_unk54 = v13;
	}
	else
	{
		unsigned int v15;
		unsigned int v16;

		v15 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_unk94 / v11;
		v16 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_unk94 % v11;
		score_struct->m_unk52 = -1;
		score_struct->m_unk54 = v15;
		if ( (unsigned int)(2 * (v10 - v9)) < v16 )
		{
			score_struct->m_unk54 = v15 + 1;
		}
	}
	score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
}
