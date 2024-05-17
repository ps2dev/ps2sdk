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

void SpuSetReverbModeDelayTime(int delay)
{
	libspu2_reverb_param_entry_t v3;

	switch ( _spu_rev_attr.mode )
	{
		case SPU_REV_MODE_DELAY:
		case SPU_REV_MODE_ECHO:
		{
			int v2;

			memcpy(&v3, &_spu_rev_param[_spu_rev_attr.mode], 0x44u);
			v3.flags = 0xc011c00;
			_spu_rev_attr.delay = delay;
			v3.mLSAME = (s16)((u16)delay << 13) / 127 - v3.dAPF1;
			v2 = (delay << 12) / 127;
			v3.mRSAME = v2 - v3.dAPF2;
			v3.dLSAME = v3.dRSAME + v2;
			v3.mLCOMB1 = v3.mRCOMB1 + v2;
			v3.mRAPF1 = v3.mRAPF2 + v2;
			v3.mLAPF1 = v3.mLAPF2 + v2;
			_spu_setReverbAttr(&v3);
			break;
		}
		default:
			break;
	}
}
