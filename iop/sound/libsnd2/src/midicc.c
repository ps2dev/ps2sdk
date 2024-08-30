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

void _SsSetControlChange(s16 sep_no, s16 seq_no, u8 control_value)
{
	libsnd2_sequence_struct_t *score_struct;
	u8 control_value_tmp;

	score_struct = &_ss_score[sep_no][seq_no];
	control_value_tmp = *score_struct->m_seq_ptr;
	score_struct->m_seq_ptr += 1;
	switch ( control_value )
	{
		case 0:
		{
			score_struct->m_vab_id = control_value_tmp;
			break;
		}
		case 6:
		{
			SsFCALL.control[CC_DATAENTRY](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 7:
		{
			SsFCALL.control[CC_MAINVOL](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 10:
		{
			SsFCALL.control[CC_PANPOT](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 11:
		{
			SsFCALL.control[CC_EXPRESSION](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 64:
		{
			SsFCALL.control[CC_DAMPER](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 91:
		{
			SsFCALL.control[CC_EXTERNAL](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 98:
		{
			SsFCALL.control[CC_NRPN1](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 99:
		{
			SsFCALL.control[CC_NRPN2](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 100:
		{
			SsFCALL.control[CC_RPN1](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 101:
		{
			SsFCALL.control[CC_RPN2](sep_no, seq_no, control_value_tmp);
			return;
		}
		case 121:
		{
			SsFCALL.control[CC_RESETALL](sep_no, seq_no, control_value_tmp);
			return;
		}
		default:
		{
			break;
		}
	}
	score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
}
