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

int SpuSetReverbModeType(int mode)
{
	unsigned int v1;
	int v2;
	vu16 *v4;
	int v5;
	vu16 *v6;
	libspu2_reverb_param_entry_t v7;

	v1 = mode;
	v2 = 0;
	if ( (mode & SPU_REV_MODE_CLEAR_WA) != 0 )
	{
		v1 = mode & ~SPU_REV_MODE_CLEAR_WA;
		v2 = 1;
	}
	if ( v1 >= SPU_REV_MODE_MAX )
	{
		return -1;
	}
	_spu_rev_attr.mode = v1;
	_spu_rev_offsetaddr = SpuGetReverbEndAddr() - (8 * _spu_rev_workareasize[v1] - 2);
	memcpy(&v7, &_spu_rev_param[v1], 0x44u);
	v7.flags = 0;
	switch ( v1 )
	{
		case SPU_REV_MODE_ECHO:
			_spu_rev_attr.feedback = 127;
			_spu_rev_attr.delay = 127;
			break;
		case SPU_REV_MODE_DELAY:
			_spu_rev_attr.feedback = 0;
			_spu_rev_attr.delay = 127;
			break;
		default:
			_spu_rev_attr.feedback = 0;
			_spu_rev_attr.delay = 0;
			break;
	}
	v4 = &_spu_RXX[512 * _spu_core];
	v5 = (v4[205] >> 7) & 1;
	if ( v5 )
		v4[205] &= ~0x80u;
	v6 = &_spu_RXX[20 * _spu_core];
	v6[946] = 0;
	v6[947] = 0;
	_spu_rev_attr.depth.left = 0;
	_spu_rev_attr.depth.right = 0;
	_spu_setReverbAttr(&v7);
	if ( v2 )
		SpuClearReverbWorkArea(v1);
	_spu_FsetRXX(368, _spu_rev_offsetaddr, 1);
	if ( v5 )
	{
		_spu_RXX[512 * _spu_core + 205] |= 0x80u;
	}
	return 0;
}
