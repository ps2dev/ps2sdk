/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <errno.h>
#include <stdio.h>
#include <kernel.h>
#include <libgs.h>

#include "internal.h"

void GsDrawSync(int mode)
{
	switch(mode)
	{
	case 0:
	default:
		GsDmaWait();
	}
}

void GsHSync(int mode)
{
	unsigned short i;

	switch(mode)
	{
	case 0:
		GS_SET_CSR_hsync_intrupt(1);
		while(!GS_GET_CSR_hsync_intrupt);
	break;
	default:
		if(mode>1)
		{
			for(i=0;i<mode;i++)
			{
				GS_SET_CSR_hsync_intrupt(1);
				while(!GS_GET_CSR_hsync_intrupt);
			}
		}
	}
}

void GsVSync(int mode)
{
	unsigned short i;

	switch(mode)
	{
	case 0: //just wait
		GS_SET_CSR_vsync_intrupt(1);
		while(!GS_GET_CSR_vsync_intrupt);
	break;
	default: // wait for num of vsync to pass
		if(mode>1)
		{

			for(i=0;i<mode;i++)
			{
				GS_SET_CSR_vsync_intrupt(1);
				while(!GS_GET_CSR_vsync_intrupt);

			}
		}
	}
}
