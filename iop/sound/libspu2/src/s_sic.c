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

SpuIRQCallbackProc SpuSetIRQCallback(SpuIRQCallbackProc func)
{
	void (*v1)(void);

	v1 = _spu_IRQCallback;
	if ( func != _spu_IRQCallback )
	{
		_spu_IRQCallback = func;
		_SpuCallback(func);
	}
	return v1;
}
