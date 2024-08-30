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

u32 _spu_keystat[2] = {0u, 0u};
u32 _spu_trans_mode = SPU_TRANSFER_BY_DMA;
u32 _spu_rev_flag = 0u;
u32 _spu_rev_reserve_wa = 0u;
u32 _spu_rev_offsetaddr = 0u;
SpuReverbAttr _spu_rev_attr = {0u, 0, {0, 0}, 0, 0};
u32 _spu_RQvoice = 0u;
u32 _spu_RQmask = 0u;
s16 _spu_voice_centerNote[2][24] = {
	{
		0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
		0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
	},
	{
		0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
		0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000, 0xc000,
	}};
u32 _spu_env = 0u;
u32 _spu_isCalled = 0u;
SpuIRQCallbackProc _spu_irq_callback = NULL;

u16 gDMADeliverEvent;

void _SpuInit(int mode)
{
	_spu_init(mode);
	if ( !mode )
	{
		int v2;

		for ( v2 = 0; v2 < 2; v2 += 1 )
		{
			int v4;

			for ( v4 = 0; v4 < 24; v4 += 1 )
			{
				_spu_voice_centerNote[v2][v4] = -16384;
			}
		}
	}
	SpuStart();
	if ( !mode )
	{
		_spu_rev_flag = 0;
		_spu_rev_reserve_wa = 0;
		_spu_rev_attr.mode = 0;
		_spu_rev_attr.depth.left = 0;
		_spu_rev_attr.depth.right = 0;
		_spu_rev_attr.delay = 0;
		_spu_rev_attr.feedback = 0;
		_spu_rev_offsetaddr = SpuGetReverbEndAddr() - (8 * _spu_rev_workareasize[0] - 2);
		_spu_FsetRXX(368, _spu_rev_offsetaddr, 1);
	}
	_spu_keystat[0] = 0;
	_spu_keystat[1] = 0;
	_spu_AllocBlockNum = 0;
	_spu_AllocLastNum = 0;
	_spu_memList = 0;
	_spu_trans_mode = SPU_TRANSFER_BY_DMA;
	_spu_transMode = 0;
	_spu_RQmask = 0;
	_spu_RQvoice = 0;
	_spu_env = 0;
}

int _SpuDefaultCallback(void *userdata)
{
	(void)userdata;

	_spu_irq_callback();
	return 0;
}

void SpuStart(void)
{
	int v0;

	v0 = 0;
	_spu_isCalled = 1;
	CpuDisableIntr();
	_SpuDataCallback(_spu_FiDMA);
	_SpuAutoDMACallback(_spu_FiAutoDMA);
	gDMADeliverEvent = 0;
	CpuEnableIntr();
	EnableIntr(IOP_IRQ_DMA_SPU);
	EnableIntr(IOP_IRQ_DMA_SPU2);
	ReleaseIntrHandler(IOP_IRQ_SPU);
	RegisterIntrHandler(IOP_IRQ_SPU, 1, _SpuDefaultCallback, &v0);
}
