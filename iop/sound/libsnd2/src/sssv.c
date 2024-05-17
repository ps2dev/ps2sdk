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

void SsSetSerialVol(char s_num, s16 voll, s16 volr)
{
	s16 voll_tmp;
	s16 volr_tmp;
	SpuCommonAttr spu_attr;

	voll_tmp = voll;
	volr_tmp = volr;
	if ( voll_tmp >= 128 )
		voll_tmp = 127;
	if ( volr_tmp >= 128 )
		volr_tmp = 127;
	switch ( s_num )
	{
		case SS_SERIAL_A:
			spu_attr.mask = SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
			spu_attr.cd.volume.left = 258 * voll_tmp;
			spu_attr.cd.volume.right = 258 * volr_tmp;
			break;
		case SS_SERIAL_B:
			spu_attr.mask = SPU_COMMON_EXTVOLL | SPU_COMMON_EXTVOLR;
			spu_attr.ext.volume.left = 258 * voll_tmp;
			spu_attr.ext.volume.right = 258 * volr_tmp;
			break;
	}
	SpuSetCommonAttr(&spu_attr);
}
