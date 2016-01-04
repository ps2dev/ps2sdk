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

int GsLoadImage(const void *source_addr, GS_IMAGE *dest)
{
	int i;
	const unsigned char *pTexSrc;
	unsigned int current, max, remainder, img_qwc;
	QWORD *p;

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
		return -1;
	}

	p=UNCACHED_SEG(GsPrimWorkArea);
	gs_setGIF_TAG(((GS_GIF_TAG*)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
	gs_setR_BITBLTBUF(((GS_R_BITBLTBUF*)&p[1]),0,0,0,dest->vram_addr,dest->vram_width,dest->psm);
	gs_setR_TRXPOS(((GS_R_TRXPOS*)&p[2]), 0,0,dest->x,dest->y,0);
	gs_setR_TRXREG(((GS_R_TRXREG*)&p[3]), dest->width,dest->height);
	gs_setR_TRXDIR(((GS_R_TRXDIR*)&p[4]), 0);

	GsDmaSend(GsPrimWorkArea, 5);
	GsDmaWait();

	// Ok , We Send Image Now
	max		= img_qwc / 16384;
	remainder	= img_qwc % 16384;
	current		= 16384;
	pTexSrc = (const unsigned char *)source_addr;
	for(i=0;i<max;i++)
	{
		//1st we signal gs we are about to send
		//16384 qwords

		gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), current,1,0,0,GS_GIF_IMAGE,0,0x00);

		GsDmaSend(GsPrimWorkArea, 1);
		GsDmaWait();

		//we now send 16384 more qwords
		GsDmaSend(pTexSrc, current);
		GsDmaWait();

		pTexSrc += current*16;
	}

	//transfer the rest if we have left overs
	current = remainder;
	//or exit if none is left
	if(current)
	{
		// we signal are about to send whats left
		gs_setGIF_TAG(((GS_GIF_TAG *)&p[0]), current,1,0,0,GS_GIF_IMAGE,0,0x00);

		GsDmaSend(GsPrimWorkArea, 1);
		GsDmaWait();

		//send data leftover
		GsDmaSend(pTexSrc, current);
		GsDmaWait();
	}

	return 1;
}

/*************************************************
*
*
* VRAM
*
*************************************************/
static unsigned int vr_addr=0;
static unsigned int vr_tex_start=0;
static unsigned int vr_2ndtolast_alloc=0;	//address before last alloc so we can Free the last alloc

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
