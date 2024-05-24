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

void _SsSeqPlay(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	int m_unk54;
	int m_delta_value;
	int diff1;

	score_struct = &_ss_score[sep_no][seq_no];
	m_unk54 = score_struct->m_unk54;
	m_delta_value = score_struct->m_delta_value;
	diff1 = m_delta_value - m_unk54;
	if ( diff1 <= 0 )
	{
		int v10;
		int v11;
		int v15;

		v11 = score_struct->m_delta_value;
		if ( m_unk54 < m_delta_value )
			return;
		do
		{
			int v14;

			do
			{
				_SsGetSeqData(sep_no, seq_no);
				v14 = score_struct->m_delta_value;
			} while ( !v14 );
			v15 = score_struct->m_unk54;
			v11 += v14;
			v10 = v11 - v15;
		} while ( v11 < v15 );
		score_struct->m_delta_value = v10;
	}
	else
	{
		if ( score_struct->m_unk52 > 0 )
		{
			score_struct->m_unk52 -= 1;
			return;
		}
		if ( score_struct->m_unk52 )
		{
			score_struct->m_delta_value = diff1;
			return;
		}
		score_struct->m_unk52 = score_struct->m_unk54;
		score_struct->m_delta_value -= 1;
	}
}

void _SsSeqGetEof(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	int m_l_count;
	u8 *m_unk04;

	score_struct = &_ss_score[sep_no][seq_no];
	m_l_count = score_struct->m_l_count;
	score_struct->m_unk21 += 1;
	if ( m_l_count != SSPLAY_INFINITY )
	{
		if ( score_struct->m_unk21 >= m_l_count )
		{
			score_struct->m_flags &= ~1u;
			score_struct->m_flags &= ~8u;
			score_struct->m_flags &= ~2u;
			score_struct->m_flags |= 0x200u;
			score_struct->m_flags |= 4u;
			score_struct->m_play_mode = SSPLAY_PAUSE;
			if ( (score_struct->m_flags & 0x400) != 0 )
				score_struct->m_unk08 = score_struct->m_unk0C;
			else
				score_struct->m_unk08 = score_struct->m_unk04;
			if ( score_struct->m_next_sep != -1 )
			{
				score_struct->m_play_mode = SSPLAY_PAUSE;
				_SsSndNextSep(score_struct->m_next_sep, score_struct->m_next_seq);
				_SsVmSeqKeyOff(sep_no | (seq_no << 8));
			}
			_SsVmSeqKeyOff(sep_no | (seq_no << 8));
			score_struct->m_delta_value = score_struct->m_unk54;
		}
		else
		{
			score_struct->m_unk88 = 0;
			score_struct->m_unk1C = 0;
			score_struct->m_delta_value = 0;
			if ( (score_struct->m_flags & 0x400) != 0 )
				m_unk04 = score_struct->m_unk0C;
			else
				m_unk04 = score_struct->m_unk04;
			score_struct->m_seq_ptr = m_unk04;
			score_struct->m_unk08 = m_unk04;
		}
	}
	else
	{
		score_struct->m_unk88 = 0;
		score_struct->m_unk1C = 0;
		score_struct->m_delta_value = 0;
		if ( (score_struct->m_flags & 0x400) != 0 )
			score_struct->m_seq_ptr = score_struct->m_unk0C;
		else
			score_struct->m_seq_ptr = score_struct->m_unk04;
	}
}

int _SsGetSeqData(s16 sep_no, s16 seq_no)
{
	libsnd2_sequence_struct_t *score_struct;
	u8 midi_byte;
	u8 midi_byte_1;
	u8 midi_byte_2;

	score_struct = &_ss_score[sep_no][seq_no];
	midi_byte = *(score_struct->m_seq_ptr);
	score_struct->m_seq_ptr += 1;
	if ( (score_struct->m_flags & 0x401) == 0x401 )
	{
		if ( score_struct->m_seq_ptr == score_struct->m_unk10 + 1 )
		{
			_SsSeqGetEof(sep_no, seq_no);
			return -1;
		}
	}
	if ( (midi_byte & 0x80) != 0 )
	{
		score_struct->m_channel_idx = midi_byte & 0xF;
		switch ( midi_byte & 0xF0 )
		{
			case 0x90:
			{
				u8 midi_byte_next_1;

				score_struct->m_running_status = 0x90;
				midi_byte_next_1 = *(score_struct->m_seq_ptr);
				score_struct->m_seq_ptr += 1;
				midi_byte_1 = *(score_struct->m_seq_ptr);
				score_struct->m_seq_ptr += 1;
				score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
				SsFCALL.noteon(sep_no, seq_no, midi_byte_next_1, midi_byte_1);
				break;
			}
			case 0xB0:
				score_struct->m_running_status = 0xB0;
				score_struct->m_seq_ptr += 1;
				SsFCALL.control[CC_NUMBER](sep_no, seq_no, midi_byte);
				break;
			case 0xC0:
				score_struct->m_running_status = 0xC0;
				score_struct->m_seq_ptr += 1;
				SsFCALL.programchange(sep_no, seq_no, midi_byte);
				break;
			case 0xE0:
				score_struct->m_running_status = 0xE0;
				score_struct->m_seq_ptr += 1;
				SsFCALL.pitchbend(sep_no, seq_no);
				break;
			case 0xF0:
				score_struct->m_running_status = -1;
				midi_byte_2 = *(score_struct->m_seq_ptr);
				score_struct->m_seq_ptr += 1;
				if ( midi_byte_2 == 0x2F )
				{
					_SsSeqGetEof(sep_no, seq_no);
					return 1;
				}
				SsFCALL.metaevent(sep_no, seq_no, midi_byte);
				break;
			default:
				break;
		}
	}
	else
	{
		switch ( score_struct->m_running_status )
		{
			case 0x90:
			{
				u8 midi_byte_3;

				midi_byte_3 = *(score_struct->m_seq_ptr);
				score_struct->m_seq_ptr += 1;
				score_struct->m_delta_value = _SsReadDeltaValue(sep_no, seq_no);
				SsFCALL.noteon(sep_no, seq_no, midi_byte, midi_byte_3);
				break;
			}
			case 0xB0:
				SsFCALL.control[CC_NUMBER](sep_no, seq_no, midi_byte);
				break;
			case 0xC0:
				SsFCALL.programchange(sep_no, seq_no, midi_byte);
				break;
			case 0xE0:
				SsFCALL.pitchbend(sep_no, seq_no);
				break;
			case 0xFF:
				if ( midi_byte == 0x2F )
				{
					_SsSeqGetEof(sep_no, seq_no);
					return 1;
				}
				SsFCALL.metaevent(sep_no, seq_no, midi_byte);
				break;
			default:
				break;
		}
	}
	return 0;
}
