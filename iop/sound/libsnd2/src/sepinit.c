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

int _SsInitSoundSep(s16 sep_no, int seq_no, u8 vab_id, u8 *addr)
{
	int v5;
	int v6;
	libsnd2_sequence_struct_t *score_struct;
	u8 v13;
	u8 v15;
	u8 v16;
	u8 v17;
	int tempo;
	char v20;
	char v21;
	int v22;
	u8 v23;
	u8 v24;
	u8 v25;
	s32 delta_value;
	int v27;
	unsigned int v28;
	int tmp1;
	unsigned int tmp2;
	int v33;

	v5 = 0;
	score_struct = &_ss_score[sep_no][seq_no];
	score_struct->m_l_count = 1;
	score_struct->m_unk15 = 0;
	score_struct->m_running_status = 0;
	score_struct->m_channel_idx = 0;
	score_struct->m_unk18 = 0;
	score_struct->m_unk19 = 0;
	score_struct->m_fn_idx = 0;
	score_struct->m_unk1B = 0;
	score_struct->m_unk1C = 0;
	score_struct->m_unk1D = 0;
	score_struct->m_unk1E = 0;
	score_struct->m_unk1F = 0;
	score_struct->m_play_mode = SSPLAY_PAUSE;
	score_struct->m_unk21 = 0;
	score_struct->m_unk52 = 1;
	score_struct->m_resolution_of_quarter_note = 0;
	score_struct->m_vab_id = vab_id;
	score_struct->m_unk56 = 0;
	score_struct->m_unk84 = 0;
	score_struct->m_unk88 = 0;
	score_struct->m_tempo = 0;
	score_struct->m_delta_value = 0;
	score_struct->m_channel_mute = 0;
	score_struct->m_rhythm_n = 0;
	score_struct->m_rhythm_d = 0;
	for ( v6 = 0; v6 < 16; v6 += 1 )
	{
		score_struct->m_panpot[v6] = 64;
		score_struct->m_programs[v6] = v6;
		score_struct->m_vol[v6] = 127;
	}
	score_struct->m_seq_ptr = addr;
	if ( (seq_no & 0xFFFF) != 0 )
	{
		score_struct->m_seq_ptr += 2;
		v5 = 2;
	}
	else
	{
		int v10;

		v10 = *addr;
		if ( v10 == 0x53 || v10 == 0x70 )
		{
			int v11;

			score_struct->m_seq_ptr += 5;
			v11 = *(score_struct->m_seq_ptr);
			score_struct->m_seq_ptr += 1;
			if ( v11 )
			{
				printf("This is not SEP Data.\n");
				return -1;
			}
			score_struct->m_seq_ptr += 2;
			v5 = 8;
		}
	}
	v13 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v15 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	score_struct->m_resolution_of_quarter_note = v15 | (v13 << 8);
	v16 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v17 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	tempo = 60000000 / ((v16 << 16) | (v17 << 8) | *(score_struct->m_seq_ptr));
	score_struct->m_seq_ptr += 1;
	score_struct->m_tempo = tempo;
	score_struct->m_unk94 = score_struct->m_tempo;
	v20 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	score_struct->m_rhythm_n = v20;
	v21 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	score_struct->m_rhythm_d = v21;
	v22 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v23 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v24 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v25 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v27 = (v22 << 24) + (v23 << 16) + (v24 << 8) + v25;
	delta_value = _SsReadDeltaValue(sep_no, seq_no);
	v28 = score_struct->m_resolution_of_quarter_note * score_struct->m_tempo;
	score_struct->m_unk84 = delta_value;
	score_struct->m_delta_value = delta_value;
	score_struct->m_unk10 = 0;
	score_struct->m_unk08 = score_struct->m_seq_ptr;
	score_struct->m_unk04 = score_struct->m_seq_ptr;
	score_struct->m_unk0C = score_struct->m_seq_ptr;
	tmp1 = VBLANK_MINUS;
	tmp2 = 60 * tmp1;
	v33 = v5 + 11;
	if ( 10 * v28 < (unsigned int)(60 * tmp1) )
	{
		unsigned int v34;

		if ( !v28 )
			__builtin_trap();
		v34 = 600 * tmp1 / v28;
		score_struct->m_unk52 = v34;
		score_struct->m_unk54 = v34;
	}
	else
	{
		unsigned int v36;
		unsigned int v37;

		v36 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_tempo / tmp2;
		v37 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_tempo % tmp2;
		score_struct->m_unk52 = -1;
		score_struct->m_unk54 = v36;
		if ( (unsigned int)(30 * tmp1) < v37 )
		{
			score_struct->m_unk54 = v36 + 1;
		}
	}
	score_struct->m_unk56 = score_struct->m_unk54;
	return v33 + v27;
}
