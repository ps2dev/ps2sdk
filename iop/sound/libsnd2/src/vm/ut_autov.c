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

s16 SsUtAutoVol(s16 vc, s16 start_vol, s16 end_vol, s16 delta_time)
{
	_autovol = (libsnd2_auto_vol_pan_callback)SetAutoVol;
	if ( (u16)vc >= 0x18u )
		return -1;
	SeAutoVol(vc, start_vol, end_vol, delta_time);
	return 0;
}
