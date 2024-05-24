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

s16 SsVabFakeBody(s16 vab_id)
{
	if ( (u16)vab_id >= 0x11u )
		return -1;
	if ( _svm_vab_used[vab_id] != 2 )
		return -1;
	_spu_setInTransfer(0);
	_svm_vab_used[vab_id] = 1;
	return vab_id;
}
