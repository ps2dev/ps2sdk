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

int SpuSetIRQ(int on_off)
{
	int v7;

	switch ( on_off )
	{
		case SPU_OFF:
		case SPU_RESET:
		{
			vu16 *v2;
			unsigned int v3;

			v2 = &_spu_RXX[512 * _spu_core];
			v2[205] &= ~0x40u;
			v3 = 1;
			while ( (v2[205] & 0x40) != 0 )
			{
				if ( v3 >= 0xF01 )
				{
					printf("SPU:T/O [%s]\n", "wait (IRQ/ON)");
					return -1;
				}
				v3 += 1;
			}
			DisableIntr(IOP_IRQ_SPU, &v7);
			break;
		}
		default:
			break;
	}
	switch ( on_off )
	{
		case SPU_ON:
		case SPU_RESET:
		{
			vu16 *v5;
			unsigned int v6;

			v5 = &_spu_RXX[512 * _spu_core];
			v5[205] |= 0x40u;
			v6 = 1;
			while ( (v5[205] & 0x40) == 0 )
			{
				if ( v6 >= 0xF01 )
				{
					printf("SPU:T/O [%s]\n", "wait (IRQ/OFF)");
					return -1;
				}
				v6 += 1;
			}
			EnableIntr(IOP_IRQ_SPU);
			break;
		}
		default:
			break;
	}
	return on_off;
}
