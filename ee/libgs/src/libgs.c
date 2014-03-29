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

/*static*/
static int gs_dma_send(unsigned int *addr, unsigned int qwords);
static int gs_dma_send_tag(unsigned int *addr, unsigned int qwords, GS_GIF_DMACHAIN_TAG *tag);
static int gs_dma_wait(void);

static QWORD prim_work[64] __attribute__((aligned(64))); /*align to 64-byte boundaries. */

static GS_TEST GSGLOBAL_TEST1;
static GS_TEST GSGLOBAL_TEST2;

static int gs_db_draw_buffer=0;

/*-------------------------------------------
-											-
- LOW LEVEL FUNTIONS						-
-											-
-------------------------------------------*/

int GsSetXYOffset1(unsigned short x, unsigned short y)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_XYOFFSET_1(((GS_R_XYOFFSET *)&prim_work[1]), x,y);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));

	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetXYOffset2(unsigned short x, unsigned short y)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_XYOFFSET_2(((GS_R_XYOFFSET *)&prim_work[1]), x,y);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetScissor1(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_SCISSOR_1(((GS_R_SCISSOR *)&prim_work[1]), upper_x,lower_x,upper_y,lower_y);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetScissor2(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_SCISSOR_2(((GS_R_SCISSOR *)&prim_work[1]), upper_x,lower_x,upper_y,lower_y);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetFrame1(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char psm, unsigned int draw_mask)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_FRAME_1(((GS_R_FRAME *)&prim_work[1]), framebuffer_addr,framebuffer_width,psm,draw_mask);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetFrame2(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char psm, unsigned int draw_mask)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_FRAME_2(((GS_R_FRAME *)&prim_work[1]), framebuffer_addr,framebuffer_width,psm,draw_mask);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsTextureFlush(void)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEXFLUSH(((GS_R_TEXFLUSH *)&prim_work[1]));

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetPixelTest1(unsigned char enable_alpha_test, unsigned char alpha_test_method, unsigned char alpha_reference, unsigned char alpha_fail_method, unsigned char enable_dest_alpha_test, unsigned char dest_alpha_test_mode, unsigned char enable_zbuff_test, unsigned char alpha_zbuff_method)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEST_1(((GS_R_TEST *)&prim_work[1]), enable_alpha_test, alpha_test_method, alpha_reference, alpha_fail_method, enable_dest_alpha_test, dest_alpha_test_mode, enable_zbuff_test, alpha_zbuff_method);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSetPixelTest2(unsigned char enable_alpha_test, unsigned char alpha_test_method, unsigned char alpha_reference, unsigned char alpha_fail_method, unsigned char enable_dest_alpha_test, unsigned char dest_alpha_test_mode, unsigned char enable_zbuff_test, unsigned char alpha_zbuff_method)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEST_2(((GS_R_TEST *)&prim_work[1]), enable_alpha_test, alpha_test_method, alpha_reference, alpha_fail_method, enable_dest_alpha_test, dest_alpha_test_mode, enable_zbuff_test, alpha_zbuff_method);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSelectTexure1(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEX0_1(((GS_R_TEX0 *)&prim_work[1]), tex_addr,addr_width,tex_pixmode, twh4(tex_width), twh4(tex_height),1,0,clut_addr,clut_pixmode,clut_storagemode,clut_offset,4); /*4 load contex 0*/

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsSelectTexure2(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEX0_2(((GS_R_TEX0 *)&prim_work[1]), tex_addr, addr_width, tex_pixmode, twh4(tex_width), twh4(tex_height), 1, 0, clut_addr, clut_pixmode, clut_storagemode,clut_offset,5);/*5 load contex 2*/

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

void GsSetFogColor(unsigned char r, unsigned char g, unsigned char b)
{

	gs_setGIF_TAG(((GS_GIF_TAG		 *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_FOGCOLOR(((GS_R_FOGCOLOR *)&prim_work[1]), r,g,b);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();
}

void GsEnableColorClamp(unsigned short enable)
{
	gs_setGIF_TAG(((GS_GIF_TAG			*)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_COLCLAMP(((GS_R_COLCLAMP	*)&prim_work[1]), enable);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();
}

/*-------------------------------------------
-
- Initialization functions.											-
-
-------------------------------------------*/

static unsigned short int VideoMode;

int GsInit(short int interlace, short int videomode, short int ffmd)
{
	/*Reset/init dma(gif channel only)*/
	__asm__(
	"li	$2,0x1000A000	\n"
	"nop				\n"
	"sw	$0,0x80($2)		\n"
	"sw	$0,0($2)		\n"
	"sw	$0,0x30($2)		\n"
	"sw	$0,0x10($2)		\n"
	"sw	$0,0x50($2)		\n"
	"sw	$0,0x40($2)		\n"
	"li	$2,0xFF1F		\n"//	0xFF1F
	"sw	$2,0x1000E010	\n"
	"lw	$2,0x1000E010	\n"
	"li	$3,0xFF1F		\n"//0xFF1F
	"and	$2,$3		\n"
	"sw	$2,0x1000E010	\n"
	"sync.p				\n"
	"sw	$0,0x1000E000	\n"
	"sw	$0,0x1000E020	\n"
	"sw	$0,0x1000E030	\n"
	"sw	$0,0x1000E050	\n"
	"sw	$0,0x1000E040	\n"
	"li	$3,1			\n"
	"lw	$2,0x1000E000	\n"
	"ori	$3,$2,1		\n"
	"nop				\n"
	"sw	$3,0x1000E000	\n"
	"nop				\n"
	);

	//reset the GS
	GS_SET_CSR_flush(1);
	GS_SET_CSR_reset(1);

	//wait for GS to finish reset
	__asm__(
		"sync.p \n"
		"nop	\n"
		);

	return GsSetVideoMode(interlace, videomode,  ffmd);
}

int GsSetCRTCSettings(unsigned long settings, unsigned char alpha_value)
{
	*((volatile unsigned long *)(gs_p_pmode)) =  (settings|((unsigned long)(001) << 2)|((unsigned long)(alpha_value) 	<< 8));

	return 0;
}

int GsSetVideoMode(short int interlace, short int videomode, short int ffmd)
{
	SetGsCrt(interlace, videomode, ffmd);
	GsSetCRTCSettings(CRTC_SETTINGS_DEFAULT1, 255);
	VideoMode=videomode;
	return 0;
}

int GsSetDefaultDrawEnv(GS_DRAWENV *drawenv, unsigned short int w, unsigned short int h)
{
	drawenv->offset_x	= 2048-w/2;
	drawenv->offset_y	= 2048-h/2;
	drawenv->clip.x		= 0;
	drawenv->clip.y		= 0;
	drawenv->clip.w		= w;
	drawenv->clip.h		= h;
	drawenv->draw_mask	= 0;
	drawenv->auto_clear	= 1;

	drawenv->bg_color.r	= 0x01;
	drawenv->bg_color.g	= 0x01;
	drawenv->bg_color.b	= 0x01;
	drawenv->bg_color.a	= 0x80;
	drawenv->bg_color.q	= 0.0f;

	return 0;
}

int GsSetDefaultDrawEnvAddress(GS_DRAWENV *drawenv, unsigned short vram_addr, unsigned char fbw, unsigned char psm)
{
	drawenv->vram_addr=vram_addr;
	drawenv->fbw=fbw;
	drawenv->psm=psm;
	drawenv->vram_x=0;
	drawenv->vram_y=0;

	return 0;
}

int GsSetDefaultDisplayEnv(GS_DISPENV *dispenv, unsigned short int w, unsigned short int h, unsigned short int x, unsigned short int y)
{
	int result;

	dispenv->disp.pad1=dispenv->disp.pad2=0;

	switch(VideoMode){
		case GS_MODE_NTSC:
			dispenv->disp.display_w	= 2560-1;
			dispenv->disp.display_h	= h-1;
			dispenv->disp.display_x	= 636+x*(2560/w);
			dispenv->disp.display_y	= 50+y;
			dispenv->disp.magnify_h	= (2560+w-1)/w-1;
			dispenv->disp.magnify_v	= 0;
			result=0;
			break;
		case GS_MODE_PAL:
			dispenv->disp.display_w	= 2560-1;
			dispenv->disp.display_h	= h-1;
			dispenv->disp.display_x	= 656+x*(2560/w);
			dispenv->disp.display_y	= 72+y;
			dispenv->disp.magnify_h	= (2560+w-1)/w-1;
			dispenv->disp.magnify_v	= 0;
			result=0;
			break;
		case GS_MODE_DTV_480P:
			dispenv->disp.display_w	= (w<<1)-1;
			dispenv->disp.display_h	= h-1;
			dispenv->disp.display_x = ((((((720-w)>>31)+(720-w))>>1<<1)+((x<<1)+232))&0xFFF)|0x00800000;
			dispenv->disp.display_y = (y+35)&0xFFF;
			dispenv->disp.magnify_h	= 0;
			dispenv->disp.magnify_v	= 0;
			result=0;
			break;
		default:
			printf("GsSetDefaultDisplayEnv: Unsupported video mode: 0x%x\n", VideoMode);
			result=-EINVAL;
	}

	return 0;
}

int GsSetDefaultDisplayEnvAddress(GS_DISPENV *dispenv, unsigned short vram_addr, unsigned char fbw, unsigned char psm)
{
	dispenv->dispfb.fbw=fbw;
	dispenv->dispfb.psm=psm;
	dispenv->dispfb.pad1=dispenv->dispfb.pad2=0;
	dispenv->dispfb.x=0;
	dispenv->dispfb.y=0;
	dispenv->dispfb.address=vram_addr;

	return 0;
}

int GsSetDefaultZBufferEnv(GS_ZENV *zenv, unsigned char update_mask)
{
	zenv->update_mask = update_mask;

	return 0;
}

int GsSetDefaultZBufferEnvAddress(GS_ZENV *zenv, unsigned short vram_addr, unsigned char psm)
{
	zenv->vram_addr=vram_addr;
	zenv->psm=psm;

	return 0;
}

int GsPutDrawEnv1(GS_DRAWENV *drawenv)
{
	GsSetXYOffset1(drawenv->offset_x<<4, drawenv->offset_y<<4);
	GsSetScissor1(drawenv->clip.x, drawenv->clip.y, drawenv->clip.x+drawenv->clip.w, drawenv->clip.y+drawenv->clip.h);
	GsSetFrame1(drawenv->vram_addr, drawenv->fbw, drawenv->psm, drawenv->draw_mask);

	//use a sprite to clear background
	if(drawenv->auto_clear)
	{
		GsClearDrawEnv1(drawenv);
	}

	return 0;
}

int GsPutDrawEnv2(GS_DRAWENV *drawenv)
{
	GsSetXYOffset2(drawenv->offset_x<<4, drawenv->offset_y<<4);
	GsSetScissor2(drawenv->clip.x, drawenv->clip.y, drawenv->clip.x+drawenv->clip.w, drawenv->clip.y+drawenv->clip.h);
	GsSetFrame2(drawenv->vram_addr, drawenv->fbw, drawenv->psm, drawenv->draw_mask);

	//use a sprite to clear background
	if(drawenv->auto_clear)
	{
		GsClearDrawEnv2(drawenv);
	}

	return 0;
}

int GsClearDrawEnv1(GS_DRAWENV *drawenv)
{
	unsigned char context;

	// use a sprite to clear background
	context =0; // contex 1

	gs_setGIF_TAG(((GS_GIF_TAG	*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);
	gs_setR_PRIM(((GS_R_PRIM	*)&prim_work[1]), GS_PRIM_SPRITE,0, 0, 0, 1, 0, 1, context, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ	*)&prim_work[2]), drawenv->bg_color.r, drawenv->bg_color.g, drawenv->bg_color.b, drawenv->bg_color.a, drawenv->bg_color.q);
	gs_setR_XYZ2(((GS_R_XYZ		*)&prim_work[3]), (drawenv->offset_x+drawenv->clip.x)<<4, (drawenv->offset_y+drawenv->clip.y)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&prim_work[4]), (drawenv->offset_x+drawenv->clip.x+drawenv->clip.w)<<4, (drawenv->offset_y+drawenv->clip.y+drawenv->clip.h)<<4, 0x00000000);

	SyncDCache(prim_work, prim_work+5*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],5);
	gs_dma_wait();

	return 0;
}

int GsClearDrawEnv2(GS_DRAWENV *drawenv)
{
	unsigned char	context;

	// use a sprite to clear background
	context =1; // contex 2

	gs_setGIF_TAG(((GS_GIF_TAG	*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);
	gs_setR_PRIM(((GS_R_PRIM	*)&prim_work[1]), GS_PRIM_SPRITE,0, 0, 0, 1, 0, 1, context, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ	*)&prim_work[2]), drawenv->bg_color.r, drawenv->bg_color.g, drawenv->bg_color.b, drawenv->bg_color.a, drawenv->bg_color.q);
	gs_setR_XYZ2(((GS_R_XYZ		*)&prim_work[3]), (drawenv->offset_x+drawenv->clip.x)<<4, (drawenv->offset_y+drawenv->clip.y)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&prim_work[4]), (drawenv->offset_x+drawenv->clip.x+drawenv->clip.w)<<4, (drawenv->offset_y+drawenv->clip.y+drawenv->clip.h)<<4, 0x00000000);

	SyncDCache(prim_work, prim_work+5*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],5);
	gs_dma_wait();

	return 0;
}

int GsPutDisplayEnv1(GS_DISPENV *dispenv)
{
	*((volatile unsigned long *)(gs_p_display1)) =  *(unsigned long *)(&dispenv->disp);
	*((volatile unsigned long *)(gs_p_dispfb1)) =  *(unsigned long *)(&dispenv->dispfb);

	return 1;
}

int GsPutDisplayEnv2(GS_DISPENV *dispenv)
{
	*((volatile unsigned long *)(gs_p_display2)) =  *(unsigned long *)(&dispenv->disp);
	*((volatile unsigned long *)(gs_p_dispfb2)) =  *(unsigned long *)(&dispenv->dispfb);

	return 1;
}

int GsPutZBufferEnv1(GS_ZENV *zenv)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_ZBUF_1(((GS_R_ZBUF *)&prim_work[1]), zenv->vram_addr, zenv->psm, zenv->update_mask);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsPutZBufferEnv2(GS_ZENV *zenv)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);
	gs_setR_ZBUF_2(((GS_R_ZBUF *)&prim_work[1]), zenv->vram_addr, zenv->psm, zenv->update_mask);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();

	return 0;
}

int GsOverridePrimAttributes(char override, char iip, char tme, char fge, char abe, char aa1, char fst, char ctxt, char fix)
{
	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 2,1,0,0,0,0,1,0x0e);

	//override. (0 = use PRIM)(1 = use PRMODE)
	if(override)
	{
		gs_setR_PRMODECONT(((GS_R_PRMODECONT *)&prim_work[1]), 0);
	}
	else
	{
		gs_setR_PRMODECONT(((GS_R_PRMODECONT *)&prim_work[1]), 1);
	}

	gs_setR_PRMODE(((GS_R_PRMODE *)&prim_work[2]), iip,tme,fge,abe,aa1,fst,ctxt,fix);

	SyncDCache(prim_work, prim_work+3*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],3);
	gs_dma_wait();

	return 0;
}

void GsEnableDithering(unsigned char enable, int mode)
{
	if(enable)
	{
		gs_setGIF_TAG(((GS_GIF_TAG	*)&prim_work[0]), 3,1,0,0,0,0,1,0x0e);
		gs_setR_DTHE(((GS_R_DTHE	*)&prim_work[1]), 1);
		gs_setR_DIMX(((GS_R_DIMX	*)&prim_work[2]), 4,2,5,3,0,6,1,7,5,3,4,2,1,7,0,6); //thanks to:"Maximus32" on ps2dev.org
		gs_setR_COLCLAMP(((GS_R_COLCLAMP *)&prim_work[3]), 1);
	}
	else
	{
		gs_setGIF_TAG(((GS_GIF_TAG	*)&prim_work[0]), 3,1,0,0,0,0,1,0x0e);
		gs_setR_DTHE(((GS_R_DTHE	*)&prim_work[1]), 0);
		gs_setR_DIMX(((GS_R_DIMX	*)&prim_work[2]), 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
		gs_setR_NOP (((GS_R_NOP		*)&prim_work[3])); // NOP to replace clamp
	}

	SyncDCache(prim_work, prim_work+4*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],4);
	gs_dma_wait();
}

void GsEnableAlphaTransparency1(unsigned short enable,unsigned short method,unsigned char alpha_ref,unsigned short fail_method)
{
	GSGLOBAL_TEST1.atest_enable		 = enable;
	GSGLOBAL_TEST1.atest_method		 = method;
	GSGLOBAL_TEST1.atest_reference	 = alpha_ref;
	GSGLOBAL_TEST1.atest_fail_method = fail_method;


	GsSetPixelTest1(GSGLOBAL_TEST1.atest_enable	, GSGLOBAL_TEST1.atest_method, GSGLOBAL_TEST1.atest_reference, GSGLOBAL_TEST1.atest_fail_method,
					GSGLOBAL_TEST1.datest_enable, GSGLOBAL_TEST1.datest_mode,
					GSGLOBAL_TEST1.ztest_enable,  GSGLOBAL_TEST1.ztest_method);

	// tell GS in 16bit texture 1=transparent,0=solid
	gs_setGIF_TAG(((GS_GIF_TAG		*)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEXA(((GS_R_TEXA		*)&prim_work[1]), 0x80, 0, 0x00);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();
}

void GsEnableAlphaTransparency2(unsigned short enable,unsigned short method,unsigned char alpha_ref,unsigned short fail_method)
{
	GSGLOBAL_TEST2.atest_enable		 = enable;
	GSGLOBAL_TEST2.atest_method		 = method;
	GSGLOBAL_TEST2.atest_reference	 = alpha_ref;
	GSGLOBAL_TEST2.atest_fail_method = fail_method;

	GsSetPixelTest2(GSGLOBAL_TEST2.atest_enable	, GSGLOBAL_TEST2.atest_method, GSGLOBAL_TEST2.atest_reference, GSGLOBAL_TEST2.atest_fail_method,
					GSGLOBAL_TEST2.datest_enable, GSGLOBAL_TEST2.datest_mode,
					GSGLOBAL_TEST2.ztest_enable,  GSGLOBAL_TEST2.ztest_method);

	// tell GS in 16bit texture 1=transparent,0=solid
	gs_setGIF_TAG(((GS_GIF_TAG		*)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEXA(((GS_R_TEXA		*)&prim_work[1]), 0x80, 0, 0x00);

	SyncDCache(prim_work, prim_work+2*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],2);
	gs_dma_wait();
}

void GsEnableZbuffer1(unsigned short enable,unsigned short test_method)
{
	GSGLOBAL_TEST1.ztest_enable = enable;
	GSGLOBAL_TEST1.ztest_method	= test_method;

	GsSetPixelTest1(GSGLOBAL_TEST1.atest_enable	, GSGLOBAL_TEST1.atest_method, GSGLOBAL_TEST1.atest_reference, GSGLOBAL_TEST1.atest_fail_method,
					GSGLOBAL_TEST1.datest_enable, GSGLOBAL_TEST1.datest_mode,
					GSGLOBAL_TEST1.ztest_enable,  GSGLOBAL_TEST1.ztest_method);
}

void GsEnableZbuffer2(unsigned short enable,unsigned short test_method)
{
	GSGLOBAL_TEST2.ztest_enable = enable;
	GSGLOBAL_TEST2.ztest_method	= test_method;


	GsSetPixelTest2(GSGLOBAL_TEST2.atest_enable	, GSGLOBAL_TEST2.atest_method, GSGLOBAL_TEST2.atest_reference, GSGLOBAL_TEST2.atest_fail_method,
					GSGLOBAL_TEST2.datest_enable, GSGLOBAL_TEST2.datest_mode,
					GSGLOBAL_TEST2.ztest_enable,  GSGLOBAL_TEST2.ztest_method);

}

int GsGifPacketsClear(GS_PACKET_TABLE *table)
{
	table->packet_offset=0;
	table->qword_offset=0;

	return 0;
}

void GsEnableAlphaBlending1(unsigned short enable, unsigned short mode)
{
	gs_setGIF_TAG(((GS_GIF_TAG		*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);

	if(enable)
	{
		gs_setR_ALPHA_1(((GS_R_ALPHA	*)&prim_work[1]), 0,1,0,1,0x00); //last arg(0x00) is not used bceause of other settings
	}
	else
	{
		gs_setR_ALPHA_1(((GS_R_ALPHA	*)&prim_work[1]), 0,0,0,0,0x80);
	}
	gs_setR_PABE(((GS_R_PABE		*)&prim_work[2]), 0);
	gs_setR_FBA_1(((GS_R_FBA		*)&prim_work[3]), 0);
	gs_setR_TEXA(((GS_R_TEXA		*)&prim_work[4]), 0x80, 0, 0x00);

	SyncDCache(prim_work, prim_work+5*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],5);
	gs_dma_wait();
}

void GsEnableAlphaBlending2(unsigned short enable, unsigned short mode)
{
	gs_setGIF_TAG(((GS_GIF_TAG		*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);

	if(enable)
	{
		gs_setR_ALPHA_2(((GS_R_ALPHA	*)&prim_work[1]), 0,1,0,1,0x00); //last arg(0x00) is not used bceause of other settings
	}
	else
	{
		gs_setR_ALPHA_2(((GS_R_ALPHA	*)&prim_work[1]), 0,0,0,0,0x80);
	}
	gs_setR_PABE(((GS_R_PABE		*)&prim_work[2]), 0);
	gs_setR_FBA_2(((GS_R_FBA		*)&prim_work[3]), 0);
	gs_setR_TEXA(((GS_R_TEXA		*)&prim_work[4]), 0x80, 0, 0x00);

	SyncDCache(prim_work, prim_work+5*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],5);
	gs_dma_wait();
}

QWORD *GsGifPacketsAlloc(GS_PACKET_TABLE *table, unsigned int num_qwords)
{
	void	*pointer;
	GS_GIF_PACKET *packet;

	if(num_qwords <= (GS_PACKET_DATA_QWORD_MAX-table->qword_offset))			//check if we can alloc in current packet
	{
		pointer=&table->packets[table->packet_offset].data[table->qword_offset];
		table->qword_offset += num_qwords;
	}
	else	//couldnt fit so we going to have to try to jump to next packet
	{
		if(table->packet_offset+1 == table->packet_count) //no more packet available so we return error;
		{
			pointer=NULL;
		}
		else //there is another pocket available so we can jump to it
		{
			//before we jup to this packet then we got to end current packet and point it to the new one
			packet=(GS_GIF_PACKET*)UNCACHED_SEG(&table->packets[table->packet_offset]);
			packet->tag.qwc=table->qword_offset;
			packet->tag.pad1=0;
			packet->tag.pce=0;
			packet->tag.id=0x02; //next = tag.addr
			packet->tag.irq	=0;
			packet->tag.addr=(unsigned int)&((GS_GIF_PACKET*)UNCACHED_SEG(&table->packets[table->packet_offset + 1]))->tag;
			packet->tag.spr		=0;
			packet->tag.pad2	=0;

			//now reset qwords offset in this packet & update packet offset
			table->qword_offset= 0;
			table->packet_offset++;
		}

		//now we use the new packet
		// but we still got to check if enough mem is available
		if( num_qwords <= (GS_PACKET_DATA_QWORD_MAX-table->qword_offset) )
		{
			pointer=&table->packets[table->packet_offset].data[table->qword_offset];
			table->qword_offset += num_qwords;
		}
		else //not enough
		{
			pointer=NULL;
		}
	}

	return pointer;
}

int GsGifPacketsExecute(GS_PACKET_TABLE *table, unsigned short wait)
{
	GS_GIF_PACKET *packet;

	if(table->packet_offset==0    &&   table->qword_offset==0)
		return 0;

	if(table->packets == NULL)
		return -1;

	//close the current pocket
	packet=(GS_GIF_PACKET*)UNCACHED_SEG(&table->packets[table->packet_offset]);
	packet->tag.qwc		=table->qword_offset;
	packet->tag.pad1	=0;
	packet->tag.pce		=0;
	packet->tag.id		=0x07; //end
	packet->tag.irq		=0;
	packet->tag.addr	=(unsigned int)0;
	packet->tag.spr		=0;
	packet->tag.pad2	=0;

	gs_dma_send_tag(0, 0, &table->packets[0].tag);
	if(wait)
		gs_dma_wait();

	return 0;
}

int GsDrawSync(int mode)
{
	switch(mode)
	{
	case 0:
	default:
		gs_dma_wait();
	}

	return 1;
}

int GsHSync(int mode)
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
			return 0;
		}
	}

	return 0;
}

int GsVSync(int mode)
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
			return 0;
		}
	}

	return 0;
}

