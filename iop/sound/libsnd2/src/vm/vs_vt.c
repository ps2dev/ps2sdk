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

s16 SsVabTransfer(u8 *vh_addr, u8 *vb_addr, s16 vab_id, s16 i_flag)
{
	s16 v6;
	int v8;

	v6 = SsVabOpenHead(vh_addr, vab_id);
	if ( v6 < 0 )
		return -1;
	v8 = SsVabTransBody(vb_addr, v6);
	if ( v8 < 0 )
	{
		return -2;
	}
	if ( i_flag != SS_IMMEDIATE )
	{
		SsVabTransCompleted(SS_WAIT_COMPLETED);
	}
	return v8;
}
