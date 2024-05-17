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

int SpuSetNoiseClock(int n_clock)
{
	int n_clock_fixed;

	n_clock_fixed = 0;
	if ( n_clock >= 0 )
	{
		n_clock_fixed = n_clock;
		if ( n_clock >= 64 )
			n_clock_fixed = 63;
	}
	_spu_RXX[512 * _spu_core + 205] = (_spu_RXX[512 * _spu_core + 205] & ~0x3F00) | ((n_clock_fixed & 0x3F) << 8);
	return n_clock_fixed;
}
