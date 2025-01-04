/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"

#include "iop_mmio_hwport.h"

// Based on the module from SDK 3.1.0.

int _start(int ac, char **av)
{
	u16 bootmode_7_val;
  USE_IOP_MMIO_HWPORT();

	(void)av;
	if ( ac > 0 )
	{
		return 1;
	}
	{
		u32 bootmode_tmp[1];

		bootmode_tmp[0] = (iop_mmio_hwport->exp2_r2[4612] & 0xFFFF) | 0x60000;
		RegisterBootMode((iop_bootmode_t *)bootmode_tmp);
	}
	switch ( iop_mmio_hwport->exp2_r2[4612] & 0xF8 )
	{
		case 0:
		case 16:
		case 32:
		case 48:
			bootmode_7_val = 0x00C8;
			break;
		case 64:
		case 80:
			bootmode_7_val = 0x012C;
			break;
		default:
			bootmode_7_val = 0x0126;
			break;
	}
	{
		u32 bootmode_tmp[1];

		bootmode_tmp[0] = (bootmode_7_val & 0xFFFF) | 0x70000;
		RegisterBootMode((iop_bootmode_t *)bootmode_tmp);
	}
	return 1;
}
