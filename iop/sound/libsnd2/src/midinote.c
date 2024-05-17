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

void _SsNoteOn(s16 sep_no, s16 seq_no, u8 note, u8 vollr)
{
	libsnd2_sequence_struct_t *score_struct;
	int m_channel_idx;

	score_struct = &_ss_score[sep_no][seq_no];
	m_channel_idx = score_struct->m_channel_idx;
	if ( vollr == 0 )
	{
		_SsVmKeyOff(
			sep_no | (u16)(seq_no << 8), (char)score_struct->m_vab_id, (u8)score_struct->m_programs[m_channel_idx], note);
	}
	else if ( ((score_struct->m_channel_mute >> m_channel_idx) & 1) == 0 )
		_SsVmKeyOn(
			sep_no | (u16)(seq_no << 8),
			(char)score_struct->m_vab_id,
			(u8)score_struct->m_programs[m_channel_idx],
			note,
			vollr,
			(u8)score_struct->m_panpot[m_channel_idx]);
}
