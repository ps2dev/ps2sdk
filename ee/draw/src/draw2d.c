#include <floatlib.h>

#include <draw.h>
#include <draw2d.h>
#include <draw3d.h>

#include <gif_tags.h>

#include <gs_gp.h>

#define DRAW_POINT_NREG 4
#define DRAW_POINT_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_NOP)   << 12

qword_t *draw_point(qword_t *q, int context, point_t *point)
{

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_POINT_NREG),DRAW_POINT_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_POINT,0,0,0,DRAW_ENABLE,0,0,context,0);
	q->dw[1] = point->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(point->v0.x),ftoi4(point->v0.y),point->v0.z);
	q->dw[1] = 0;
	q++;

	return q;

}

#define DRAW_LINE_NREG 4
#define DRAW_LINE_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_XYZ2)  << 12

qword_t *draw_line(qword_t *q, int context, line_t *line)
{

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_LINE_NREG),DRAW_LINE_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_LINE,0,0,0,DRAW_ENABLE,0,0,context,0);
	q->dw[1] = line->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(line->v0.x),ftoi4(line->v0.y),line->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(line->v1.x),ftoi4(line->v1.y),line->v0.z);
	q++;

	return q;

}

#define DRAW_TRIANGLE_OUT_NREG 6
#define DRAW_TRIANGLE_OUT_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_XYZ2)  << 12 | \
		((u64)GIF_REG_XYZ2)  << 16 | \
		((u64)GIF_REG_XYZ2)  << 20

qword_t *draw_triangle_outline(qword_t *q, int context, triangle_t *triangle)
{

	int __xi0 = ftoi4(triangle->v0.x);
	int __yi0 = ftoi4(triangle->v0.y);

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_TRIANGLE_OUT_NREG),DRAW_TRIANGLE_OUT_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_LINE_STRIP,0,0,0,DRAW_ENABLE,0,0,context,0);
	q->dw[1] = triangle->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(__xi0,__yi0,triangle->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(triangle->v1.x),ftoi4(triangle->v1.y),triangle->v0.z);
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(triangle->v2.x),ftoi4(triangle->v2.y),triangle->v0.z);
	q->dw[1] = GIF_SET_XYZ(__xi0,__yi0,triangle->v0.z);
	q++;


	return q;

}

#define DRAW_TRIANGLE_NREG 6
#define DRAW_TRIANGLE_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_XYZ2)  << 12 | \
		((u64)GIF_REG_XYZ2)  << 16 | \
		((u64)GIF_REG_NOP)   << 20

qword_t *draw_triangle_filled(qword_t *q, int context,triangle_t *triangle)
{

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_TRIANGLE_NREG),DRAW_TRIANGLE_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_TRIANGLE,0,0,0,DRAW_ENABLE,0,0,context,0);
	q->dw[1] = triangle->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(triangle->v0.x),ftoi4(triangle->v0.y),triangle->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(triangle->v1.x),ftoi4(triangle->v1.y),triangle->v0.z);
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(triangle->v2.x),ftoi4(triangle->v2.y),triangle->v0.z);
	q->dw[1] = 0;
	q++;

	return q;

}

#define DRAW_RECT_OUT_NREG 8
#define DRAW_RECT_OUT_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_XYZ2)  << 12 | \
		((u64)GIF_REG_XYZ2)  << 16 | \
		((u64)GIF_REG_XYZ2)  << 20 | \
		((u64)GIF_REG_XYZ2)  << 24 | \
		((u64)GIF_REG_NOP)   << 28

qword_t *draw_rect_outline(qword_t *q, int context, rect_t *rect)
{

	int __xi0 = ftoi4(rect->v0.x);
	int __yi0 = ftoi4(rect->v0.y);
	int __xi1 = ftoi4(rect->v1.x);
	int __yi1 = ftoi4(rect->v1.y);

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_RECT_OUT_NREG),DRAW_RECT_OUT_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_LINE_STRIP,0,0,0,DRAW_ENABLE,0,0,context,0);
	q->dw[1] = rect->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(__xi0,__yi0,rect->v0.z);
	q->dw[1] = GIF_SET_XYZ(__xi1,__yi0,rect->v0.z);
	q++;

	q->dw[0] = GIF_SET_XYZ(__xi1,__yi1,rect->v0.z);
	q->dw[1] = GIF_SET_XYZ(__xi0,__yi1,rect->v0.z);
	q++;

	q->dw[0] = GIF_SET_XYZ(__xi0,__yi0,rect->v0.z);
	q->dw[1] = 0;
	q++;

	return q;

}

