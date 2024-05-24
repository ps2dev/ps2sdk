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

void SsSetNext(s16 sep_no1, s16 seq_no1, s16 sep_no2, s16 seq_no2)
{
	libsnd2_sequence_struct_t *score_struct_1;
	libsnd2_sequence_struct_t *score_struct_2;

	score_struct_1 = &_ss_score[sep_no1][seq_no1];
	score_struct_1->m_next_seq = seq_no2;
	score_struct_1->m_next_sep = sep_no2;
	score_struct_2 = &_ss_score[sep_no2][seq_no2];
	score_struct_2->m_flags |= 0x100u;
	score_struct_2->m_play_mode = SSPLAY_PAUSE;
}

void SsSeqSetNext(s16 sep_no1, s16 sep_no2)
{
	libsnd2_sequence_struct_t *score_struct_1;
	libsnd2_sequence_struct_t *score_struct_2;

	score_struct_1 = &_ss_score[sep_no1][0];
	score_struct_1->m_next_sep = sep_no2;
	score_struct_1->m_next_seq = 0;
	score_struct_2 = &_ss_score[sep_no2][0];
	score_struct_2->m_flags |= 0x100u;
	score_struct_2->m_play_mode = SSPLAY_PAUSE;
}
