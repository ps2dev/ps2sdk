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

unsigned int SpuFlush(unsigned int ev)
{
	// Unofficial: Fixed _spu_RQ and _spu_RQmask access offsets
	unsigned int retval;
	unsigned int ev_tmp;

	retval = 0;
	ev_tmp = ev;
	if ( ev_tmp == 0 )
	{
		ev_tmp = 0xFFFFFFFF;
	}
	if ( ((ev_tmp & SPU_EVENT_PITCHLFO) != 0) && (_spu_RQmask & 2) != 0 )
	{
		vu16 *regstmp1;

		_spu_RQmask &= ~2u;
		regstmp1 = &_spu_RXX[512 * _spu_core];
		regstmp1[192] = _spu_RQ[4];
		retval |= SPU_EVENT_PITCHLFO;
		regstmp1[193] = _spu_RQ[5];
	}
	if ( ((ev_tmp & SPU_EVENT_NOISE) != 0) && (_spu_RQmask & 4) != 0 )
	{
		vu16 *regstmp2;

		_spu_RQmask &= ~4u;
		regstmp2 = &_spu_RXX[512 * _spu_core];
		regstmp2[194] = _spu_RQ[6];
		retval |= SPU_EVENT_NOISE;
		regstmp2[195] = _spu_RQ[7];
	}
	if ( ((ev_tmp & SPU_EVENT_REVERB) != 0) && (_spu_RQmask & 8) != 0 )
	{
		vu16 *regstmp3;
		_spu_RQmask &= ~8u;
		regstmp3 = &_spu_RXX[512 * _spu_core];
		regstmp3[198] = _spu_RQ[10];
		regstmp3[202] = _spu_RQ[14];
		regstmp3[199] = _spu_RQ[11];
		retval |= SPU_EVENT_REVERB;
		regstmp3[203] = _spu_RQ[15];
	}
	if ( ((ev_tmp & SPU_EVENT_KEY) != 0) && (_spu_RQmask & 1) != 0 )
	{
		vu16 *regstmp4;
		int i1;

		_spu_RQmask &= ~1u;
		regstmp4 = &_spu_RXX[512 * _spu_core];
		regstmp4[208] = _spu_RQ[0];
		regstmp4[209] = _spu_RQ[1];
		regstmp4[210] = _spu_RQ[2];
		regstmp4[211] = _spu_RQ[3];
		for ( i1 = 0; i1 < 4; i1 += 1 )
		{
			_spu_RQ[i1] = 0;
		}
		retval |= SPU_EVENT_KEY;
		if ( (_spu_env & 1) != 0 )
			_spu_keystat[_spu_core] = _spu_RQvoice;
	}
	return retval;
}
