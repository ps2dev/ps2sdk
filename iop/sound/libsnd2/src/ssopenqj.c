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

s16 SsSeqOpenJ(unsigned int *addr, s16 vab_id)
{
	s16 v2;
	unsigned int v4;

	v2 = 0;
	if ( _snd_openflag == (u32)-1 )
	{
		printf("Can't Open Sequence data any more\n\n");
		return -1;
	}
	for ( v4 = 0; v4 < 32; v4 += 1 )
	{
		if ( (_snd_openflag & ((u32)1 << v4)) == 0 )
		{
			v2 = v4;
			break;
		}
	}
	_snd_openflag |= 1 << v2;
	if ( _SsInitSoundSeq(v2, vab_id, (u8 *)addr) == -1 )
		return -1;
	return v2;
}
