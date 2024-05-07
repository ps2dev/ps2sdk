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

int SpuIsTransferCompleted(int flag)
{
	int result;

	if ( _spu_trans_mode == SPU_TRANSFER_BY_IO || _spu_inTransfer == 1 )
		return 1;
	result = (s16)gDMADeliverEvent;
	if ( flag == SPU_TRANSFER_WAIT )
	{
		while ( !gDMADeliverEvent )
			;
		gDMADeliverEvent = 0;
		_spu_inTransfer = 1;
		return 1;
	}
	else if ( gDMADeliverEvent == 1 )
	{
		_spu_inTransfer = (s16)gDMADeliverEvent;
#ifndef LIB_1300
		// Added in OSDSND 110U
		gDMADeliverEvent = 0;
#endif
	}
	return result;
}
