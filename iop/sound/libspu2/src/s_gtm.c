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

int SpuGetTransferMode(void)
{
	_spu_trans_mode = ( _spu_transMode == SPU_TRANSFER_BY_IO ) ? SPU_TRANSFER_BY_IO : SPU_TRANSFER_BY_DMA;
	return _spu_trans_mode;
}
