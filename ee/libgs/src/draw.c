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

void GsSetDefaultDrawEnv(GS_DRAWENV *drawenv, u16 psm, u16 w, u16 h)
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

	drawenv->fbw = (w+63)/64;
	drawenv->psm = psm;
	drawenv->vram_addr = 0;
	drawenv->vram_x = 0;
	drawenv->vram_y = 0;
}

void GsSetDefaultDrawEnvAddress(GS_DRAWENV *drawenv, u16 vram_addr)
{
	drawenv->vram_addr=vram_addr;
}

int checkModelVersion(void)
{
	int fd, result, i;
	char data[256], *pData;

	if((fd=_ps2sdk_open("rom0:ROMVER", O_RDONLY))>=0)
	{
		for(pData=data,i=0; i<sizeof(data); i++)
		{
			_ps2sdk_read(fd, pData, 1);
			if(*pData++=='\0') break;
		}
		_ps2sdk_close(fd);

		//ROMVER string format: VVVVRTYYYYMMDD\n
		result=(20010608<atoi(data+i-9));
	}
	else result=-1;

	return result;
}

void GsSetDefaultDisplayEnv(GS_DISPENV *dispenv, u16 psm, u16 w, u16 h, u16 dx, u16 dy)
{
	GsGParam_t *pGParams;
	int gs_DH, gs_DW, gs_DY, gs_DX;

	pGParams=GsGetGParam();
	dispenv->disp.pad1=dispenv->disp.pad2=0;

	if(pGParams->omode >= GS_MODE_NTSC && pGParams->omode <= GS_MODE_PAL)
	{
		gs_DH=0;
		gs_DW=0;
		gs_DY=0;
		gs_DX=0;
	} else {
		if(checkModelVersion())
			_GetGsDxDyOffset(pGParams->omode, &gs_DX, &gs_DY, &gs_DW, &gs_DH);
		else{
			gs_DH=0;
			gs_DW=0;
			gs_DY=0;
			gs_DX=0;
		}
	}

	dispenv->dispfb.pad1=dispenv->dispfb.pad2=0;
	dispenv->dispfb.x=0;
	dispenv->dispfb.y=0;
	dispenv->dispfb.address=0;
	dispenv->dispfb.fbw=(w+63)/64;
	dispenv->dispfb.psm=psm;
	switch(pGParams->omode)
	{
		case GS_MODE_NTSC:
			if(pGParams->interlace)
			{
				dispenv->disp.display_y	= dy+gs_DY+0x32;
				dispenv->disp.display_x	= (gs_DX+0x27C) + dx*((w+0x9FF)/w);
				dispenv->disp.magnify_h	= (w+0x9FF)/w - 1;
				dispenv->disp.magnify_v	= 0;
				dispenv->disp.display_w	= (w+0x9FF)/w*w - 1;

				if(pGParams->ffmode)
					dispenv->disp.display_h = (h<<1) - 1;
				else
					dispenv->disp.display_h = h - 1;
			} else {
				dispenv->disp.display_h = h-1;
				dispenv->disp.display_x = gs_DX+0x27C + dx*((w+0x9FF)/w);
				dispenv->disp.display_y = gs_DY+dy+0x19;
				dispenv->disp.magnify_h = (w+0x9FF)/w - 1;
				dispenv->disp.magnify_v	= 0;
				dispenv->disp.display_w = (w+0x9FF)/w*w - 1;
			}
			break;
		case GS_MODE_PAL:
			if(pGParams->interlace)
			{
				dispenv->disp.display_y	= gs_DY+dy+0x48;
				dispenv->disp.display_x	= gs_DX+0x290 + dx*((w+0x9FF)/w);
				dispenv->disp.magnify_h	= (w+0x9FF)/w - 1;
				dispenv->disp.magnify_v	= 0;
				dispenv->disp.display_w	= (w+0x9FF)/w*w - 1;

				if(pGParams->ffmode)
					dispenv->disp.display_h = (h<<1) - 1;
				else
					dispenv->disp.display_h = h - 1;
			} else {
				dispenv->disp.display_h = h-1;
				dispenv->disp.display_x = gs_DX+0x290 + dx*((w+0x9FF)/w);
				dispenv->disp.display_y = dy+gs_DY+0x48;
				dispenv->disp.magnify_h = (w+0x9FF)/w - 1;
				dispenv->disp.magnify_v	= 0;
				dispenv->disp.display_w = (w+0x9FF)/w*w - 1;
			}
			break;
		case GS_MODE_DTV_480P:
			dispenv->disp.display_x = gs_DX+((0x2D0-w) + ((0x2D0-w)>>31))/2*2 + (dx<<1)+0xE8;
			dispenv->disp.display_h	= h-1;
			dispenv->disp.display_w	= (w<<1) - 1;
			dispenv->disp.display_y = gs_DY+dy+0x23;
			dispenv->disp.magnify_h	= 1;
			dispenv->disp.magnify_v	= 0;
			break;
		default:
			printf("GsSetDefaultDisplayEnv: Unsupported video mode: 0x%x\n", pGParams->omode);
	}
}

