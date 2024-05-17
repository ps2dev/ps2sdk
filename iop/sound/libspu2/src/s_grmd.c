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

void SpuGetReverbModeDepth(s16 *depth_left, s16 *depth_right)
{
	*depth_left = _spu_rev_attr.depth.left;
	*depth_right = _spu_rev_attr.depth.right;
}
