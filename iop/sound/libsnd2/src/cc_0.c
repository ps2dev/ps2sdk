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

void _SsContBankChange(s16 sep_no, s16 seq_no, u8 control_value)
{
	libsnd2_sequence_struct_t *score_struct;

	(void)control_value;

	score_struct = &_ss_score[sep_no][seq_no];
	score_struct->m_vab_id = *score_struct->m_seq_ptr;
	score_struct->m_seq_ptr += 1;
	score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
}
