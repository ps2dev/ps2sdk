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

void _SsContResetAll(s16 sep_no, s16 seq_no, u8 control_value)
{
	libsnd2_sequence_struct_t *score_struct;

	(void)control_value;

	score_struct = &_ss_score[sep_no][seq_no];
	SsUtReverbOff();
	_SsVmDamperOff();
	score_struct->m_programs[score_struct->m_channel_idx] = score_struct->m_channel_idx;
	score_struct->m_unk18 = 0;
	score_struct->m_unk19 = 0;
	score_struct->m_vol[score_struct->m_channel_idx] = 127;
	score_struct->m_panpot[score_struct->m_channel_idx] = 64;
	score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
}
