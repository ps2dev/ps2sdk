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

void SpuGetReverbModeParam(SpuReverbAttr *attr)
{
	attr->mode = _spu_rev_attr.mode;
	attr->delay = _spu_rev_attr.delay;
	attr->feedback = _spu_rev_attr.feedback;
	attr->depth = _spu_rev_attr.depth;
}
