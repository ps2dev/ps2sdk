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

void _SsSndSetAccele(s16 sep_no, s16 seq_no, int tempo, int v_time)
{
	libsnd2_sequence_struct_t *score_struct;
	unsigned int m_flags;

	score_struct = &_ss_score[sep_no][seq_no];
	m_flags = score_struct->m_flags;
	if ( (m_flags & 1) != 0 && (m_flags & 2) == 0 )
	{
		int m_unk94;

		m_unk94 = score_struct->m_unk94;
		if ( tempo != m_unk94 )
		{
			score_struct->m_unkA8 = v_time;
			score_struct->m_unkA4 = v_time;
			score_struct->m_unkAC = tempo;
			if (
				((tempo - m_unk94 >= 0) && (tempo - m_unk94 < v_time))
				|| ((tempo - m_unk94 < 0) && (m_unk94 - tempo < v_time)) )
			{
				int v9;
				int m_unkA8;
				int v11;
				int v13;

				v9 = score_struct->m_unk94;
				m_unkA8 = score_struct->m_unkA8;
				v11 = tempo - v9;
				v13 = v9 - tempo;
				if ( v11 < 0 )
				{
					if ( v13 == -1 && (unsigned int)m_unkA8 == 0x80000000 )
						__builtin_trap();
					score_struct->m_unk4E = m_unkA8 / v13;
				}
				else if ( v11 > 0 )
				{
					score_struct->m_unk4E = m_unkA8 / v11;
				}
				else
				{
					__builtin_trap();
				}
			}
			else
			{
				int v14;
				int v15;
				int v16;

				v14 = score_struct->m_unk94;
				v15 = score_struct->m_unkA8;
				v16 = tempo - v14;
				if ( tempo - v14 < 0 )
					v16 = v14 - tempo;
				if ( v15 == -1 && (unsigned int)v16 == 0x80000000 )
					__builtin_trap();
				score_struct->m_unk4E = ~(u16)(v16 / v15);
			}
			score_struct->m_flags |= 0x40u;
			score_struct->m_flags &= ~0x80u;
		}
	}
}

void SsSeqSetAccelerando(s16 seq_no, int tempo, int v_time)
{
	_SsSndSetAccele(seq_no, 0, tempo, v_time);
}

void SsSepSetAccelerando(s16 seq_no, s16 sep_no, int tempo, int v_time)
{
	_SsSndSetAccele(seq_no, sep_no, tempo, v_time);
}
