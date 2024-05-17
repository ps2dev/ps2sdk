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

int SpuSetReverbModeParam(SpuReverbAttr *attr)
{
	int b_set_spucnt;
	int b_r_mode_in_bounds;
	int b_mode_is_7_to_9_bit0x8;
	unsigned int mask;
	int b_mode_is_7_to_9_bit0x10;
	u32 flagstmp;
	libspu2_reverb_param_entry_t entry;
	int b_clear_reverb_work_area;

	b_set_spucnt = 0;
	b_r_mode_in_bounds = 0;
	b_mode_is_7_to_9_bit0x8 = 0;
	b_clear_reverb_work_area = 0;
	mask = attr->mask;
	b_mode_is_7_to_9_bit0x10 = 0;
	entry.flags = 0;
	if ( mask == 0 )
		mask = 0xFFFFFFFF;
	if ( (mask & SPU_REV_MODE) != 0 )
	{
		unsigned int mode;

		mode = attr->mode;
		if ( (mode & SPU_REV_MODE_CLEAR_WA) != 0 )
		{
			mode &= ~SPU_REV_MODE_CLEAR_WA;
			b_clear_reverb_work_area = 1;
		}
		b_r_mode_in_bounds = 1;
		if ( mode >= SPU_REV_MODE_MAX )
			return SPU_ERROR;
		_spu_rev_attr.mode = mode;
		_spu_rev_offsetaddr = SpuGetReverbEndAddr() - (8 * _spu_rev_workareasize[mode] - 2);
		printf("_spu_rev_offsetaddr %x\n", _spu_rev_offsetaddr);
		memcpy(&entry, &_spu_rev_param[_spu_rev_attr.mode], sizeof(entry));
		switch ( _spu_rev_attr.mode )
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
	}
	if (
		((mask & SPU_REV_DELAYTIME) != 0) && _spu_rev_attr.mode <= SPU_REV_MODE_DELAY
		&& _spu_rev_attr.mode >= SPU_REV_MODE_ECHO )
	{
		int delay_converted;

		b_mode_is_7_to_9_bit0x8 = 1;
		if ( !b_r_mode_in_bounds )
		{
			memcpy(&entry, &_spu_rev_param[_spu_rev_attr.mode], sizeof(entry));
			entry.flags = 0xc011c00;
		}
		_spu_rev_attr.delay = attr->delay;
		entry.mLSAME = (s16)((_spu_rev_attr.delay & 0xFFFF) << 13) / 127 - entry.dAPF1;
		delay_converted = (_spu_rev_attr.delay << 12) / 127;
		entry.mRSAME = delay_converted - entry.dAPF2;
		entry.dLSAME = entry.dRSAME + delay_converted;
		entry.mLCOMB1 = entry.mRCOMB1 + delay_converted;
		entry.mRAPF1 = entry.mRAPF2 + delay_converted;
		entry.mLAPF1 = entry.mLAPF2 + delay_converted;
	}
	if (
		((mask & SPU_REV_FEEDBACK) != 0) && _spu_rev_attr.mode <= SPU_REV_MODE_DELAY
		&& _spu_rev_attr.mode >= SPU_REV_MODE_ECHO )
	{
		b_mode_is_7_to_9_bit0x10 = 1;
		if ( !b_r_mode_in_bounds )
		{
			if ( b_mode_is_7_to_9_bit0x8 )
			{
				flagstmp = entry.flags | 0x80;
			}
			else
			{
				memcpy(&entry, &_spu_rev_param[_spu_rev_attr.mode], sizeof(entry));
				flagstmp = 128;
			}
			entry.flags = flagstmp;
		}
		_spu_rev_attr.feedback = attr->feedback;
		entry.vWALL = 33024 * _spu_rev_attr.feedback / 127;
	}
	if ( b_r_mode_in_bounds )
	{
		vu16 *regsptr;
		vu16 *regstmp1;

		regsptr = &_spu_RXX[512 * _spu_core];
		b_set_spucnt = (regsptr[205] >> 7) & 1;
		if ( b_set_spucnt )
			regsptr[205] &= ~0x80u;
		regstmp1 = &_spu_RXX[20 * _spu_core];
		regstmp1[946] = 0;
		regstmp1[947] = 0;
		_spu_rev_attr.depth.left = 0;
		_spu_rev_attr.depth.right = 0;
	}
	else
	{
		if ( (mask & SPU_REV_DEPTHL) != 0 )
		{
			_spu_RXX[20 * _spu_core + 946] = attr->depth.left;
			_spu_rev_attr.depth.left = attr->depth.left;
		}
		if ( (mask & SPU_REV_DEPTHR) != 0 )
		{
			_spu_RXX[20 * _spu_core + 947] = attr->depth.right;
			_spu_rev_attr.depth.right = attr->depth.right;
		}
	}
	if ( b_r_mode_in_bounds || b_mode_is_7_to_9_bit0x8 || b_mode_is_7_to_9_bit0x10 )
		_spu_setReverbAttr(&entry);
	if ( b_clear_reverb_work_area )
		SpuClearReverbWorkArea(_spu_rev_attr.mode);
	if ( b_r_mode_in_bounds )
	{
		_spu_FsetRXX(368, _spu_rev_offsetaddr, 1);
		if ( b_set_spucnt )
			_spu_RXX[512 * _spu_core + 205] |= 0x80u;
	}
	return 0;
}
