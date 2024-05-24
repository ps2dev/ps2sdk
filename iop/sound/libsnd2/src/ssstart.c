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

libsnd2_seq_tick_env_t _snd_seq_tick_env = {60, 1, &SsSeqCalledTbyT, NULL, 0u, 0u, 127u, 0u};
int _snd_seq_interval = 1;

#ifdef LIB_1300
void _SsTrapIntrVSync(void)
{
	if ( _snd_seq_tick_env.m_vsync_callback )
		_snd_seq_tick_env.m_vsync_callback();
	_snd_seq_tick_env.m_tick_callback();
}
#endif

#ifdef LIB_1300
void _SsSeqCalledTbyT_1per2(void)
{
	static int n_4 = 0;

	if ( n_4 )
	{
		n_4 = 0;
		_snd_seq_tick_env.m_tick_callback();
	}
	else
	{
		n_4 = 1;
	}
}
#endif

int _SsTrapIntrProcIOP(void *userdata)
{
	_snd_seq_tick_env.m_tick_callback();
	return *(u32 *)userdata;
}

static void _SsStart(int start_param)
{
	int wait_tmp;
#ifndef LIB_1300
	u32 rcount_target;
	iop_sys_clock_t iop_clock;
#endif

#ifndef LIB_1300
	rcount_target = 0x1046;
#endif
	for ( wait_tmp = 0; wait_tmp < 999; wait_tmp += 1 )
	{
		__asm__ __volatile__("" : "+g"(wait_tmp) : :);
	}
	_snd_seq_tick_env.m_alarm_tick = 6;
	_snd_seq_tick_env.m_vsync_tick = 0;
	_snd_seq_tick_env.m_unk11 = 0;
	_snd_seq_tick_env.m_vsync_callback = 0;
	switch ( _snd_seq_tick_env.m_tick_mode )
	{
		case SS_NOTICK0:
			_snd_seq_tick_env.m_alarm_tick = 127;
			return;
		case SS_TICK240:
			break;
		case SS_TICK120:
#ifndef LIB_1300
			rcount_target = 0x208c;
#endif
			break;
		case SS_TICKVSYNC:
			_snd_seq_tick_env.m_alarm_tick = 0;
			if ( !start_param )
				_snd_seq_tick_env.m_vsync_tick = 1;
#ifndef LIB_1300
			else
				rcount_target = 1;
#endif
			break;
		case SS_TICK60:
		case SS_TICK50:
		default:
			if ( _snd_seq_tick_env.m_manual_tick )
				return;
#ifdef LIB_1300
			if ( _snd_seq_tick_env.m_tick_mode < 70 )
			{
				_snd_seq_tick_env.m_unk11 += 1;
				return;
			}
#endif
#ifndef LIB_1300
			if ( _snd_seq_tick_env.m_tick_mode == 0 )
				__builtin_trap();
			rcount_target = -1000000;
#endif
			break;
	}
#ifndef LIB_1300
	CpuDisableIntr();
	if ( _snd_seq_tick_env.m_vsync_tick )
	{
		RegisterVblankHandler(0, 64, _SsTrapIntrProcIOP, &_snd_seq_interval);
		EnableIntr(0);
	}
	else if ( _snd_seq_tick_env.m_alarm_tick )
	{
		USec2SysClock(rcount_target, &iop_clock);
		_snd_seq_interval = iop_clock.lo;
		SetAlarm(&iop_clock, (unsigned int (*)(void *))_SsTrapIntrProcIOP, &_snd_seq_interval);
	}
	else
	{
		EnableIntr(0);
		RegisterVblankHandler(0, 64, _SsTrapIntrProcIOP, &_snd_seq_interval);
	}
	CpuEnableIntr();
#endif
}

void SsStart(void)
{
	_SsStart(1);
}

void SsStart2(void)
{
	_SsStart(0);
}
