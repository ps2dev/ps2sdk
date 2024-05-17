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

static void _SsClose(s16 seq_sep_no)
{
	int sep_no;
	int seq_no;

	_SsVmSetSeqVol(seq_sep_no, 0, 0);
	_SsVmSeqKeyOff(seq_sep_no);
	_snd_openflag &= ~(1 << seq_sep_no);
	sep_no = seq_sep_no;
	for ( seq_no = 0; seq_no < _snd_seq_t_max; seq_no += 1 )
	{
		libsnd2_sequence_struct_t *score_struct;

		score_struct = &_ss_score[sep_no][seq_no];
		score_struct->m_flags = 0;
		score_struct->m_next_sep = 255;
		score_struct->m_next_seq = 0;
		score_struct->m_unk48 = 0;
		score_struct->m_unk4A = 0;
		score_struct->m_unk9C = 0;
		score_struct->m_unkA0 = 0;
		score_struct->m_unk4C = 0;
		score_struct->m_unkAC = 0;
		score_struct->m_unkA8 = 0;
		score_struct->m_unkA4 = 0;
		score_struct->m_unk4E = 0;
		score_struct->m_voll = 127;
		score_struct->m_volr = 127;
	}
}

void SsSeqClose(s16 seq_sep_no)
{
	_SsClose(seq_sep_no);
}

void SsSepClose(s16 seq_sep_no)
{
	_SsClose(seq_sep_no);
}
