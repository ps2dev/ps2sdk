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

void GsSetDefaultZBufferEnv(GS_ZENV *zenv, unsigned char update_mask)
{
	zenv->update_mask = update_mask;
}

void GsSetDefaultZBufferEnvAddress(GS_ZENV *zenv, unsigned short vram_addr, unsigned char psm)
{
	zenv->vram_addr=vram_addr;
	zenv->psm=psm;
}

void GsPutZBufferEnv1(GS_ZENV *zenv)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_ZBUF_1(((GS_R_ZBUF *)&p[1]), zenv->vram_addr, zenv->psm, zenv->update_mask);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}

void GsPutZBufferEnv2(GS_ZENV *zenv)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_ZBUF_2(((GS_R_ZBUF *)&p[1]), zenv->vram_addr, zenv->psm, zenv->update_mask);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}
