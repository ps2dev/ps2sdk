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

void SsSetSerialAttr(char s_num, char attr, char mode)
{
	SpuCommonAttr spu_attr;

	switch ( s_num )
	{
		case SS_SERIAL_A:
			switch ( attr )
			{
				case SS_MIX:
					spu_attr.mask = SPU_COMMON_CDMIX;
					spu_attr.cd.mix = mode;
					break;
				case SS_REV:
					spu_attr.mask = SPU_COMMON_CDREV;
					spu_attr.cd.reverb = mode;
					break;
				default:
					break;
			}
			break;
		case SS_SERIAL_B:
			switch ( attr )
			{
				case SS_MIX:
					spu_attr.mask = SPU_COMMON_EXTMIX;
					spu_attr.ext.mix = mode;
					break;
				case SS_REV:
					spu_attr.mask = SPU_COMMON_EXTREV;
					spu_attr.ext.reverb = mode;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	SpuSetCommonAttr(&spu_attr);
}
