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

int _SpuIsInAllocateArea(u32 addr)
{
	libspu2_malloc_t *p_a;
	unsigned int ptr;

	if ( !_spu_memList )
		return 0;
	for ( p_a = _spu_memList;; p_a += 1 )
	{
		if ( (p_a->addr_area & 0x80000000) != 0 )
			continue;
		ptr = p_a->addr_area & 0xFFFFFFF;
		if ( (p_a->addr_area & 0x40000000) != 0 )
			break;
		if ( ptr >= addr || addr < ptr + p_a->size_area )
			return 1;
	}
	return 0;
}

int _SpuIsInAllocateArea_(u32 addr)
{
	return _SpuIsInAllocateArea(addr);
}
