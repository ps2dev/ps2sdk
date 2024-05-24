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

void SsSetTempo(s16 sep_no, s16 seq_no, s16 tempo)
{
	libsnd2_sequence_struct_t *score_struct;
	unsigned int v4;
	int v5;
	int v6;
	unsigned int v7;

	score_struct = &_ss_score[sep_no][seq_no];
	v4 = score_struct->m_resolution_of_quarter_note * tempo;
	v5 = VBLANK_MINUS;
	v6 = 15 * VBLANK_MINUS;
	score_struct->m_unk94 = tempo;
	v7 = 4 * v6;
	if ( 10 * v4 < (unsigned int)(4 * v6) )
	{
		unsigned int v9;

		if ( !v4 )
			__builtin_trap();
		v9 = 600 * v5 / v4;
		score_struct->m_unk52 = v9;
		score_struct->m_unk54 = v9;
	}
	else
	{
		unsigned int v11;
		unsigned int v12;

		v11 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_unk94 / v7;
		v12 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_unk94 % v7;
		score_struct->m_unk52 = -1;
		score_struct->m_unk54 = v11;
		if ( (unsigned int)(2 * v6) < v12 )
		{
			score_struct->m_unk54 = v11 + 1;
		}
	}
	score_struct->m_unk56 = score_struct->m_unk54;
}
