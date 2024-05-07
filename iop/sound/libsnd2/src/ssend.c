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

void SsEnd(void)
{
#ifndef LIB_1300
	if ( !_snd_seq_tick_env.m_manual_tick )
	{
		_snd_seq_tick_env.m_unk11 = 0;
		if ( _snd_seq_tick_env.m_alarm_tick != 127 )
		{
			CpuDisableIntr();
			if ( _snd_seq_tick_env.m_vsync_tick )
			{
				ReleaseVblankHandler(0, _SsTrapIntrProcIOP);
				_snd_seq_tick_env.m_vsync_tick = 0;
			}
			else if ( _snd_seq_tick_env.m_alarm_tick )
			{
				CancelAlarm((unsigned int (*)(void *))_SsTrapIntrProcIOP, &_snd_seq_interval);
			}
			else
			{
				ReleaseVblankHandler(0, _SsTrapIntrProcIOP);
				_snd_seq_tick_env.m_vsync_callback = 0;
			}
			CpuEnableIntr();
			_snd_seq_tick_env.m_alarm_tick = 127;
		}
	}
#endif
}
