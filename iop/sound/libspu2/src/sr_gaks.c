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

int SpuRGetAllKeysStatus(int min_, int max_, char *status)
{
	int voice;

	if ( min_ < 0 )
	{
		min_ = 0;
	}
	if ( min_ < 0 || min_ >= 24 )
		return SPU_INVALID_ARGS;
	if ( max_ >= 24 )
		max_ = 23;
	if ( max_ < 0 )
		return SPU_INVALID_ARGS;
	if ( max_ < min_ )
		return SPU_INVALID_ARGS;
	for ( voice = min_; voice < (max_ + 1); voice += 1 )
	{
		const vu16 *v9;

		v9 = &_spu_RXX[512 * _spu_core + 8 * voice];
		if ( (_spu_keystat[_spu_core] & (1 << voice)) != 0 )
		{
			if ( v9[5] )
			{
				status[voice] = SPU_ON;
			}
			else
			{
				status[voice] = SPU_ON_ENV_OFF;
			}
		}
		else
		{
			if ( v9[5] )
			{
				status[voice] = SPU_OFF_ENV_ON;
			}
			else
			{
				status[voice] = SPU_OFF;
			}
		}
	}
	return SPU_SUCCESS;
}

void SpuGetAllKeysStatus(char *status)
{
	SpuRGetAllKeysStatus(0, 23, status);
}
