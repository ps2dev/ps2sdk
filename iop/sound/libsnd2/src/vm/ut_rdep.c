/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

void SsUtSetReverbDepth(s16 ldepth, s16 rdepth)
{
	_svm_rattr.mask = SPU_REV_DEPTHL | SPU_REV_DEPTHR;
	_svm_rattr.depth.left = (s16)(0x7FFF * ldepth) / 127;
	_svm_rattr.depth.right = (s16)(0x7FFF * rdepth) / 127;
	SpuSetReverbModeParam(&_svm_rattr);
}
