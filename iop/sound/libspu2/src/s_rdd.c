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

int SpuReadDecodedData(SpuDecodedData *d_data, int flag)
{
	int v3;
	u32 v4;

	v3 = 0;
	v4 = 32;
	switch ( flag )
	{
		case SPU_CDONLY:
			break;
		case SPU_VOICEONLY:
			d_data = (SpuDecodedData *)((char *)d_data + 2048);
			v3 = 256;
			break;
		case SPU_ALL:
		default:
			v4 = 64;
			break;
	}
	_spu_Fr_(d_data, v3, v4);
	return ((_spu_RXX[512 * _spu_core + 418] & 0x800) != 0) ? SPU_DECODED_SECONDHALF : SPU_DECODED_FIRSTHALF;
}
