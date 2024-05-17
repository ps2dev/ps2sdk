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

void SpuGetVoiceEnvelopeAttr(int v_num, int *key_stat, s16 *envx)
{
	int v3;

	v3 = _spu_RXX[512 * _spu_core + 5 + 8 * v_num];
	*envx = v3;
	if ( (_spu_keystat[_spu_core] & (1 << v_num)) != 0 )
	{
		if ( v3 )
		{
			*key_stat = SPU_ON;
		}
		else
		{
			*key_stat = SPU_ON_ENV_OFF;
		}
	}
	else
	{
		if ( v3 )
		{
			*key_stat = SPU_OFF_ENV_ON;
		}
		else
		{
			*key_stat = SPU_OFF;
		}
	}
}
