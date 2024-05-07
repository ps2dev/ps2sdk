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

int SpuClearReverbWorkArea(int mode)
{
	int ck_1;
	unsigned int m;
	unsigned int n;
	u32 t;
	int ck_2;
	void (*bk)(void);

	bk = 0;
	ck_1 = 0;
	if ( (unsigned int)mode >= SPU_REV_MODE_MAX )
		return -1;
	if ( mode )
	{
		m = 8 * _spu_rev_workareasize[mode];
		n = (SpuGetReverbEndAddr() - m) >> 1;
	}
	else
	{
		m = 32;
		n = 2097120;
	}
	printf("### addr = %u  size = %u\n", n, m);
	t = _spu_transMode;
	if ( _spu_transMode == 1 )
	{
		_spu_transMode = 0;
		ck_1 = 1;
	}
	ck_2 = 1;
	if ( _spu_transferCallback )
	{
		bk = _spu_transferCallback;
		_spu_transferCallback = 0;
	}
	while ( ck_2 )
	{
		int m_tmp;

		m_tmp = 1024;
		if ( m < 0x401 )
		{
			m_tmp = m;
			ck_2 = 0;
		}
		gDMADeliverEvent = 0;
		_spu_t(2, n);
		_spu_t(1);
		_spu_t(3, _spu_zerobuf, m_tmp);
		while ( !gDMADeliverEvent )
			;
		gDMADeliverEvent = 0;
		m -= 1024;
		n += 512;
	}
	if ( ck_1 )
		_spu_transMode = t;
	if ( bk )
	{
		_spu_transferCallback = bk;
	}
	return 0;
}
