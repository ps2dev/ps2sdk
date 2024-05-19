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

void _SsSndNextSep(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	score_struct->m_l_count = 1;
	score_struct->m_unk21 = 0;
	score_struct->m_flags &= ~0x100u;
	score_struct->m_flags &= ~8u;
	score_struct->m_flags &= ~2u;
	score_struct->m_flags &= ~4u;
	score_struct->m_flags &= ~0x200u;
	score_struct->m_play_mode = SSPLAY_PLAY;
	score_struct->m_seq_ptr = score_struct->m_unk04;
	score_struct->m_flags |= 1u;
}
