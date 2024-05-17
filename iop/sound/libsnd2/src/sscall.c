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

void SsSeqCalledTbyT(void)
{
	if ( _snd_ev_flag != 1 )
	{
		int sep_no;

		_snd_ev_flag = 1;

		_SsVmFlush();
		for ( sep_no = 0; sep_no < _snd_seq_s_max; sep_no += 1 )
		{
			if ( (_snd_openflag & (1 << sep_no)) != 0 )
			{
				int seq_no;

				for ( seq_no = 0; seq_no < _snd_seq_t_max; seq_no += 1 )
				{
					libsnd2_sequence_struct_t *score_struct;

					score_struct = &_ss_score[sep_no][seq_no];
					if ( (score_struct->m_flags & 1) != 0 )
					{
						_SsSndPlay(sep_no, seq_no);
						if ( (score_struct->m_flags & 0x10) != 0 )
						{
							_SsSndCrescendo(sep_no, seq_no);
#ifdef LIB_1300
							printf("--- _SsSndCrescendo ---\n");
#endif
						}
						if ( (score_struct->m_flags & 0x20) != 0 )
						{
							_SsSndCrescendo(sep_no, seq_no);
#ifdef LIB_1300
							printf("--- _SsSndCrescendo(DE) ---\n");
#endif
						}
						if ( (score_struct->m_flags & 0x40) != 0 )
						{
							_SsSndTempo(sep_no, seq_no);
#ifdef LIB_1300
							printf("--- _SsSndTempo(ACE) ---\n");
#endif
						}
						if ( (score_struct->m_flags & 0x80) != 0 )
						{
							_SsSndTempo(sep_no, seq_no);
#ifdef LIB_1300
							printf("--- _SsSndTempo(RIT) ---\n");
#endif
						}
					}
					if ( (score_struct->m_flags & 2) != 0 )
					{
						_SsSndPause(sep_no, seq_no);
#ifdef LIB_1300
						printf("--- _SsSndPause ---\n");
#endif
					}
					if ( (score_struct->m_flags & 8) != 0 )
					{
						_SsSndReplay(sep_no, seq_no);
#ifdef LIB_1300
						printf("--- _SsSndReplay ---\n");
#endif
					}
					if ( (score_struct->m_flags & 4) != 0 )
					{
						_SsSndStop(sep_no, seq_no);
						score_struct->m_flags = 0;
#ifdef LIB_1300
						printf("--- _SsSndStop ---\n");
#endif
					}
				}
			}
		}
		_snd_ev_flag = 0;
	}
}
