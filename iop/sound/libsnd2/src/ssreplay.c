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

void _SsSndSetReplayMode(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	unsigned int m_flags;

	score_struct = &_ss_score[sep_no][seq_no];
	m_flags = score_struct->m_flags;
	if ( (m_flags & 0x204) == 0 && (m_flags & 0x100) == 0 )
	{
		score_struct->m_flags = m_flags & ~2;
		score_struct->m_flags |= 8u;
		score_struct->m_flags |= 1u;
	}
}

void SsSeqReplay(s16 sep_no)
{
	_SsSndSetReplayMode(sep_no, 0);
}

void SsSepReplay(s16 sep_no, s16 seq_no)
{
	_SsSndSetReplayMode(sep_no, seq_no);
}
