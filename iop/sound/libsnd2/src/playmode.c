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

void Snd_SetPlayMode(s16 sep_no, s16 seq_no, char play_mode, char l_count)
{
	libsnd2_sequence_struct_t *score_struct;
	u8 *m_unk04;

	score_struct = &_ss_score[sep_no][seq_no];
	m_unk04 = score_struct->m_unk04;
	score_struct->m_seq_ptr = m_unk04;
	score_struct->m_unk08 = m_unk04;
	score_struct->m_unk0C = m_unk04;
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
			score_struct->m_play_mode = SSPLAY_PLAY;
			score_struct->m_unk21 = 0;
			_SsVmSetSeqVol((sep_no | (seq_no << 8)), score_struct->m_voll, score_struct->m_volr);
			break;
		default:
			break;
	}
}
