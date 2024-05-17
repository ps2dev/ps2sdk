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

int SsSeqSkip(s16 sep_no, s16 seq_no, char unit, s16 count)
{
	int m_unk54;
	int v5;
	libsnd2_sequence_struct_t *score_struct;
	void (*noteon)(s16, s16, u8, u8);
	u8 *m_unk04;
	int m_unk84;
	u8 *v19;

	m_unk54 = 0;
	v5 = 0;
	score_struct = &_ss_score[sep_no][seq_no];
	if ( count < 0 )
		return -1;
	if ( !count )
		return 0;
	noteon = SsFCALL.noteon;
	m_unk04 = score_struct->m_unk04;
	_snd_ev_flag = 1;
	score_struct->m_seq_ptr = m_unk04;
	SsFCALL.noteon = dmy_nothing1;
	switch ( unit )
	{
		case SSSKIP_TICK:
			m_unk54 = score_struct->m_unk54 * count;
			break;
		case SSSKIP_NOTE4:
			m_unk54 = count * 10 * score_struct->m_resolution_of_quarter_note;
			break;
		case SSSKIP_NOTE8:
			m_unk54 = count * 5 * score_struct->m_resolution_of_quarter_note;
			break;
		case SSSKIP_BAR:
		{
			int m_rhythm_d;

			m_rhythm_d = score_struct->m_rhythm_d;
			if ( m_rhythm_d == 2 )
			{
				m_unk54 = score_struct->m_rhythm_n * count * 10 * score_struct->m_resolution_of_quarter_note;
			}
			else
			{
				int v15;

				v15 = count * 10 * score_struct->m_resolution_of_quarter_note;
				if ( (1 << m_rhythm_d) == 0 )
					__builtin_trap();
				if ( 1 << m_rhythm_d == -1 && (unsigned int)v15 == 0x80000000 )
					__builtin_trap();
				m_unk54 = v15 / (1 << m_rhythm_d) * score_struct->m_rhythm_n;
			}
			break;
		}
		default:
			break;
	}
	do
	{
		do
		{
			if ( _SsGetSeqData(sep_no, seq_no) == 1 )
			{
				m_unk54 = score_struct->m_unk54;
				m_unk84 = score_struct->m_unk84;
				v19 = score_struct->m_unk04;
				v5 = -1;
				score_struct->m_unk10 = 0;
				score_struct->m_delta_value = m_unk84;
				score_struct->m_seq_ptr = v19;
				score_struct->m_unk0C = v19;
				break;
			}
		} while ( !score_struct->m_delta_value );
		m_unk54 -= score_struct->m_delta_value;
	} while ( score_struct->m_unk54 < m_unk54 );
	SsFCALL.noteon = noteon;
	_snd_ev_flag = 0;
	return v5;
}
