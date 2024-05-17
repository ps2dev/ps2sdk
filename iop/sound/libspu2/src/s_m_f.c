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

void SpuFree(unsigned int addr)
{
	int block_counter;

	for ( block_counter = 0; block_counter < _spu_AllocBlockNum; block_counter += 1 )
	{
		if ( (_spu_memList[block_counter].addr_area & 0x40000000) != 0 )
		{
			break;
		}
		if ( _spu_memList[block_counter].addr_area == addr )
		{
			_spu_memList[block_counter].addr_area = addr | 0x80000000;
			break;
		}
	}
	_spu_gcSPU();
}
