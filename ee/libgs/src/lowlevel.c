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

/* Miscellaneous */

static s8 twh(s16 val)
{
	s8 res;

	asm volatile ("plzcw   %0, %1\n": "=r" (res) : "r" (val));
	res = 31 - (res + 1);
	if(val > res)
		res++;

	return res;
}

/* LOW LEVEL FUNCTIONS */

int GsSetXYOffset1(u16 x, u16 y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_XYOFFSET_1(((GS_R_XYOFFSET *)&p[1]), x,y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetXYOffset2(u16 x, u16 y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_XYOFFSET_2(((GS_R_XYOFFSET *)&p[1]), x,y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetScissor1(u16 upper_x, u16 upper_y, u16 lower_x, u16 lower_y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_SCISSOR_1(((GS_R_SCISSOR *)&p[1]), upper_x,lower_x,upper_y,lower_y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetScissor2(u16 upper_x, u16 upper_y, u16 lower_x, u16 lower_y)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_SCISSOR_2(((GS_R_SCISSOR *)&p[1]), upper_x,lower_x,upper_y,lower_y);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetFrame1(u16 framebuffer_addr, u8 framebuffer_width, u8 psm, u32 draw_mask)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_FRAME_1(((GS_R_FRAME *)&p[1]), framebuffer_addr,framebuffer_width,psm,draw_mask);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetFrame2(u16 framebuffer_addr, u8 framebuffer_width, u8 psm, u32 draw_mask)
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

int GsSetPixelTest1(u8 enable_alpha_test, u8 alpha_test_method, u8 alpha_reference, u8 alpha_fail_method, u8 enable_dest_alpha_test, u8 dest_alpha_test_mode, u8 enable_zbuff_test, u8 alpha_zbuff_method)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEST_1(((GS_R_TEST *)&p[1]), enable_alpha_test, alpha_test_method, alpha_reference, alpha_fail_method, enable_dest_alpha_test, dest_alpha_test_mode, enable_zbuff_test, alpha_zbuff_method);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSetPixelTest2(u8 enable_alpha_test, u8 alpha_test_method, u8 alpha_reference, u8 alpha_fail_method, u8 enable_dest_alpha_test, u8 dest_alpha_test_mode, u8 enable_zbuff_test, u8 alpha_zbuff_method)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEST_2(((GS_R_TEST *)&p[1]), enable_alpha_test, alpha_test_method, alpha_reference, alpha_fail_method, enable_dest_alpha_test, dest_alpha_test_mode, enable_zbuff_test, alpha_zbuff_method);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSelectTexure1(u16 tex_addr, u8 addr_width, u8 tex_pixmode, u16 tex_width, u16 tex_height, u16 clut_addr, u8 clut_pixmode, u8 clut_storagemode,u8 clut_offset)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEX0_1(((GS_R_TEX0 *)&p[1]), tex_addr,addr_width,tex_pixmode, twh(tex_width), twh(tex_height),1,0,clut_addr,clut_pixmode,clut_storagemode,clut_offset,4); /*4 load contex 0*/

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

int GsSelectTexure2(u16 tex_addr, u8 addr_width, u8 tex_pixmode, u16 tex_width, u16 tex_height, u16 clut_addr, u8 clut_pixmode, u8 clut_storagemode,u8 clut_offset)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_TEX0_2(((GS_R_TEX0 *)&p[1]), tex_addr, addr_width, tex_pixmode, twh(tex_width), twh(tex_height), 1, 0, clut_addr, clut_pixmode, clut_storagemode,clut_offset,5);/*5 load contex 2*/

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();

	return 0;
}

void GsSetFogColor(u8 r, u8 g, u8 b)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG*)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_FOGCOLOR(((GS_R_FOGCOLOR*)&p[1]), r,g,b);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}

void GsEnableColorClamp(u16 enable)
{
	QWORD *p;
	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 1,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_COLCLAMP(((GS_R_COLCLAMP *)&p[1]), enable);

	GsDmaSend(GsPrimWorkArea, 2);
	GsDmaWait();
}
