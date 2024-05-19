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

void SpuQuit(void)
{
	if ( _spu_isCalled == 1 )
	{
#ifndef LIB_1300
		// Added in OSDSND 110U
		int v0[2];
#endif

		_spu_isCalled = 0;
		CpuDisableIntr();
		_spu_transferCallback = 0;
		_spu_IRQCallback = 0;
		_SpuDataCallback(0);
		_SpuAutoDMACallback(0);
#ifndef LIB_1300
		// Added in OSDSND 110U
		ReleaseIntrHandler(IOP_IRQ_DMA_SPU2);
		ReleaseIntrHandler(IOP_IRQ_DMA_SPU);
		ReleaseIntrHandler(IOP_IRQ_SPU);
		DisableIntr(IOP_IRQ_DMA_SPU2, v0);
		DisableIntr(IOP_IRQ_DMA_SPU, v0);
		DisableIntr(IOP_IRQ_SPU, v0);
#endif
		CpuEnableIntr();
	}
}