void GsSetDefaultDisplayEnvAddress(GS_DISPENV *dispenv, unsigned short vram_addr)
{
	dispenv->dispfb.address=vram_addr;
}

void GsPutDrawEnv1(GS_DRAWENV *drawenv)
{
	GsSetXYOffset1(drawenv->offset_x<<4, drawenv->offset_y<<4);
	GsSetScissor1(drawenv->clip.x, drawenv->clip.y, drawenv->clip.x+drawenv->clip.w, drawenv->clip.y+drawenv->clip.h);
	GsSetFrame1(drawenv->vram_addr, drawenv->fbw, drawenv->psm, drawenv->draw_mask);

	//use a sprite to clear background
	if(drawenv->auto_clear)
	{
		GsClearDrawEnv1(drawenv);
	}
}

void GsClearDrawEnv1(GS_DRAWENV *drawenv)
{
	unsigned char context;
	QWORD *p;

	// use a sprite to clear background
	context =0; // contex 1

	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_PRIM(((GS_R_PRIM *)&p[1]), GS_PRIM_SPRITE,0, 0, 0, 1, 0, 1, context, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ *)&p[2]), drawenv->bg_color.r, drawenv->bg_color.g, drawenv->bg_color.b, drawenv->bg_color.a, drawenv->bg_color.q);
	gs_setR_XYZ2(((GS_R_XYZ *)&p[3]), (drawenv->offset_x+drawenv->clip.x)<<4, (drawenv->offset_y+drawenv->clip.y)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ *)&p[4]), (drawenv->offset_x+drawenv->clip.x+drawenv->clip.w)<<4, (drawenv->offset_y+drawenv->clip.y+drawenv->clip.h)<<4, 0x00000000);

	GsDmaSend(GsPrimWorkArea, 5);
	GsDmaWait();
}

void GsPutDisplayEnv1(GS_DISPENV *dispenv)
{
	*((volatile GS_DISPLAY *)(gs_p_display1)) =  dispenv->disp;
	*((volatile GS_DISPFB *)(gs_p_dispfb1)) =  dispenv->dispfb;
}

void GsPutDrawEnv2(GS_DRAWENV *drawenv)
{
	GsSetXYOffset2(drawenv->offset_x<<4, drawenv->offset_y<<4);
	GsSetScissor2(drawenv->clip.x, drawenv->clip.y, drawenv->clip.x+drawenv->clip.w, drawenv->clip.y+drawenv->clip.h);
	GsSetFrame2(drawenv->vram_addr, drawenv->fbw, drawenv->psm, drawenv->draw_mask);

	if(drawenv->auto_clear)
	{
		GsClearDrawEnv2(drawenv);
	}
}

void GsClearDrawEnv2(GS_DRAWENV *drawenv)
{
	unsigned char context;
	QWORD *p;

	// use a sprite to clear background
	context =1; // contex 2

	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_PRIM(((GS_R_PRIM *)&p[1]), GS_PRIM_SPRITE,0, 0, 0, 1, 0, 1, context, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ *)&p[2]), drawenv->bg_color.r, drawenv->bg_color.g, drawenv->bg_color.b, drawenv->bg_color.a, drawenv->bg_color.q);
	gs_setR_XYZ2(((GS_R_XYZ *)&p[3]), (drawenv->offset_x+drawenv->clip.x)<<4, (drawenv->offset_y+drawenv->clip.y)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ *)&p[4]), (drawenv->offset_x+drawenv->clip.x+drawenv->clip.w)<<4, (drawenv->offset_y+drawenv->clip.y+drawenv->clip.h)<<4, 0x00000000);

	GsDmaSend(GsPrimWorkArea, 5);
	GsDmaWait();
}

void GsPutDisplayEnv2(GS_DISPENV *dispenv)
{
	*((volatile GS_DISPLAY *)(gs_p_display2)) =  dispenv->disp;
	*((volatile GS_DISPFB *)(gs_p_dispfb2)) =  dispenv->dispfb;
}
