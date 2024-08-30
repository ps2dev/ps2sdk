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

s16 SsSepOpenJ(unsigned int *addr, s16 vab_id, s16 seq_cnt)
{
	s16 v4;
	unsigned int v6;
	s16 v8;

	v4 = 0;
	if ( _snd_openflag == (u32)-1 )
	{
		printf("Can't Open Sequence data any more\n\n");
		return -1;
	}
	for ( v6 = 0; v6 < 32; v6 += 1 )
	{
		if ( (_snd_openflag & ((u32)1 << v6)) == 0 )
		{
			v4 = v6;
			break;
		}
	}
	_snd_openflag |= 1 << v4;
	for ( v8 = 0; v8 < seq_cnt; v8 += 1 )
	{
		int inited;

		inited = _SsInitSoundSep(v4, v8, vab_id, (u8 *)addr);
		addr = (unsigned int *)((char *)addr + inited);
		if ( inited == -1 )
			return -1;
	}
	return v4;
}
