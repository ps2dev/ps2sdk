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

void SpuSetAutoDMAAttr(s16 vol_l, s16 vol_r, s16 dry_on, s16 effect_on)
{
	*((vu16 *)0xBF90076C) = vol_l;
	*((vu16 *)0xBF90076E) = vol_r;
	switch ( dry_on )
	{
		case SPU_ON:
			*((vu16 *)0xBF900198) = *((vu16 *)0xBF900198) | 0xC0;
			break;
		case SPU_OFF:
		default:
			*((vu16 *)0xBF900198) = *((vu16 *)0xBF900198) & ~0xc0;
			break;
	}
	switch ( effect_on )
	{
		case SPU_ON:
			*((vu16 *)0xBF900198) |= 0x30u;
			break;
		case SPU_OFF:
		default:
			*((vu16 *)0xBF900198) &= ~0x30;
			break;
	}
}

void SpuSetSerialInAttr(s16 dry_on, s16 effect_on)
{
	switch ( dry_on )
	{
		case SPU_ON:
			*((vu16 *)0xBF900598) = *((vu16 *)0xBF900598) | 0xC;
			break;
		case SPU_OFF:
		default:
			*((vu16 *)0xBF900598) = *((vu16 *)0xBF900598) & ~0xC;
			break;
	}
	switch ( effect_on )
	{
		case SPU_ON:
			*((vu16 *)0xBF900598) |= 3u;
			break;
		case SPU_OFF:
		default:
			*((vu16 *)0xBF900598) &= ~3;
			break;
	}
}
