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

unsigned int _snd_vmask = 0u;
static s16 volume_dat_2[16] = {16383, 16383, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

libsnd2_sequence_marker_t _SsMarkCallback[32];
int VBLANK_MINUS;
unsigned int _snd_openflag;
int _snd_ev_flag;
_SsFCALL SsFCALL;
libsnd2_sequence_struct_t *_ss_score[32];
s16 _snd_seq_s_max;
s16 _snd_seq_t_max;

void _SsInit(void)
{
	s16 *reg_set_ptr;
	int i;
	int j;

	reg_set_ptr = (s16 *)0xBF900760;
	for ( i = 0; i < 16; i += 1 )
	{
		reg_set_ptr[i] = volume_dat_2[i];
	}
	_SsVmInit(24);
	for ( j = 0; j < 32; j += 1 )
	{
		for ( i = 0; i < 16; i += 1 )
		{
			_SsMarkCallback[j].m_entries[i] = 0;
		}
	}
	VBLANK_MINUS = 60;
	_snd_openflag = 0;
	_snd_ev_flag = 0;
}
