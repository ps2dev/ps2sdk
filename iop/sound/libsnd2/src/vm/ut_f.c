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

void SsUtFlush(void)
{
	if ( _snd_ev_flag != 1 )
	{
		_snd_ev_flag = 1;
		_SsVmFlush();
		_snd_ev_flag = 0;
	}
}
