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

void SsSetTableSize(char *table, s16 s_max, s16 t_max)
{
	int typed_table_i;
	unsigned int openflag_i;
	int i;

	_snd_seq_s_max = s_max;
	_snd_seq_t_max = t_max;
	for ( typed_table_i = 0; typed_table_i < s_max; typed_table_i += 1 )
	{
		_ss_score[typed_table_i] =
			(libsnd2_sequence_struct_t *)&table[sizeof(libsnd2_sequence_struct_t) * (t_max * typed_table_i)];
	}
	for ( openflag_i = s_max; openflag_i < 32; openflag_i += 1 )
	{
		_snd_openflag |= (u32)1 << openflag_i;
	}
	for ( i = 0; i < _snd_seq_s_max; i += 1 )
	{
		int j;

		j = 0;
		for ( j = 0; j < _snd_seq_t_max; j += 1 )
		{
			libsnd2_sequence_struct_t *score_struct;

			score_struct = &_ss_score[i][j];
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
			score_struct->m_unk5C = 127;
			score_struct->m_unk5E = 127;
		}
	}
}