#define DRAW_SPRITE_NREG 4
#define DRAW_SPRITE_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_XYZ2)  << 12

qword_t *draw_rect_filled(qword_t *q, int context, rect_t *rect)
{

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_SPRITE_NREG),DRAW_SPRITE_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_SPRITE,0,0,0,DRAW_ENABLE,0,0,context,0);
	q->dw[1] = rect->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(rect->v0.x),ftoi4(rect->v0.y),rect->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(rect->v1.x),ftoi4(rect->v1.y),rect->v0.z);
	q++;

	return q;

}

#define DRAW_SPRITE_TEX_NREG 6
#define DRAW_SPRITE_TEX_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_UV)    <<  8 | \
		((u64)GIF_REG_XYZ2)  << 12 | \
		((u64)GIF_REG_UV)    << 16 | \
		((u64)GIF_REG_XYZ2)  << 20

qword_t *draw_rect_textured(qword_t *q, int context, texrect_t *rect)
{

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_SPRITE_TEX_NREG),DRAW_SPRITE_TEX_REGLIST);
	q++;

	// Fill vertex information
	q->dw[0] = GIF_SET_PRIM(PRIM_SPRITE,0,DRAW_ENABLE,0,DRAW_ENABLE,0,PRIM_MAP_UV,context,0);
	q->dw[1] = rect->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_UV(ftoi4(rect->t0.u),ftoi4(rect->t0.v));
	q->dw[1] = GIF_SET_XYZ(ftoi4(rect->v0.x),ftoi4(rect->v0.y),rect->v0.z);
	q++;

	q->dw[0] = GIF_SET_UV(ftoi4(rect->t1.u),ftoi4(rect->t1.v));
	q->dw[1] = GIF_SET_XYZ(ftoi4(rect->v1.x),ftoi4(rect->v1.y),rect->v0.z);
	q++;

	return q;

}

qword_t *draw_rect_filled_strips(qword_t *q, int context, rect_t *rect)
{

	qword_t *giftag;
	float __xf0 = rect->v0.x;
	int __yi0 = ftoi4(rect->v0.y);

	// Weird thing that needs to be done to
	// remain compatible with expected GS DDA behavior
	float __xf1 = rect->v1.x + 0.9375f;
	int __yi1 = ftoi4(rect->v1.y + 0.9375f);

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_SPRITE,0,0,0,DRAW_ENABLE,0,0,context,0),GIF_REG_PRIM);
	q++;

	PACK_GIFTAG(q,rect->color.rgbaq,GIF_REG_RGBAQ);
	q++;

	giftag = q;
	q++;

	// Fill vertex information in 32 pixel wide strips
	while(__xf0 < __xf1)
	{

		q->dw[0] = GIF_SET_XYZ(ftoi4(__xf0),__yi0,rect->v0.z);

		__xf0 += 32.0f;

		q->dw[1] = GIF_SET_XYZ(ftoi4(__xf0),__yi1,rect->v0.z);
		q++;

	}

	PACK_GIFTAG(giftag,GIF_SET_TAG(q-giftag-1,0,0,0,GIF_FLG_REGLIST,2),DRAW_XYZ_REGLIST);

	return q;

}

