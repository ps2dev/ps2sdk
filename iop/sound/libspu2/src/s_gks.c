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

int SpuGetKeyStatus(unsigned int voice_bit)
{
	int v1;
	int v2;

	v1 = -1;
	for ( v2 = 0; v2 < 24; v2 += 1 )
	{
		if ( (voice_bit & (1 << v2)) != 0 )
		{
			v1 = v2;
			break;
		}
	}
	return ( v1 == -1 ) ? -1 : (( (_spu_keystat[_spu_core] & (1 << v1)) != 0 ) ? (( _spu_RXX[512 * _spu_core + 5 + 8 * v1] != 0 ) ? SPU_ON : SPU_ON_ENV_OFF) : (( _spu_RXX[512 * _spu_core + 5 + 8 * v1] != 0 ) ? SPU_OFF_ENV_ON : SPU_OFF));
}