int GsLoadImage(void *source_addr, GS_IMAGE *dest)
{
	int i;
	static int	current,max,remainder,img_qwc;

	switch(dest->psm)
	{
	case GS_TEX_32:	//32 bit image
		img_qwc = ((dest->width * dest->height)*4)/16;
	break;
	case GS_TEX_24:	//24 bit image
		img_qwc = ((dest->width * dest->height)*3)/16;
	break;
	case GS_TEX_16:	//16 bit image
		img_qwc = ((dest->width * dest->height)*2)/16;
	break;
	case GS_TEX_8:	//8 bit image
		img_qwc = ((dest->width * dest->height)*1)/16;
	break;
	case GS_TEX_4:	//4 bit image
		img_qwc = ((dest->width * dest->height)/2)/16;
	break;
	default:
		//printf("unable to load unsupported image(%02x)",dest->psm);
	break;
	}

	gs_setGIF_TAG(((GS_GIF_TAG				*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);
	gs_setR_BITBLTBUF(((GS_R_BITBLTBUF		*)&prim_work[1]),0,0,0,dest->vram_addr,dest->vram_width,dest->psm);
	gs_setR_TRXPOS(((GS_R_TRXPOS			*)&prim_work[2]), 0,0,dest->x,dest->y,0);
	gs_setR_TRXREG(((GS_R_TRXREG			*)&prim_work[3]), dest->width,dest->height);
	gs_setR_TRXDIR(((GS_R_TRXDIR			*)&prim_work[4]), 0);

	SyncDCache(prim_work, prim_work+5*sizeof(QWORD));
	gs_dma_send((unsigned int *)&prim_work[0],5);
	gs_dma_wait();

	// Ok , We Send Image Now
	max			= img_qwc / 16384;
	remainder	= img_qwc % 16384;
	current		= 16384;
	for(i=0;i<max;i++)
	{
		//1st we signal gs we are about to send
		//16384 qwords

		gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), current,1,0,0,0,2,0,0x00);

		SyncDCache(prim_work, prim_work+1*sizeof(QWORD));
		gs_dma_send((unsigned int *)&prim_work[0],1);
		gs_dma_wait();

		//we now send 16384 more qwords
		gs_dma_send((unsigned int *)source_addr,current);
		gs_dma_wait();

		(unsigned char *)source_addr += current*16;
	}

	//transfer the rest if we have left overs
	current = remainder;
	//or exit if none is left
	if(current)
	{
		// we signal are about to send whats left
		gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), current,1,0,0,0,2,0,0x00);

		SyncDCache(prim_work, prim_work+1*sizeof(QWORD));
		gs_dma_send((unsigned int *)&prim_work[0],1);
		gs_dma_wait();

		//send data leftover
		gs_dma_send((unsigned int *)source_addr,current);

		//dont wait
		gs_dma_wait();
	}

	return 1;
}

