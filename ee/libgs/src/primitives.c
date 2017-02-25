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

extern QWORD GsPrimWorkArea[];

static GS_TEST GSGLOBAL_TEST1;
static GS_TEST GSGLOBAL_TEST2;

void GsOverridePrimAttributes(s8 override, s8 iip, s8 tme, s8 fge, s8 abe, s8 aa1, s8 fst, s8 ctxt, s8 fix)
{
	QWORD *p;

	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 2,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);

	//override. (0 = use PRIM)(1 = use PRMODE)
	if(override)
	{
		gs_setR_PRMODECONT(((GS_R_PRMODECONT *)&p[1]), 0);
	}
	else
	{
		gs_setR_PRMODECONT(((GS_R_PRMODECONT *)&p[1]), 1);
	}

	gs_setR_PRMODE(((GS_R_PRMODE *)&p[2]), iip,tme,fge,abe,aa1,fst,ctxt,fix);

	GsDmaSend(GsPrimWorkArea, 3);
	GsDmaWait();
}

void GsEnableDithering(u8 enable, int mode)
{
	QWORD *p;

	p=UNCACHED_SEG(GsPrimWorkArea);
	if(enable)
	{
		gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 3,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
		gs_setR_DTHE(((GS_R_DTHE *)&p[1]), 1);
		gs_setR_DIMX(((GS_R_DIMX *)&p[2]), 4,2,5,3,0,6,1,7,5,3,4,2,1,7,0,6); //thanks to:"Maximus32" on ps2dev.org
		gs_setR_COLCLAMP(((GS_R_COLCLAMP *)&p[3]), 1);

		GsDmaSend(GsPrimWorkArea, 4);
	}
	else
	{
		gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 2,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
		gs_setR_DTHE(((GS_R_DTHE *)&p[1]), 0);
		gs_setR_DIMX(((GS_R_DIMX *)&p[2]), 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

		GsDmaSend(GsPrimWorkArea, 3);
	}

	GsDmaWait();
}

void GsEnableAlphaTransparency1(u16 enable,u16 method,u8 alpha_ref,u16 fail_method)
{
	QWORD *p;

	p=UNCACHED_SEG(GsPrimWorkArea);
	GSGLOBAL_TEST1.atest_enable = enable;
	GSGLOBAL_TEST1.atest_method = method;
	GSGLOBAL_TEST1.atest_reference = alpha_ref;
	GSGLOBAL_TEST1.atest_fail_method = fail_method;

	GsSetPixelTest1(GSGLOBAL_TEST1.atest_enable, GSGLOBAL_TEST1.atest_method, GSGLOBAL_TEST1.atest_reference, GSGLOBAL_TEST1.atest_fail_method,
					GSGLOBAL_TEST1.datest_enable, GSGLOBAL_TEST1.datest_mode,
					GSGLOBAL_TEST1.ztest_enable,  GSGLOBAL_TEST1.ztest_method);

	// tell GS in 16bit texture 1=transparent,0=solid
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEXA(((GS_R_TEXA *)&p[1]), 0x80, 0, 0x00);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}

void GsEnableAlphaTransparency2(u16 enable,u16 method,u8 alpha_ref,u16 fail_method)
{
	QWORD *p;

	p=UNCACHED_SEG(GsPrimWorkArea);
	GSGLOBAL_TEST2.atest_enable = enable;
	GSGLOBAL_TEST2.atest_method = method;
	GSGLOBAL_TEST2.atest_reference = alpha_ref;
	GSGLOBAL_TEST2.atest_fail_method = fail_method;

	GsSetPixelTest2(GSGLOBAL_TEST2.atest_enable, GSGLOBAL_TEST2.atest_method, GSGLOBAL_TEST2.atest_reference, GSGLOBAL_TEST2.atest_fail_method,
					GSGLOBAL_TEST2.datest_enable, GSGLOBAL_TEST2.datest_mode,
					GSGLOBAL_TEST2.ztest_enable,  GSGLOBAL_TEST2.ztest_method);

	// tell GS in 16bit texture 1=transparent,0=solid
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEXA(((GS_R_TEXA *)&p[1]), 0x80, 0, 0x00);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}

void GsEnableZbuffer1(u16 enable,u16 test_method)
{
	GSGLOBAL_TEST1.ztest_enable = enable;
	GSGLOBAL_TEST1.ztest_method = test_method;

	GsSetPixelTest1(GSGLOBAL_TEST1.atest_enable, GSGLOBAL_TEST1.atest_method, GSGLOBAL_TEST1.atest_reference, GSGLOBAL_TEST1.atest_fail_method,
					GSGLOBAL_TEST1.datest_enable, GSGLOBAL_TEST1.datest_mode,
					GSGLOBAL_TEST1.ztest_enable,  GSGLOBAL_TEST1.ztest_method);
}

void GsEnableZbuffer2(u16 enable,u16 test_method)
{
	GSGLOBAL_TEST2.ztest_enable = enable;
	GSGLOBAL_TEST2.ztest_method = test_method;

	GsSetPixelTest2(GSGLOBAL_TEST2.atest_enable, GSGLOBAL_TEST2.atest_method, GSGLOBAL_TEST2.atest_reference, GSGLOBAL_TEST2.atest_fail_method,
					GSGLOBAL_TEST2.datest_enable, GSGLOBAL_TEST2.datest_mode,
					GSGLOBAL_TEST2.ztest_enable,  GSGLOBAL_TEST2.ztest_method);

}

void GsEnableAlphaBlending1(u16 enable)
{
	QWORD *p;

	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);

	if(enable)
	{
		gs_setR_ALPHA_1(((GS_R_ALPHA *)&p[1]), 0,1,0,1,0x00); //last arg(0x00) is not used bceause of other settings
	}
	else
	{
		gs_setR_ALPHA_1(((GS_R_ALPHA *)&p[1]), 0,0,0,0,0x80);
	}
	gs_setR_PABE(((GS_R_PABE *)&p[2]), 0);
	gs_setR_FBA_1(((GS_R_FBA *)&p[3]), 0);
	gs_setR_TEXA(((GS_R_TEXA *)&p[4]), 0x80, 0, 0x00);

	GsDmaSend(GsPrimWorkArea, 5);
	GsDmaWait();
}

void GsEnableAlphaBlending2(u16 enable)
{
	QWORD *p;

	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG*)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);

	if(enable)
	{
		gs_setR_ALPHA_2(((GS_R_ALPHA*)&p[1]), 0,1,0,1,0x00); //last arg(0x00) is not used bceause of other settings
	}
	else
	{
		gs_setR_ALPHA_2(((GS_R_ALPHA *)&p[1]), 0,0,0,0,0x80);
	}
	gs_setR_PABE(((GS_R_PABE *)&p[2]), 0);
	gs_setR_FBA_2(((GS_R_FBA *)&p[3]), 0);
	gs_setR_TEXA(((GS_R_TEXA *)&p[4]), 0x80, 0, 0x00);

	GsDmaSend(GsPrimWorkArea, 5);
	GsDmaWait();
}
