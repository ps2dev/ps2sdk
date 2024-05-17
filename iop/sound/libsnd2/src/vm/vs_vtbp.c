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

static s16 vabid_old_2 = -1;

s16 SsVabTransBodyPartly(u8 *addr, unsigned int bufsize, s16 vab_id)
{
	unsigned int bufsize_tmp;
	int new_vabid;

	if ( (u16)vab_id >= 0x11u || _svm_vab_used[vab_id] != 2 )
	{
		_spu_setInTransfer(0);
		return -1;
	}
	if ( !_svm_vab_not_send_size )
	{
		vabid_old_2 = vab_id;
		_svm_vab_not_send_size = _svm_vab_total[vab_id];
		SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
		if ( !SpuSetTransferStartAddr(_svm_vab_start[vab_id]) )
		{
			_svm_vab_not_send_size = 0;
			vabid_old_2 = -1;
			_spu_setInTransfer(0);
			return -1;
		}
	}
	new_vabid = vabid_old_2;
	if ( vabid_old_2 != vab_id )
	{
		_spu_setInTransfer(0);
		return -1;
	}
	bufsize_tmp = bufsize;
	if ( (unsigned int)_svm_vab_not_send_size < bufsize )
		bufsize_tmp = _svm_vab_not_send_size;
	_spu_setInTransfer(1);
	SpuWritePartly(addr, bufsize_tmp);
	_svm_vab_not_send_size -= bufsize_tmp;
	if ( _svm_vab_not_send_size != 0 )
	{
		return -2;
	}
	vabid_old_2 = -1;
	_svm_vab_not_send_size = 0;
	_svm_vab_used[new_vabid] = 1;
	return new_vabid;
}