/*************************************************
*
*
* VRAM
*
*************************************************/
static unsigned int		vr_addr=0;
static unsigned int		vr_tex_start=0;
static unsigned int		vr_2ndtolast_alloc=0;	//address before last alloc so we can Free the last alloc

int GsVramAllocFrameBuffer(short w, short h, short psm)
{
	int size, remainder, ret, byte_pp;	// byte per pixel

	switch(psm)
	{
	case GS_PIXMODE_32:
	case GS_TEX_4HH:		// these are 32bit
	case GS_TEX_4HL:		// ..
	case GS_TEX_8H:			// ..
	case GS_ZBUFF_32:		//
	case GS_ZBUFF_24:		//
	case GS_PIXMODE_24:		// also 24bit takes up 4 bytes
		byte_pp = 4;
	break;
	case GS_PIXMODE_16:
	case GS_PIXMODE_16S:
	case GS_ZBUFF_16:
		byte_pp = 2;
	break;
	case GS_TEX_8:
		byte_pp	= 1;
	break;
	case GS_TEX_4:
		byte_pp	= 0;
	break;
	default:
		return -EINVAL;
	}

	if(byte_pp > 0)		// 8 to 32 bit
	{
		size = ((w*h)*byte_pp)/4;
	}
	else			// 4 bit
	{
		size = (w*h)/2;
	}

	remainder = (vr_addr % (2048));

	if(remainder)
		vr_addr += ((2048)-remainder);

	ret = vr_addr/(2048);
	vr_addr += size;
	vr_tex_start = vr_addr;

	return ret;
}