qword_t *draw_rect_textured_strips(qword_t *q, int context, texrect_t *rect)
{

	qword_t *giftag;

	// Texel coordinates
	float __uf0 = rect->t0.u;
	int __vi0 = ftoi4(rect->t0.v);
	int __vi1 = ftoi4(rect->t1.v);

	// Primitive coordinates
	float __xf0 = rect->v0.x;
	int __yi0 = ftoi4(rect->v0.y);

	// Weird thing that needs to be done to
	// remain compatible with expected GS DDA behavior
	float __xf1 = rect->v1.x + 0.9375f;
	int __yi1 = ftoi4(rect->v1.y + 0.9375f);

	float strip = 0.0f;

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_SPRITE,0,DRAW_ENABLE,0,DRAW_ENABLE,0,PRIM_MAP_UV,context,0),GIF_REG_PRIM);
	q++;

	PACK_GIFTAG(q,rect->color.rgbaq,GIF_REG_RGBAQ);
	q++;

	giftag = q;
	q++;

	// Fill vertex information in 32 pixel wide strips along with texel information
	while(__xf0 < __xf1)
	{

		q->dw[0] = GIF_SET_UV(ftoi4(__uf0),__vi0);
		q->dw[1] = GIF_SET_XYZ(ftoi4(__xf0),__yi0,rect->v0.z);
		q++;

		// Due to the GS DDA, 0-32 only draws pixels 0-31
		// Due to the above, texels 0.5 - 30.5 will map 0-31

		strip += 32.0f;

		__xf0 = rect->v0.x + strip;
		__uf0 = strip - 1.5f;

		q->dw[0] = GIF_SET_UV(ftoi4(__uf0),__vi1);
		q->dw[1] = GIF_SET_XYZ(ftoi4(__xf0),__yi1,rect->v0.z);
		q++;

	}

	PACK_GIFTAG(giftag,GIF_SET_TAG(q-giftag-1,0,0,0,GIF_FLG_REGLIST,2),DRAW_UV_REGLIST);

	return q;

}

qword_t *draw_arc_outline(qword_t *q, int context, point_t *center, float radius, float angle_start, float angle_end)
{

	qword_t *giftag;

	u64 *dw;
	u64 pdw;

	int __xi0 = ftoi4(center->v0.x);
	int __yi0 = ftoi4(center->v0.y);
	int __xi1;
	int __yi1;
	float __arc_radians;

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_LINE_STRIP,0,0,0,DRAW_ENABLE,0,0,context,0),GIF_REG_PRIM);
	q++;

	PACK_GIFTAG(q,center->color.rgbaq,GIF_REG_RGBAQ);
	q++;

	giftag = q;
	q++;

	dw = (u64*)q;

	// Fill vertex information
	for( ; angle_start <= angle_end; angle_start++)
	{
		__arc_radians = angle_start * 0.017453293f;
		__xi1 = (int)((cosf(__arc_radians) * radius) * 16.0f);
		__yi1 = (int)((sinf(__arc_radians) * radius) * 16.0f);
		*dw++ = GIF_SET_XYZ(__xi0 + __xi1,__yi0 + __yi1,center->v0.z);
	}

	// Copy the last 64-bits if we haven't completed the last qword
	if ((u32)dw % 16)
	{

		pdw = *(dw-1);
		*dw++ = pdw;

	}

	q = (qword_t*)dw;

	PACK_GIFTAG(giftag,GIF_SET_TAG(q-giftag-1,0,0,0,GIF_FLG_REGLIST,2),DRAW_XYZ_REGLIST);

	return q;

}

qword_t *draw_arc_filled(qword_t *q, int context, point_t *center, float radius, float angle_start, float angle_end)
{

	qword_t *giftag;

	u64 *dw;
	u64 pdw;

	int __xi0 = ftoi4(center->v0.x);
	int __yi0 = ftoi4(center->v0.y);
	int __xi1;
	int __yi1;
	float __arc_radians;

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_TRIANGLE_FAN,0,0,0,DRAW_ENABLE,0,0,context,0),GIF_REG_PRIM);
	q++;

	PACK_GIFTAG(q,center->color.rgbaq,GIF_REG_RGBAQ);
	q++;

	giftag = q;
	q++;

	dw = (u64*)q;

	// Fill vertex information
	*dw++ = GIF_SET_XYZ(__xi0,__yi0,center->v0.z);

	for( ; angle_start <= angle_end; angle_start++)
	{
		__arc_radians = 0.017453293f * angle_start;
		__xi1 = (int)(cosf(__arc_radians) * radius) * 16.0f;
		__yi1 = (int)(sinf(__arc_radians) * radius) * 16.0f;
		*dw++ = GIF_SET_XYZ(__xi0 + __xi1,__yi0 + __yi1,center->v0.z);
	}

	// Copy the last vertex if we're uneven
	if ((u32)dw % 16)
	{

		pdw = *(dw-1);
		*dw++ = pdw;

	}

	q = (qword_t*)dw;

	PACK_GIFTAG(giftag,GIF_SET_TAG(q-giftag-1,0,0,0,GIF_FLG_REGLIST,2),DRAW_XYZ_REGLIST);

	return q;

}
