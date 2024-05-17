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

void _spu_print(void)
{
	int v0;

	for ( v0 = 0; v0 <= _spu_AllocBlockNum; v0 += 1 )
	{
		printf(
			"[%d] %08lx / %08lx (%08ld)\n",
			v0,
			_spu_memList[v0].addr_area,
			_spu_memList[v0].size_area,
			_spu_memList[v0].size_area);
		if ( (_spu_memList[v0].addr_area & 0x40000000) != 0 )
		{
			break;
		}
	}
}
