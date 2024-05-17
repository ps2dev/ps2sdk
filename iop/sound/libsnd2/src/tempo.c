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

void _SsSndTempo(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	int unk_a8_dec;
	int m_unk4E;
	unsigned int calc;

	score_struct = &_ss_score[sep_no][seq_no];
	unk_a8_dec = score_struct->m_unkA8 - 1;
	score_struct->m_unkA8 = unk_a8_dec;
	if ( unk_a8_dec < 0 )
	{
		score_struct->m_flags &= ~0x40u;
		score_struct->m_flags &= ~0x80u;
		return;
	}
	m_unk4E = score_struct->m_unk4E;
	if ( m_unk4E < 0 )
	{
		unsigned int m_unk94;
		unsigned int m_unkAC;
		unsigned int v14;

		m_unk94 = score_struct->m_unk94;
		m_unkAC = score_struct->m_unkAC;
		v14 = m_unk94 + m_unk4E;
		if ( m_unkAC >= m_unk94 )
		{
			if ( m_unk94 < m_unkAC )
			{
				unsigned int v16;

				v16 = m_unk94 - m_unk4E;
				m_unkAC = score_struct->m_unkAC;
				score_struct->m_unk94 = v16;
				if ( m_unkAC < v16 )
					score_struct->m_unk94 = m_unkAC;
			}
		}
		else
		{
			score_struct->m_unk94 = v14;
			if ( v14 < m_unkAC )
				score_struct->m_unk94 = m_unkAC;
		}
	}
	else if ( m_unk4E > 0 )
	{
		unsigned int v9;
		unsigned int v10;

		if ( unk_a8_dec % m_unk4E )
			return;
		v9 = score_struct->m_unk94;
		v10 = score_struct->m_unkAC;
		if ( v10 < v9 )
			score_struct->m_unk94 = v9 - 1;
		else if ( v9 < v10 )
			score_struct->m_unk94 = v9 + 1;
	}
	else
	{
		__builtin_trap();
	}
	calc = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_unk94 / (unsigned int)(60 * VBLANK_MINUS);
	if ( (60 * VBLANK_MINUS) == 0 )
		__builtin_trap();
	score_struct->m_unk54 = calc;
	if ( (int)calc <= 0 )
		score_struct->m_unk54 = 1;
	if ( !score_struct->m_unkA8 || score_struct->m_unk94 == score_struct->m_unkAC )
	{
		score_struct->m_flags &= ~0x40u;
		score_struct->m_flags &= ~0x80u;
	}
}