int GsVramAllocTextureBuffer(short w, short h, short psm)
{
	int size, remainder, ret, byte_pp;	// byte per pixel

	switch(psm)
	{
	case GS_PIXMODE_32:
	case GS_PIXMODE_24:		// also 24bit takes up 4 bytes
	case GS_TEX_8H:			// ..
	case GS_TEX_4HH:		// these are 32bit
	case GS_TEX_4HL:		// ..
		byte_pp = 4;
	break;
	case GS_PIXMODE_16:
	case GS_PIXMODE_16S:
		byte_pp = 2;
	break;
	case GS_TEX_8:
		byte_pp	= 1;
	break;
	case GS_TEX_4:
		byte_pp	= 0;
	break;
	default:
		return -EINVAL;
	}

	if(byte_pp > 0)		// 8 to 32 bit
	{
		size = ((w*h)*byte_pp)/4;
	}
	else			// 4 bit
	{
		size = (w*h)/2;
	}

	remainder = (vr_addr % (64));

	//---
	vr_2ndtolast_alloc = vr_addr;

	if(remainder)
		vr_addr += ((64)-remainder);

	ret = vr_addr/(64);
	vr_addr += size;

	return ret;
}

void GsVramFreeAllTextureBuffer(void)
{
	vr_addr = vr_tex_start;
}

