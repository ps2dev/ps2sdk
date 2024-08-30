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

unsigned int SpuWrite0(unsigned int size)
{
	u32 old_tmode;
	int ck_1;
	int ck_2;
	u32 tsa;
	unsigned int written;
	void (*bk)(void);

	old_tmode = _spu_transMode;
	bk = 0;
	ck_1 = 0;
	if ( _spu_transMode == 1 )
	{
		_spu_transMode = 0;
		ck_1 = 1;
	}
	ck_2 = 1;
	tsa = _spu_tsa[1];
	written = 0;
	if ( _spu_transferCallback )
	{
		bk = _spu_transferCallback;
		_spu_transferCallback = 0;
		written = 0;
	}
	while ( ck_2 )
	{
		unsigned int bsize_1;
		int bsize_2;

		bsize_1 = size >> 6;
		if ( size < 0x401 )
		{
			bsize_2 = bsize_1 << 6;
			ck_2 = 0;
			if ( (unsigned int)bsize_2 < size )
				bsize_2 += 64;
		}
		else
		{
			bsize_2 = 1024;
		}
		_spu_t(2, tsa);
		_spu_t(1);
		FlushDcache();
		_spu_t(3, _spu_zerobuf, bsize_2);
		while ( !gDMADeliverEvent )
			;
		gDMADeliverEvent = 0;
		size -= 1024;
		tsa += 512;
		written += bsize_2;
	}
	if ( ck_1 )
		_spu_transMode = old_tmode;
	if ( bk )
	{
		_spu_transferCallback = bk;
	}
	return written;
}
