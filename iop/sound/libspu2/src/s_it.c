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

void _spu_setInTransfer(int mode)
{
	_spu_inTransfer = mode != 1;
}

int _spu_getInTransfer(void)
{
	return _spu_inTransfer != 1;
}
