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

s32 _SsReadDeltaValue(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	int delta_value;
	s32 result;
	int dv_mul4;

	score_struct = &_ss_score[sep_no][seq_no];
	delta_value = *score_struct->m_seq_ptr;
	score_struct->m_seq_ptr += 1;
	if ( !delta_value )
		return 0;
	if ( (delta_value & 0x80) != 0 )
	{
		char next_byte;

		delta_value &= ~0x80;
		do
		{
			next_byte = *score_struct->m_seq_ptr;
			score_struct->m_seq_ptr += 1;
			delta_value = (delta_value << 7) + (next_byte & 0x7F);
		} while ( (next_byte & 0x80) != 0 );
	}
	dv_mul4 = 4 * delta_value;
	result = 2 * (dv_mul4 + delta_value);
	score_struct->m_unk88 += result;
	return result;
}
