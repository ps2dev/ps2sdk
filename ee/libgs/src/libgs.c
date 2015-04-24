/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <errno.h>
#include <stdio.h>
#include <kernel.h>
#include <libgs.h>

#include "internal.h"

extern QWORD GsPrimWorkArea[];

/*-------------------------------------------
-
- Initialization functions.											-
-
-------------------------------------------*/

static GsGParam_t gp_15={GS_INTERLACED, GS_MODE_NTSC, GS_FFMD_FRAME, 3};

GsGParam_t *GsGetGParam(void){
	return &gp_15;
}

void GsResetGraph(short int mode, short int interlace, short int omode, short int ffmode)
{
	GsGParam_t *pGParams;

	switch(mode){
		case GS_INIT_RESET:
			GsDmaInit();	//It seems like the Sony developers reset the GIF DMA channel with their libdma library, but a lot of homebrew GS libraries will do this on their own.

			pGParams=GsGetGParam();
			GS_SET_CSR_reset(1);
			pGParams->ffmode=ffmode;
			pGParams->interlace=interlace;
			pGParams->omode=omode;
			pGParams->version=GS_GET_CSR_gs_rev_number>>16;

			GsPutIMR(0xFF00);

			SetGsCrt(interlace&1, omode&0xFF, ffmode&1);
			GsSetCRTCSettings(CRTC_SETTINGS_DEFAULT1, 0x80);
			break;
		case GS_INIT_DRAW_RESET:
			GS_SET_CSR_flush(1);
			break;
	/*	There is a mode 5 present in the Sony sceGsResetGraph, but is not documented. It seems to change the video mode without resetting anything. */
	}
}

void GsSetCRTCSettings(unsigned long settings, unsigned char alpha_value)
{
	*((volatile unsigned long *)(gs_p_pmode)) =  (settings|((unsigned long)(0x001) << 2)|((unsigned long)(alpha_value) 	<< 8));
}
