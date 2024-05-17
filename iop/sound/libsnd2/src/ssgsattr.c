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

char SsGetSerialAttr(char s_num, char attr)
{
	SpuCommonAttr spu_attr;

	SpuGetCommonAttr(&spu_attr);
	switch ( s_num )
	{
		case SS_SERIAL_A:
			switch ( attr )
			{
				case SS_MIX:
					return spu_attr.cd.mix;
				case SS_REV:
					return spu_attr.cd.reverb;
				default:
					break;
			}
			break;
		case SS_SERIAL_B:
			switch ( attr )
			{
				case SS_MIX:
					return spu_attr.ext.mix;
				case SS_REV:
					return spu_attr.ext.reverb;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return -1;
}
