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

#ifdef _IOP
IRX_ID("SifInit", 1, 1);
#endif
// Based on the module from SCE SDK 1.3.4.

int _start(int ac, char **av)
{
	const int *BootMode3;

	(void)ac;
	(void)av;

	BootMode3 = QueryBootMode(3);
	if ( BootMode3 && (BootMode3[1] & 1) != 0 )
	{
		printf(" Skip SIF init\n");
		return 1;
	}
	else
	{
		const int *BootMode1;

		BootMode1 = QueryBootMode(1);
		if ( BootMode1 && *(const u16 *)BootMode1 == 1 )
		{
			printf(" Skip SIF init (it is DECI1)\n");
			return 1;
		}
		else
		{
			sceSifInit();
			return 1;
		}
	}
}
