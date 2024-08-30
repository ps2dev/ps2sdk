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

void _SpuDataCallback(int (*callback)(void *userdata))
{
	char dummyarg[8];

	ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
	RegisterIntrHandler(IOP_IRQ_DMA_SPU2, 1, callback, dummyarg);
}

void _SpuAutoDMACallback(int (*callback)(void *userdata))
{
	char dummyarg[8];

	ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
	RegisterIntrHandler(IOP_IRQ_DMA_SPU, 1, callback, dummyarg);
}
