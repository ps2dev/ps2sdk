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

void SsUtSetReverbDelay(s16 delay)
{
	_svm_rattr.delay = delay;
	_svm_rattr.mask = SPU_REV_DELAYTIME;
	SpuSetReverbModeParam(&_svm_rattr);
}
