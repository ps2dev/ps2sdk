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

/*****************************************************************
*** Miscellaneous
***
*****************************************************************/

static char twh(short int val)
{
	char res;

	asm volatile ("plzcw   %0, %1\n": "=r" (res) : "r" (val));
	res = 31 - (res + 1);
	if(val > res)
		res++;

	return res;
}

/*-------------------------------------------
-											-
- LOW LEVEL FUNTIONS						-
-											-
-------------------------------------------*/

int GsSetXYOffset1(unsigned short x, unsigned short y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_XYOFFSET_1(((GS_R_XYOFFSET *)&p[1]), x,y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetXYOffset2(unsigned short x, unsigned short y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_XYOFFSET_2(((GS_R_XYOFFSET *)&p[1]), x,y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetScissor1(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_SCISSOR_1(((GS_R_SCISSOR *)&p[1]), upper_x,lower_x,upper_y,lower_y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetScissor2(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_SCISSOR_2(((GS_R_SCISSOR *)&p[1]), upper_x,lower_x,upper_y,lower_y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetFrame1(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char psm, unsigned int draw_mask)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_FRAME_1(((GS_R_FRAME *)&p[1]), framebuffer_addr,framebuffer_width,psm,draw_mask);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetFrame2(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char psm, unsigned int draw_mask)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_FRAME_2(((GS_R_FRAME *)&p[1]), framebuffer_addr,framebuffer_width,psm,draw_mask);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsTextureFlush(void)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEXFLUSH(((GS_R_TEXFLUSH *)&p[1]));

	GsDmaSend(GsPrimWorkArea,2);
	GsDmaWait();

	return 0;
}

int GsSetPixelTest1(unsigned char enable_alpha_test, unsigned char alpha_test_method, unsigned char alpha_reference, unsigned char alpha_fail_method, unsigned char enable_dest_alpha_test, unsigned char dest_alpha_test_mode, unsigned char enable_zbuff_test, unsigned char alpha_zbuff_method)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEST_1(((GS_R_TEST *)&p[1]), enable_alpha_test, alpha_test_method, alpha_reference, alpha_fail_method, enable_dest_alpha_test, dest_alpha_test_mode, enable_zbuff_test, alpha_zbuff_method);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetPixelTest2(unsigned char enable_alpha_test, unsigned char alpha_test_method, unsigned char alpha_reference, unsigned char alpha_fail_method, unsigned char enable_dest_alpha_test, unsigned char dest_alpha_test_mode, unsigned char enable_zbuff_test, unsigned char alpha_zbuff_method)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEST_2(((GS_R_TEST *)&p[1]), enable_alpha_test, alpha_test_method, alpha_reference, alpha_fail_method, enable_dest_alpha_test, dest_alpha_test_mode, enable_zbuff_test, alpha_zbuff_method);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSelectTexure1(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEX0_1(((GS_R_TEX0 *)&p[1]), tex_addr,addr_width,tex_pixmode, twh(tex_width), twh(tex_height),1,0,clut_addr,clut_pixmode,clut_storagemode,clut_offset,4); /*4 load contex 0*/

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSelectTexure2(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEX0_2(((GS_R_TEX0 *)&p[1]), tex_addr, addr_width, tex_pixmode, twh(tex_width), twh(tex_height), 1, 0, clut_addr, clut_pixmode, clut_storagemode,clut_offset,5);/*5 load contex 2*/

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

void GsSetFogColor(unsigned char r, unsigned char g, unsigned char b)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG*)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_FOGCOLOR(((GS_R_FOGCOLOR*)&p[1]), r,g,b);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}

void GsEnableColorClamp(unsigned short enable)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_COLCLAMP(((GS_R_COLCLAMP *)&p[1]), enable);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}
