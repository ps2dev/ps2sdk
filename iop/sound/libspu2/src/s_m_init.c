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

int SpuInitMalloc(int num, char *top)
{
	libspu2_malloc_t *top_tmp;

	if ( num <= 0 )
		return 0;
	top_tmp = (libspu2_malloc_t *)top;
	_spu_memList = top_tmp;
	_spu_AllocLastNum = 0;
	top_tmp->addr_area = 0x40005010;
	_spu_AllocBlockNum = num;
	top_tmp->size_area = 0x1FAFF0;
	return num;
}
