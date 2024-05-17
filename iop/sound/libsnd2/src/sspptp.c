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

void SsSeqPlayPtoP(s16 sep_no, s16 seq_no, u8 *start_point, u8 *end_point, char play_mode, s16 l_count)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	if ( start_point == end_point || end_point < start_point )
	{
		printf("bad address setting!!\n");
		return;
	}
	score_struct->m_seq_ptr = start_point;
	score_struct->m_unk08 = start_point;
	score_struct->m_unk0C = start_point;
	score_struct->m_unk10 = end_point;
	score_struct->m_flags &= ~0x200u;
	score_struct->m_flags &= ~4u;
	score_struct->m_l_count = l_count;
	switch ( play_mode )
	{
		case SSPLAY_PAUSE:
			score_struct->m_flags |= 2u;
			break;
		case SSPLAY_PLAY:
			score_struct->m_flags |= 1u;
			score_struct->m_flags |= 0x400u;
			score_struct->m_play_mode = SSPLAY_PLAY;
			score_struct->m_unk21 = 0;
			_SsVmSetSeqVol((sep_no | (seq_no << 8)), score_struct->m_voll, score_struct->m_volr);
			break;
		default:
			break;
	}
}
