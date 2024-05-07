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

static void __attribute__((optimize("no-unroll-loops"))) _spu_wait_SpuSetVoiceDR(void)
{
	int i;
	int v1;

	v1 = 1;
	for ( i = 0; i < 2; i += 1 )
	{
		v1 *= 13;
		__asm__ __volatile__("" : "+g"(v1) : :);
	}
}

void SpuSetVoiceDR(int v_num, u16 dr)
{
	vu16 *v2;

	v2 = &_spu_RXX[512 * _spu_core + 8 * v_num];
	v2[3] = (v2[3] & ~0xf0) | (16 * dr);
	_spu_wait_SpuSetVoiceDR();
}