/*
int VramFreeLastTextureBuffer()
{
	if(vr_2ndtolast_alloc)
	{
		vr_addr = vr_2ndtolast_alloc;
		vr_2ndtolast_alloc =0;
	}
}
*/

void GsVramFreeAll(void)
{
	vr_addr			= 0;
	vr_tex_start	= 0;
}

/*****************************************************************
***
***
*****************************************************************/

int GsDbGetDrawBuffer(void)
{
	return gs_db_draw_buffer;
}

int GsDbGetDisplayBuffer(void)
{
	return(gs_db_draw_buffer? 0: 1);
}

void GsDbSwapBuffer(void)
{
	gs_db_draw_buffer = gs_db_draw_buffer? 0: 1;
}

/*****************************************************************
*** Miscellaneous
***
*****************************************************************/

// i think this was from ito
char twh4(short val)
{
	char l=0;

	val--;
	while(val>0) val>>=1, l++;
	return (l);
}

/******************************************************************
* STATIC(PRIVATE) Miscellaneous
*
*
******************************************************************/
#define gif_chcr	0x1000a000	// GIF Channel Control Register
#define gif_madr	0x1000a010	// Transfer Address Register
#define gif_qwc		0x1000a020	// Transfer Size Register (in qwords)
#define gif_tadr	0x1000a030	// ...

 #define DMA_TAG_REFE	0x00
 #define DMA_TAG_CNT	0x01
 #define DMA_TAG_NEXT	0x02
 #define DMA_TAG_REF	0x03
 #define DMA_TAG_REFS	0x04
 #define DMA_TAG_CALL	0x05
 #define DMA_TAG_RET	0x06
 #define DMA_TAG_END	0x07

