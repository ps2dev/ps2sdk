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

void SsSetTickMode(int tick_mode)
{
	if ( (tick_mode & SS_NOTICK) != 0 )
	{
		_snd_seq_tick_env.m_manual_tick = 1;
		_snd_seq_tick_env.m_tick_mode = tick_mode & 0xFFF;
	}
	else
	{
		_snd_seq_tick_env.m_manual_tick = 0;
		_snd_seq_tick_env.m_tick_mode = tick_mode;
	}
	switch ( _snd_seq_tick_env.m_tick_mode )
	{
		case SS_NOTICK0:
		case SS_TICKVSYNC:
			VBLANK_MINUS = 60;
			break;
		case SS_TICK60:
			VBLANK_MINUS = 60;
			_snd_seq_tick_env.m_tick_mode = 5;
			break;
		case SS_TICK240:
			VBLANK_MINUS = 240;
			break;
		case SS_TICK120:
			VBLANK_MINUS = 120;
			break;
		case SS_TICK50:
			VBLANK_MINUS = 50;
			_snd_seq_tick_env.m_tick_mode = 50;
			break;
		default:
			VBLANK_MINUS = _snd_seq_tick_env.m_tick_mode;
			break;
	}
}
