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

void SpuStopFreeRun(void)
{
	u16 *v1;
	s16 *v2;
	u16 *v3;
	u16 *v4;
	int v15;
	int i;
	int j;
	int k;

	v1 = (u16 *)0xBF900000;
	v2 = (s16 *)0xBF900400;
	v3 = (u16 *)0xBF9001C0;
	v4 = (u16 *)0xBF9005C0;
	*((vu16 *)0xBF9001A8) = 0;
	*((vu16 *)0xBF9001AA) = 10240;
	// Unofficial: Fixed out of bounds dummy array access by replacing with constant setting
	for ( v15 = 0; v15 < 16; v15 += 1 )
	{
		*((vu16 *)0xBF9001AC) = 1799u;
	}
	*((vu16 *)0xBF90019A) = (*((vu16 *)0xBF90019A) & ~0x30) | 0x10;
	while ( (*((vu16 *)0xBF900344) & 0x400) != 0 )
		;
	*((vu16 *)0xBF90019A) &= ~0x30;
	for ( i = 0; i < 24; i += 1 )
	{
		s16 *v6;
		u16 *v7;
		u16 *v13;
		u16 *v14;

		v6 = v2 + (i * 8);
		v7 = v1 + (i * 8);
		v13 = v4 + (i * 6);
		v14 = v3 + (i * 6);
		v6[0] = 0;
		v7[0] = v6[0];
		v6[1] = 0;
		v7[1] = v6[1];
		v6[2] = 0x3FFF;
		v7[2] = v6[2];
		v6[3] = 0;
		v7[3] = v6[3];
		v6[4] = 0;
		v7[4] = v6[4];
		v13[0] = 0;
		v14[0] = v13[0];
		v13[1] = 10240;
		v14[1] = v13[1];
	}
	*((vu16 *)0xBF9005A0) = -1;
	*((vu16 *)0xBF9001A0) = -1;
	*((vu16 *)0xBF9005A2) = 255;
	*((vu16 *)0xBF9001A2) = 255;
	for ( j = 0; j < 3124; j += 1 )
	{
		__asm__ __volatile__("" : "+g"(j) : :);
	}
	*((vu16 *)0xBF9005A4) = -1;
	*((vu16 *)0xBF9001A4) = -1;
	*((vu16 *)0xBF9005A6) = 255;
	*((vu16 *)0xBF9001A6) = 255;
	for ( k = 0; k < 3124; k += 1 )
	{
		__asm__ __volatile__("" : "+g"(k) : :);
	}
	*((vu16 *)0xBF900342) = 0;
	*((vu16 *)0xBF900340) = 0;
}
