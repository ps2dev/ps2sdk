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

void _SsSndStop(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	int i_tmp;
	u8 *m_unk04;

	score_struct = &_ss_score[sep_no][seq_no];
	score_struct->m_flags &= ~1u;
	score_struct->m_flags &= ~2u;
	score_struct->m_flags &= ~8u;
	score_struct->m_flags &= ~0x400u;
	score_struct->m_flags |= 4u;
	_SsVmSeqKeyOff(sep_no | (seq_no << 8));
	_SsVmDamperOff();
	m_unk04 = score_struct->m_unk04;
	score_struct->m_play_mode = SSPLAY_PAUSE;
	score_struct->m_unk88 = 0;
	score_struct->m_unk1C = 0;
	score_struct->m_unk18 = 0;
	score_struct->m_unk19 = 0;
	score_struct->m_unk1E = 0;
	score_struct->m_fn_idx = 0;
	score_struct->m_unk1B = 0;
	score_struct->m_unk1F = 0;
	score_struct->m_channel_idx = 0;
	score_struct->m_unk21 = 0;
	score_struct->m_unk1C = 0;
	score_struct->m_unk1D = 0;
	score_struct->m_unk15 = 0;
	score_struct->m_running_status = 0;
	score_struct->m_delta_value = score_struct->m_unk84;
	score_struct->m_unk94 = score_struct->m_tempo;
	score_struct->m_unk54 = score_struct->m_unk56;
	score_struct->m_seq_ptr = m_unk04;
	score_struct->m_unk08 = m_unk04;
	for ( i_tmp = 0; i_tmp < 16; i_tmp += 1 )
	{
		score_struct->m_programs[i_tmp] = i_tmp;
		score_struct->m_panpot[i_tmp] = 64;
		score_struct->m_vol[i_tmp] = 127;
	}
	score_struct->m_unk5C = 127;
	score_struct->m_unk5E = 127;
}

void SsSeqStop(s16 sep_no)
{
	_SsSndStop(sep_no, 0);
}

void SsSepStop(s16 sep_no, s16 seq_no)
{
	_SsSndStop(sep_no, seq_no);
}
