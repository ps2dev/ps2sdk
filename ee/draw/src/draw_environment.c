#include <tamtypes.h>

#include <gif_tags.h>
#include <gs_gp.h>

#include <draw.h>

// Alpha Blending Per-Pixel MSB Control
QWORD *draw_pixel_alpha_control(QWORD *q, int enable)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_PABE(enable), GS_REG_PABE);
	q++;

	return q;

}

// Alpha Blending
QWORD *draw_alpha_blending(QWORD *q, int context, BLEND *blend)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_ALPHA(blend->color1,blend->color2,blend->alpha,
								blend->color3,blend->fixed_alpha), GS_REG_ALPHA + context);
	q++;

	return q;

}

// Framebuffer Attributes
QWORD *draw_framebuffer(QWORD *q, int context, FRAMEBUFFER *frame)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_FRAME(frame->address>>11,frame->width>>6,frame->psm,frame->mask), GS_REG_FRAME + context);
	q++;

	return q;

}

// ZBuffer Attributes
QWORD *draw_zbuffer(QWORD *q, int context, ZBUFFER *zbuffer)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;
	PACK_GIFTAG(q, GS_SET_ZBUF(zbuffer->address>>11,zbuffer->zsm,zbuffer->mask), GS_REG_ZBUF + context);
	q++;

	return q;

}

// TextureBuffer Attributes
QWORD *draw_texturebuffer(QWORD *q, int context, TEXBUFFER *texbuffer, TEXTURE *texture, CLUTBUFFER *clut)
{

	if(clut->storage_mode == CLUT_STORAGE_MODE2)
	{
		clut->start = 0;
	}

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_TEX0(texbuffer->address>>6,texbuffer->width>>6,texbuffer->psm,
							   texture->width,texture->height,texture->components,texture->function,
							   clut->address>>6,clut->psm,clut->storage_mode,clut->start,clut->load_method), GS_REG_TEX0 + context);
	q++;

	return q;

}

// CLUT Storage Mode 1 Information
QWORD *draw_clutbuffer(QWORD *q, int context, int psm, CLUTBUFFER *clut)
{

	if (clut->storage_mode == CLUT_STORAGE_MODE2)
	{
		clut->start = 0;
	}

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_TEX2(psm,clut->address>>6,clut->psm,clut->storage_mode,clut->start,clut->load_method), GS_REG_TEX2 + context);
	q++;

	return q;

}

// CLUT Storage Mode 2 Information
QWORD *draw_clut_offset(QWORD *q, int cbw, int u, int v)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_TEXCLUT(cbw,u,v), GS_REG_TEXCLUT);
	q++;

	return q;

}

// Dithering Switch
QWORD *draw_dithering(QWORD *q, int enable)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_DTHE(enable), GS_REG_DTHE);
	q++;

	return q;

}

// Dithering Matrix
QWORD *draw_dither_matrix(QWORD *q,DITHERMATRIX dm)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_DIMX(dm[0], dm[1], dm[2], dm[3],
							   dm[4], dm[5], dm[6], dm[7],
							   dm[8], dm[9], dm[10],dm[11],
							   dm[12],dm[13],dm[14],dm[15]), GS_REG_DIMX);
	q++;

	return q;

}

// Fog Color
QWORD *draw_fog_color(QWORD *q, unsigned char r, unsigned char g, unsigned char b)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_FOGCOL(r,g,b), GS_REG_FOGCOL);
	q++;

	return q;

}

// Scanline Masking (framebuffer)
QWORD *draw_scan_masking(QWORD *q, int mask)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GS_SET_SCANMSK(mask),GS_REG_SCANMSK);
	q++;

	return q;

}

// Color Masking/Clamping
QWORD *draw_color_clamping(QWORD *q, int enable)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GS_SET_COLCLAMP(enable),GS_REG_COLCLAMP);
	q++;

	return q;

}

// Alpha Correction
QWORD *draw_alpha_correction(QWORD *q, int context, int alpha)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GS_SET_FBA(alpha),GS_REG_FBA + context);
	q++;

	return q;

}

// Primitive Coordinate System offset
QWORD *draw_primitive_xyoffset(QWORD *q, int context, float x, float y)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_XYOFFSET((int)(x*16.0f),(int)(y*16.0f)), GS_REG_XYOFFSET + context);
	q++;

	return q;

}

// Primitive Control
QWORD *draw_primitive_override(QWORD *q, int mode)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_PRMODECONT(mode),GS_REG_PRMODECONT);
	q++;

	return q;

}

// Overridden Primitive Attributes
QWORD *draw_primitive_override_setting(QWORD *q, int context, PRIMITIVE *prim)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_PRMODE(prim->shading,prim->mapping,prim->fogging,
								 prim->blending,prim->antialiasing,prim->mapping_type,
								 context,prim->colorfix), GS_REG_PRMODE);
	q++;

	return q;

}

// Texture Sampling, Level-of-Detail, and Filtering
QWORD *draw_texture_sampling(QWORD *q, int context, LOD *lod)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_TEX1(lod->calculation,lod->max_level,lod->mag_filter,lod->min_filter,lod->mipmap_select,lod->l,(int)(lod->k*16.0f)), GS_REG_TEX1 + context);
	q++;

	return q;

}

// Mipmap levels 1-3
QWORD *draw_mipmap1(QWORD *q, int context, MIPMAP *mipmap)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_MIPTBP1(mipmap->address1,mipmap->width1,
								  mipmap->address2,mipmap->width2,
								  mipmap->address3,mipmap->width3), GS_REG_MIPTBP1 + context);
	q++;

	return q;

}

// Mipmap levels 4-6
QWORD *draw_mipmap2(QWORD *q, int context, MIPMAP *mipmap)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_MIPTBP2(mipmap->address1,mipmap->width1,
								   mipmap->address2,mipmap->width2,
								   mipmap->address3,mipmap->width3), GS_REG_MIPTBP2 + context);
	q++;

	return q;

}

// Scissoring pixel test area
QWORD *draw_scissor_area(QWORD *q, int context, int x0, int x1, int y0, int y1)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_SCISSOR(x0,x1,y0,y1), GS_REG_SCISSOR + context);
	q++;

	return q;

}

// Pixel Testing
QWORD *draw_pixel_test(QWORD *q, int context, ALPHATEST *atest, DESTTEST *dtest, DEPTHTEST *ztest)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_TEST(atest->enable,atest->method,atest->compval,atest->keep,
							   dtest->enable,dtest->pass,
							   ztest->enable,ztest->method), GS_REG_TEST + context);
	q++;

	return q;

}

// Texture Clamping
QWORD *draw_texture_wrapping(QWORD *q, int context, WRAP *wrap)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_CLAMP(wrap->horizontal,wrap->vertical,wrap->minu,
								wrap->maxu,wrap->minv,wrap->maxv), GS_REG_CLAMP + context);
	q++;

	return q;

}

// Alpha Expansion Values
QWORD *draw_texture_expand_alpha(QWORD *q, unsigned char zero_value, int expand, unsigned char one_value)
{

	PACK_GIFTAG(q, GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
	q++;

	PACK_GIFTAG(q, GS_SET_TEXA(zero_value,expand,one_value), GS_REG_TEXA);
	q++;

	return q;

}
