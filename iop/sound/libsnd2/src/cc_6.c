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

void _SsContDataEntry(s16 sep_no, s16 seq_no, u8 control_value)
{
	libsnd2_sequence_struct_t *score_struct;
	int m_channel_idx;
	ProgAtr prog_attr;
	VagAtr old_vag_attr;

	score_struct = &_ss_score[sep_no][seq_no];
	m_channel_idx = score_struct->m_channel_idx;
	SsUtGetProgAtr((char)score_struct->m_vab_id, (u8)score_struct->m_programs[m_channel_idx], &prog_attr);
	if ( score_struct->m_unk1C == 1 )
	{
		if ( !score_struct->m_unk15 )
		{
			score_struct->m_unk1D = control_value;
			score_struct->m_unk1C = 0;
			score_struct->m_unk15 = 1;
			score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
			return;
		}
	}
	if ( score_struct->m_unk1E == 2 )
	{
		int tone_idx_cur;

		for ( tone_idx_cur = 0; tone_idx_cur < prog_attr.tones; tone_idx_cur += 1 )
		{
			SsUtGetVagAtr(
				(char)score_struct->m_vab_id, (u8)score_struct->m_programs[m_channel_idx], tone_idx_cur, &old_vag_attr);
			if ( (u8)score_struct->m_unk18 == 0 )
			{
				old_vag_attr.pbmax = control_value & 0x7F;
				old_vag_attr.pbmin = control_value & 0x7F;
			}
			SsUtSetVagAtr(
				(char)score_struct->m_vab_id, (u8)score_struct->m_programs[m_channel_idx], tone_idx_cur, &old_vag_attr);
		}
		if ( score_struct->m_unk19 )
		{
			score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
			score_struct->m_unk1E = 0;
			return;
		}
	}
	if ( score_struct->m_unk1F != 2 )
	{
		score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
		return;
	}
	if ( score_struct->m_unk1B == 16 )
		SsFCALL.ccentry[(u8)score_struct->m_fn_idx](
			(char)score_struct->m_vab_id,
			(u8)score_struct->m_programs[m_channel_idx],
			0,
			old_vag_attr,
			score_struct->m_fn_idx,
			control_value);
	else
		SsFCALL.ccentry[(u8)score_struct->m_fn_idx](
			(char)score_struct->m_vab_id,
			(u8)score_struct->m_programs[m_channel_idx],
			score_struct->m_unk1B,
			old_vag_attr,
			score_struct->m_fn_idx,
			control_value);
	score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
	score_struct->m_unk1F = 0;
}
