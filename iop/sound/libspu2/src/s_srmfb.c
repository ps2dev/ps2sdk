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

void SpuSetReverbModeFeedback(int feedback)
{
	libspu2_reverb_param_entry_t v2;

	switch ( _spu_rev_attr.mode )
	{
		case SPU_REV_MODE_DELAY:
		case SPU_REV_MODE_ECHO:
		{
			memcpy(&v2, &_spu_rev_param[_spu_rev_attr.mode], 0x44u);
			v2.flags = 128;
			_spu_rev_attr.feedback = feedback;
			v2.vWALL = 33024 * feedback / 127;
			_spu_setReverbAttr(&v2);
			break;
		}
		default:
			break;
	}
}
