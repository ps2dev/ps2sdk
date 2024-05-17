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

int SpuSetTransferMode(int mode)
{
	int result;

	if ( mode == SPU_TRANSFER_BY_DMA )
		result = SPU_TRANSFER_BY_DMA;
	else
		result = (mode == SPU_TRANSFER_BY_IO) ? SPU_TRANSFER_BY_IO : SPU_TRANSFER_BY_DMA;
	_spu_trans_mode = mode;
	_spu_transMode = result;
	return result;
}
