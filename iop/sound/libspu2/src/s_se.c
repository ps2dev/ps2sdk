/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libspu2_internal.h"

void SpuSetEnv(const SpuEnv *env)
{
	int mask;

	mask = env->mask;
	if ( mask == 0 )
	{
		mask = 0xFFFFFFFF;
	}
	if ( (mask & SPU_ENV_EVENT_QUEUEING) != 0 )
	{
		switch ( env->queueing )
		{
			case SPU_ON:
				_spu_env |= 1u;
				break;
			case SPU_OFF:
			default:
				_spu_env &= ~1u;
				break;
		}
	}
}
