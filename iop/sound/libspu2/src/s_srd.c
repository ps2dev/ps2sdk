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

int SpuSetReverbDepth(SpuReverbAttr *attr)
{
	unsigned int mask;

	mask = attr->mask;
	if ( mask == 0 )
		mask = 0xFFFFFFFF;
	if ( (mask & SPU_REV_DEPTHL) != 0 )
	{
		_spu_RXX[20 * _spu_core + 946] = attr->depth.left;
		_spu_rev_attr.depth.left = attr->depth.left;
	}
	if ( (mask & SPU_REV_DEPTHR) != 0 )
	{
		_spu_RXX[20 * _spu_core + 947] = attr->depth.right;
		_spu_rev_attr.depth.right = attr->depth.right;
	}
	return 0;
}
