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

void SsSetMute(char mode)
{
	int mute_val;

	switch ( mode )
	{
		case SS_MUTE_OFF:
			mute_val = SPU_OFF;
			break;
		case SS_MUTE_ON:
			mute_val = SPU_ON;
			break;
		default:
			return;
	}
	SpuSetMute(mute_val);
}
