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

void _SsSndSetPauseMode(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	_SsVmGetSeqVol(sep_no | (seq_no << 8), &score_struct->m_unk5C, &score_struct->m_unk5E);
	score_struct->m_flags &= ~1u;
	score_struct->m_flags &= ~8u;
	score_struct->m_flags |= 2u;
}

void SsSeqPause(s16 sep_no)
{
	_SsSndSetPauseMode(sep_no, 0);
}

void SsSepPause(s16 sep_no, s16 seq_no)
{
	_SsSndSetPauseMode(sep_no, seq_no);
}