typedef struct {
	unsigned direction	:1;	// Direction
	unsigned pad1		:1; // Pad with zeros
	unsigned mode		:2;	// Mode
	unsigned asp		:2;	// Address stack pointer
	unsigned tte		:1;	// Tag trasfer enable
	unsigned tie		:1;	// Tag interrupt enable
	unsigned start_flag	:1;	// start
	unsigned pad2		:7; // Pad with more zeros
	unsigned tag		:16;// DMAtag
}DMA_CHCR;

static int gs_dma_send(unsigned int *addr, unsigned int qwords)
{
	DMA_CHCR		chcr;
	static char		spr;

	if(addr >= (unsigned int *)0x70000000 && addr <= (unsigned int *)0x70003fff)
	{
		spr = 1;
	}
	else
	{
		spr = 0;
	}

	*((volatile unsigned int *)(gif_madr)) = ( unsigned int )((( unsigned int )addr) & 0x7FFFFFFF) << 0 | (unsigned int)((spr) & 0x00000001) << 31;;

	*((volatile unsigned int *)(gif_qwc)) = qwords;

	chcr.direction	=1;
	chcr.mode		=0;
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;
	*((volatile unsigned int *)(gif_chcr)) = *(unsigned int *)&chcr;

	return 0;
}

static int gs_dma_send_tag(unsigned int *addr, unsigned int qwords, GS_GIF_DMACHAIN_TAG *tag)
{
	DMA_CHCR		chcr;
	static char		spr;

	if(addr >= (unsigned int *)0x70000000 && addr <= (unsigned int *)0x70003fff)
	{
		spr = 1;
	}
	else
	{
		spr = 0;
	}

	*((volatile unsigned int *)(gif_madr)) = ( unsigned int )((( unsigned int )addr) & 0x7FFFFFFF) << 0 | (unsigned int)((spr) & 0x00000001) << 31;;

	*((volatile unsigned int *)(gif_qwc)) = qwords;

	*((volatile unsigned int *)(gif_tadr)) = ( unsigned int )((( unsigned int )tag) & 0x7FFFFFFF) << 0 | (unsigned int)((0) & 0x00000001) << 31;;

//	*((volatile unsigned int *)(gif_tadr)) = (unsigned int)tag;

	chcr.direction	=1;
	chcr.mode		=1; //chain
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;
	*((volatile unsigned int *)(gif_chcr)) = *(unsigned int *)&chcr;

	return 0;
}

static int gs_dma_wait(void)
{
	while(*((volatile unsigned int *)(0x1000a000)) & ((unsigned int)1<<8));

	return 0;
}

