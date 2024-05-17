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

void SpuSetDigitalOut(int mode)
{
	s16 v1;
	s16 v3;

	v1 = *((vu16 *)0xBF9007C0);
	v3 = *((vu16 *)0xBF9007C6);
	switch ( mode & 0xF )
	{
		case SPU_SPDIF_OUT_OFF:
			v1 &= ~0x128;
			break;
		case SPU_SPDIF_OUT_PCM:
			v3 &= ~2;
			v1 &= ~0x128;
			v1 |= 0x20;
			break;
		case SPU_SPDIF_OUT_BITSTREAM:
			v3 |= 2;
			v1 &= ~0x128;
			v1 |= 0x100;
			break;
		case SPU_SPDIF_OUT_BYPASS:
			v1 &= ~0x128;
			v1 |= 8;
			break;
		default:
			break;
	}
	switch ( mode & 0xF0 )
	{
		case SPU_SPDIF_COPY_PROHIBIT:
			v3 |= 0x8000;
			break;
		case SPU_SPDIF_COPY_NORMAL:
		default:
			v3 &= ~0x8000;
			break;
	}
	switch ( mode & 0xF00 )
	{
		case SPU_SPDIF_MEDIA_DVD:
			*((vu16 *)0xBF9007C8) = 512;
			v3 |= 0x1800;
			break;
		case SPU_SPDIF_MEDIA_CD:
			*((vu16 *)0xBF9007C8) = 0;
			v3 &= ~0x1800;
			break;
		default:
			*((vu16 *)0xBF9007C8) = 512;
			v3 &= ~0x1800;
			v3 |= 0x800;
			break;
	}
	*((vu16 *)0xBF9007C0) = v1;
	*((vu16 *)0xBF9007C6) = v3;
}
