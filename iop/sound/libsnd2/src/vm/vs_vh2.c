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

static int VsGetAddrSpuMalloc(unsigned int size_in_bytes, int mode, s16 vab_id);

s16 SsVabOpenHead(u8 *addr, s16 vab_id)
{
	return _SsVabOpenHeadWithMode(addr, vab_id, VsGetAddrSpuMalloc, 0);
}

static int VsGetAddrSpuMalloc(unsigned int size_in_bytes, int mode, s16 vab_id)
{
	int result;

	(void)mode;

	result = SpuMalloc(size_in_bytes);
	if ( result == -1 )
	{
		_svm_vab_used[vab_id] = 0;
		_spu_setInTransfer(0);
		_svm_vab_count -= 1;
	}
	return result;
}
