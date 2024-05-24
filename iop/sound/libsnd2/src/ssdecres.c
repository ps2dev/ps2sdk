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

void _SsSndSetDecres(s16 sep_no, s16 seq_no, s16 vol, int v_time)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	_SsSndSetVolData(sep_no, seq_no, -vol, v_time);
	score_struct->m_flags |= 0x20u;
	score_struct->m_flags &= ~0x10u;
}

void SsSeqSetDecrescendo(s16 sep_no, s16 vol, int v_time)
{
	_SsSndSetDecres(sep_no, 0, vol, v_time);
}

void SsSepSetDecrescendo(s16 sep_no, s16 seq_no, s16 vol, int v_time)
{
	_SsSndSetDecres(sep_no, seq_no, vol, v_time);
}
