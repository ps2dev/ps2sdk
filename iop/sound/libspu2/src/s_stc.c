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

SpuTransferCallbackProc SpuSetTransferCallback(SpuTransferCallbackProc func)
{
	SpuTransferCallbackProc result;

	result = _spu_transferCallback;
	if ( func != _spu_transferCallback )
		_spu_transferCallback = func;
	return result;
}

SpuTransferCallbackProc SpuAutoDMASetCallback(SpuTransferCallbackProc func)
{
	SpuTransferCallbackProc result;

	result = _spu_AutoDMACallback;
	if ( func != _spu_AutoDMACallback )
		_spu_AutoDMACallback = func;
	return result;
}
