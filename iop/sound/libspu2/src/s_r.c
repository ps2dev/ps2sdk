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

unsigned int SpuRead(u8 *addr, unsigned int size)
{
	unsigned int size_tmp;

	size_tmp = ( size > 0x1FAFF0 ) ? 0x1FAFF0 : size;
	_spu_Fr(addr, size_tmp);
	if ( !_spu_transferCallback )
		_spu_inTransfer = 0;
	return size_tmp;
}
