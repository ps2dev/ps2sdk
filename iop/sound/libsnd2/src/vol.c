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

void _SsSndSetVolData(s16 sep_no, s16 seq_no, s16 vol, int v_time)
{
	libsnd2_sequence_struct_t *score_struct;
	unsigned int m_flags;

	score_struct = &_ss_score[sep_no][seq_no];
	m_flags = score_struct->m_flags;
	if ( (m_flags & 4) == 0 && (m_flags & 0x100) == 0 && vol != 0 )
	{
		score_struct->m_unk48 = vol;
		score_struct->m_unk9C = v_time;
		score_struct->m_unkA0 = 0;
		score_struct->m_unk4A = 0;
	}
}
