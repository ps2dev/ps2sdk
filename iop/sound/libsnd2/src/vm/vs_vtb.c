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

s16 SsVabTransBody(u8 *addr, s16 vab_id)
{
	if ( (u16)vab_id >= 0x11u || _svm_vab_used[vab_id] != 2 )
	{
		_spu_setInTransfer(0);
		return -1;
	}
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	if ( !SpuSetTransferStartAddr(_svm_vab_start[vab_id]) )
	{
		_spu_setInTransfer(0);
		return -1;
	}
	SpuWrite(addr, _svm_vab_total[vab_id]);
	_svm_vab_used[vab_id] = 1;
	return vab_id;
}
