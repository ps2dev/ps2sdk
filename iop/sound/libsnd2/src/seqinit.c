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

s16 _SsInitSoundSeq(s16 seq_no, s16 vab_id, u8 *addr)
{
	int v4;
	libsnd2_sequence_struct_t *score_struct;
	int v8;
	u8 v9;
	u8 v11;
	u8 v12;
	u8 v13;
	u8 v14;
	int tempo;
	int delta_value;
	unsigned int v20;
	int tmp1;
	unsigned int tmp2;

	score_struct = &_ss_score[seq_no][0];
	score_struct->m_vab_id = vab_id;
	score_struct->m_resolution_of_quarter_note = 0;
	score_struct->m_unk18 = 0;
	score_struct->m_unk19 = 0;
	score_struct->m_unk1E = 0;
	score_struct->m_fn_idx = 0;
	score_struct->m_unk1B = 0;
	score_struct->m_unk1F = 0;
	score_struct->m_channel_idx = 0;
	score_struct->m_unk84 = 0;
	score_struct->m_unk88 = 0;
	score_struct->m_tempo = 0;
	score_struct->m_unk56 = 0;
	score_struct->m_l_count = 1;
	score_struct->m_unk21 = 0;
	score_struct->m_play_mode = SSPLAY_PAUSE;
	score_struct->m_delta_value = 0;
	score_struct->m_unk1C = 0;
	score_struct->m_unk1D = 0;
	score_struct->m_unk15 = 0;
	score_struct->m_running_status = 0;
	score_struct->m_channel_mute = 0;
	score_struct->m_rhythm_n = 0;
	score_struct->m_rhythm_d = 0;
	for ( v4 = 0; v4 < 16; v4 += 1 )
	{
		score_struct->m_programs[v4] = v4;
		score_struct->m_panpot[v4] = 64;
		score_struct->m_vol[v4] = 127;
	}
	score_struct->m_unk52 = 1;
	score_struct->m_seq_ptr = addr;
	v8 = *addr;
	score_struct->m_seq_ptr += 7;
	if ( v8 != 83 && v8 != 112 )
	{
		printf("This is an old SEQ Data Format.\n");
		return 0;
	}
	v9 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	if ( v9 != 1 )
	{
		printf("This is not SEQ Data.\n");
		return -1;
	}
	v11 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v12 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	score_struct->m_resolution_of_quarter_note = v12 | (v11 << 8);
	v13 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	v14 = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	tempo = 60000000 / ((v13 << 16) | (v14 << 8) | *(score_struct->m_seq_ptr));
	score_struct->m_seq_ptr += 1;
	score_struct->m_tempo = tempo;
	score_struct->m_unk94 = score_struct->m_tempo;
	score_struct->m_rhythm_n = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	score_struct->m_rhythm_d = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	delta_value = _SsReadDeltaValue(seq_no, 0);
	v20 = score_struct->m_resolution_of_quarter_note * score_struct->m_tempo;
	score_struct->m_unk84 = delta_value;
	score_struct->m_delta_value = delta_value;
	score_struct->m_unk10 = 0;
	score_struct->m_unk08 = score_struct->m_seq_ptr;
	score_struct->m_unk04 = score_struct->m_seq_ptr;
	score_struct->m_unk0C = score_struct->m_seq_ptr;
	tmp1 = VBLANK_MINUS;
	tmp2 = 60 * tmp1;
	if ( 10 * v20 < (unsigned int)(60 * tmp1) )
	{
		unsigned int v25;

		if ( !v20 )
			__builtin_trap();
		v25 = 600 * tmp1 / v20;
		score_struct->m_unk52 = v25;
		score_struct->m_unk54 = v25;
	}
	else
	{
		unsigned int v27;
		unsigned int v28;

		v27 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_tempo / tmp2;
		v28 = 10 * score_struct->m_resolution_of_quarter_note * score_struct->m_tempo % tmp2;
		score_struct->m_unk52 = -1;
		score_struct->m_unk54 = v27;
		if ( (unsigned int)(30 * tmp1) < v28 )
		{
			score_struct->m_unk54 = v27 + 1;
		}
	}
	score_struct->m_unk56 = score_struct->m_unk54;
	return 0;
}
