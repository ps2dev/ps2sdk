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

void _SsSndSetVol(s16 sep_no, s16 seq_no, u16 voll, u16 volr)
{
	libsnd2_sequence_struct_t *score_struct;

	score_struct = &_ss_score[sep_no][seq_no];
	if ( score_struct->m_flags == 1 )
	{
		_SsVmSetSeqVol(sep_no | (seq_no << 8), voll, volr);
	}
	else
	{
		score_struct->m_voll = voll;
		score_struct->m_volr = volr;
	}
}

void SsSeqSetVol(s16 sep_no, s16 voll, s16 volr)
{
	_SsSndSetVol(sep_no, 0, voll, volr);
}

void SsSepSetVol(s16 sep_no, s16 seq_no, s16 voll, s16 volr)
{
	_SsSndSetVol(sep_no, seq_no, voll, volr);
}

void SsSeqGetVol(s16 sep_no, s16 seq_no, s16 *voll, s16 *volr)
{
	_SsVmGetSeqVol(sep_no | (seq_no << 8), voll, volr);
}
