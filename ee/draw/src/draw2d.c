#include <floatlib.h>

#include <draw.h>
#include <draw2d.h>
#include <draw3d.h>

#include <gif_tags.h>

#include <gs_gp.h>

// Normal offset
#define OFFSET 2048.0f

// Leftmost/Topmost offset (2048.0f - 0.4375f + 1)
#define START_OFFSET 2047.5625f

// Bottommost/Rightmost offset (2048.0f + 0.5625f + 1)
#define END_OFFSET 2048.5625f

#define DRAW_POINT_NREG 4
#define DRAW_POINT_REGLIST \
		((u64)GIF_REG_PRIM)  <<  0 | \
		((u64)GIF_REG_RGBAQ) <<  4 | \
		((u64)GIF_REG_XYZ2)  <<  8 | \
		((u64)GIF_REG_NOP)   << 12

static char blending = 0;

void draw_enable_blending()
{
	blending = 1;
}

void draw_disable_blending()
{
	blending = 0;
}

qword_t *draw_point(qword_t *q, int context, point_t *point)
{

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_POINT_NREG),DRAW_POINT_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_POINT,0,0,0,blending,0,0,context,0);
	q->dw[1] = point->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(point->v0.x + OFFSET),ftoi4(point->v0.y + OFFSET),point->v0.z);
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

	q->dw[0] = GIF_SET_PRIM(PRIM_LINE,0,0,0,blending,0,0,context,0);
	q->dw[1] = line->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(line->v0.x + START_OFFSET),ftoi4(line->v0.y + START_OFFSET),line->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(line->v1.x + END_OFFSET),ftoi4(line->v1.y + END_OFFSET),line->v0.z);
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

	int __xi0 = ftoi4(triangle->v0.x + OFFSET);
	int __yi0 = ftoi4(triangle->v0.y + OFFSET);

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_TRIANGLE_OUT_NREG),DRAW_TRIANGLE_OUT_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_LINE_STRIP,0,0,0,blending,0,0,context,0);
	q->dw[1] = triangle->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(__xi0,__yi0,triangle->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(triangle->v1.x + OFFSET),ftoi4(triangle->v1.y + OFFSET),triangle->v0.z);
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(triangle->v2.x + OFFSET),ftoi4(triangle->v2.y + OFFSET),triangle->v0.z);
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

	q->dw[0] = GIF_SET_PRIM(PRIM_TRIANGLE,0,0,0,blending,0,0,context,0);
	q->dw[1] = triangle->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(triangle->v0.x + OFFSET),ftoi4(triangle->v0.y + OFFSET),triangle->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(triangle->v1.x + OFFSET ),ftoi4(triangle->v1.y + OFFSET),triangle->v0.z);
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(triangle->v2.x + OFFSET),ftoi4(triangle->v2.y + OFFSET),triangle->v0.z);
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

	int __xi0 = ftoi4(rect->v0.x + START_OFFSET);
	int __yi0 = ftoi4(rect->v0.y + START_OFFSET);
	int __xi1 = ftoi4(rect->v1.x + END_OFFSET);
	int __yi1 = ftoi4(rect->v1.y + END_OFFSET);

	PACK_GIFTAG(q,GIF_SET_TAG(1,0,0,0,GIF_FLG_REGLIST,DRAW_RECT_OUT_NREG),DRAW_RECT_OUT_REGLIST);
	q++;

	q->dw[0] = GIF_SET_PRIM(PRIM_LINE_STRIP,0,0,0,blending,0,0,context,0);
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

	q->dw[0] = GIF_SET_PRIM(PRIM_SPRITE,0,0,0,blending,0,0,context,0);
	q->dw[1] = rect->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_XYZ(ftoi4(rect->v0.x + START_OFFSET),ftoi4(rect->v0.y + START_OFFSET),rect->v0.z);
	q->dw[1] = GIF_SET_XYZ(ftoi4(rect->v1.x + END_OFFSET),ftoi4(rect->v1.y + END_OFFSET),rect->v0.z);
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
	q->dw[0] = GIF_SET_PRIM(PRIM_SPRITE,0,DRAW_ENABLE,0,blending,0,PRIM_MAP_UV,context,0);
	q->dw[1] = rect->color.rgbaq;
	q++;

	q->dw[0] = GIF_SET_UV(ftoi4(rect->t0.u),ftoi4(rect->t0.v));
	q->dw[1] = GIF_SET_XYZ(ftoi4(rect->v0.x + START_OFFSET),ftoi4(rect->v0.y + START_OFFSET),rect->v0.z);
	q++;

	q->dw[0] = GIF_SET_UV(ftoi4(rect->t1.u),ftoi4(rect->t1.v));
	q->dw[1] = GIF_SET_XYZ(ftoi4(rect->v1.x + END_OFFSET),ftoi4(rect->v1.y + END_OFFSET),rect->v0.z);
	q++;

	return q;

}

qword_t *draw_rect_filled_strips(qword_t *q, int context, rect_t *rect)
{

	qword_t *giftag;

	int __xi0 = ftoi4(rect->v0.x);
	int __yi0 = ftoi4(rect->v0.y + START_OFFSET);

	int __xi1 = ftoi4(rect->v1.x);
	int __yi1 = ftoi4(rect->v1.y + END_OFFSET);

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_SPRITE,0,0,0,blending,0,0,context,0),GIF_REG_PRIM);
	q++;

	PACK_GIFTAG(q,rect->color.rgbaq,GIF_REG_RGBAQ);
	q++;

	giftag = q;
	q++;

	// Fill vertex information in 32 pixel wide strips
	while(__xi0 < __xi1)
	{

		//q->dw[0] = GIF_SET_XYZ(ftoi4(__xf0 + START_OFFSET),__yi0,rect->v0.z);
		q->dw[0] = GIF_SET_XYZ(__xi0 + ftoi4(START_OFFSET),__yi0,rect->v0.z);

		// 31<<4
		__xi0 += 496;

		// Uneven...
		if (__xi0 >= __xi1)
		{
			__xi0 = __xi1;
		}

		q->dw[1] = GIF_SET_XYZ(__xi0 + ftoi4(END_OFFSET),__yi1,rect->v0.z);

		// 1<<4
		__xi0 += 16;

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
	int __xi0 = ftoi4(rect->v0.x);
	int __yi0 = ftoi4(rect->v0.y + START_OFFSET);

	int __xi1 = ftoi4(rect->v1.x);
	int __yi1 = ftoi4(rect->v1.y + END_OFFSET);
	float strip = 0.0f;

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_SPRITE,0,DRAW_ENABLE,0,blending,0,PRIM_MAP_UV,context,0),GIF_REG_PRIM);
	q++;

	PACK_GIFTAG(q,rect->color.rgbaq,GIF_REG_RGBAQ);
	q++;

	giftag = q;
	q++;

	// Fill vertex information in 32 pixel wide strips along with texel information
	while(__xi0 < __xi1)
	{

		q->dw[0] = GIF_SET_UV(ftoi4(__uf0),__vi0);
		q->dw[1] = GIF_SET_XYZ(__xi0 + ftoi4(START_OFFSET),__yi0,rect->v0.z);
		q++;

		// Due to the GS DDA, 0-32 only draws pixels 0-31
		// Due to the above, texels 0.5 - 30.5 will map 0-31

		strip += 32.0f;

		__xi0 += 496;

		if (__xi0 >= __xi1)
		{
			__xi0 = __xi1;
		}

		__uf0 = rect->t0.u + (strip - 1.5f);

		q->dw[0] = GIF_SET_UV(ftoi4(__uf0),__vi1);
		q->dw[1] = GIF_SET_XYZ(__xi0 + ftoi4(END_OFFSET),__yi1,rect->v0.z);

		__xi0 += 16;
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

	int __xi0 = ftoi4(center->v0.x + OFFSET);
	int __yi0 = ftoi4(center->v0.y + OFFSET);
	int __xi1;
	int __yi1;
	float __arc_radians;

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_LINE_STRIP,0,0,0,blending,0,0,context,0),GIF_REG_PRIM);
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

	int __xi0 = ftoi4(center->v0.x + OFFSET);
	int __yi0 = ftoi4(center->v0.y + OFFSET);
	int __xi1;
	int __yi1;
	float __arc_radians;

	// Start primitive
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;

	PACK_GIFTAG(q,GIF_SET_PRIM(PRIM_TRIANGLE_FAN,0,0,0,blending,0,0,context,0),GIF_REG_PRIM);
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
		__xi1 = (int)((cosf(__arc_radians) * radius) * 16.0f);
		__yi1 = (int)((sinf(__arc_radians) * radius) * 16.0f);
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

qword_t *draw_round_rect_filled(qword_t *q, int context, rect_t *rect)
{

	rect_t rect_center;
	rect_t rect_side;
	point_t point_corner;

	int center_width;
	int center_height;

	float corner_size = 15.0f;

	int width = rect->v1.x - rect->v0.x;
	int height = rect->v1.y - rect->v0.y;

	center_width = width - (int)(2.0f*corner_size);
	center_height = height - (int)(2.0f*corner_size);

	rect_center.color = rect->color;
	rect_side.color = rect->color;
	rect_center.v0.z = rect->v0.z;
	rect_side.v0.z = rect->v0.z;

	// Inside rectangle
	rect_center.v0.x = rect->v0.x + corner_size;
	rect_center.v0.y = rect->v0.y + corner_size;
	rect_center.v1.x = rect_center.v0.x + (float)center_width;
	rect_center.v1.y = rect_center.v0.y + (float)center_height;

	q = draw_rect_filled_strips(q,context,&rect_center);

	// sides
	// top
	rect_side.v0.x = rect_center.v0.x;
	rect_side.v0.y = rect_center.v0.y - corner_size;
	rect_side.v1.x = rect_center.v1.x;
	rect_side.v1.y = rect_center.v0.y - 1.0f;

	q = draw_rect_filled_strips(q,context,&rect_side);

	// right
	rect_side.v0.x = rect_center.v1.x + 1.0f;
	rect_side.v0.y = rect_center.v0.y;
	rect_side.v1.x = rect_center.v1.x + corner_size;
	rect_side.v1.y = rect_center.v1.y;

	q = draw_rect_filled(q,context,&rect_side);

	// bottom
	rect_side.v0.x = rect_center.v0.x;
	rect_side.v0.y = rect_center.v1.y + 1.0f;
	rect_side.v1.x = rect_center.v1.x;
	rect_side.v1.y = rect_center.v1.y + corner_size;

	q = draw_rect_filled_strips(q,context,&rect_side);

	// left
	rect_side.v0.x = rect_center.v0.x - corner_size;
	rect_side.v0.y = rect_center.v0.y;
	rect_side.v1.x = rect_center.v0.x - 1.0f;
	rect_side.v1.y = rect_center.v1.y;

	q = draw_rect_filled(q,context,&rect_side);

	// corners
	// top right
	point_corner.v0.x = rect_center.v1.x + 1.0f;
	point_corner.v0.y = rect_center.v0.y;
	point_corner.color = rect->color;

	q = draw_arc_filled(q,context,&point_corner,corner_size,270.0f,360.0f);

	// bottom right
	point_corner.v0.x = rect_center.v1.x + 1.0f;
	point_corner.v0.y = rect_center.v1.y + 1.0f;
	point_corner.color = rect->color;

	q = draw_arc_filled(q,context,&point_corner,corner_size,0.0f,90.0f);

	// bottom left
	point_corner.v0.x = rect_center.v0.x;
	point_corner.v0.y = rect_center.v1.y + 1.0f;
	point_corner.color = rect->color;

	q = draw_arc_filled(q,context,&point_corner,corner_size,90.0f,180.0f);

	// top left
	point_corner.v0.x = rect_center.v0.x;
	point_corner.v0.y = rect_center.v0.y;
	point_corner.color = rect->color;

	q = draw_arc_filled(q,context,&point_corner,corner_size,180.0f,270.0f);

	return q;

}

qword_t *draw_round_rect_outline(qword_t *q, int context, rect_t *rect)
{

	rect_t rect_center;
	line_t line_side;
	point_t point_corner;

	int center_width;
	int center_height;

	float corner_size = 15.0f;

	int width = rect->v1.x - rect->v0.x;
	int height = rect->v1.y - rect->v0.y;

	center_width = width - (int)(2.0f*corner_size);
	center_height = height - (int)(2.0f*corner_size);

	rect_center.color = rect->color;
	line_side.color = rect->color;
	rect_center.v0.z = rect->v0.z;
	line_side.v0.z = rect->v0.z;

	// Inside rectangle
	rect_center.v0.x = rect->v0.x + corner_size;
	rect_center.v0.y = rect->v0.y + corner_size;
	rect_center.v1.x = rect_center.v0.x + (float)center_width;
	rect_center.v1.y = rect_center.v0.y + (float)center_height;

	//q = draw_rect_filled_strips(q,context,&rect_center);

	// sides
	// top
	line_side.v0.x = rect_center.v0.x;
	line_side.v0.y = rect_center.v0.y - corner_size + 1.0f;
	line_side.v1.x = rect_center.v1.x;
	line_side.v1.y = rect_center.v0.y - corner_size;

	q = draw_line(q,context,&line_side);

	// right
	line_side.v0.x = rect_center.v1.x + corner_size;
	line_side.v0.y = rect_center.v0.y;
	line_side.v1.x = rect_center.v1.x + corner_size - 1.0f;
	line_side.v1.y = rect_center.v1.y;

	q = draw_line(q,context,&line_side);

	// bottom
	line_side.v0.x = rect_center.v0.x;
	line_side.v0.y = rect_center.v1.y + corner_size + 1.0f;
	line_side.v1.x = rect_center.v1.x;
	line_side.v1.y = rect_center.v1.y + corner_size;

	q = draw_line(q,context,&line_side);

	// left
	line_side.v0.x = rect_center.v0.x - corner_size;
	line_side.v0.y = rect_center.v0.y;
	line_side.v1.x = rect_center.v0.x - corner_size - 1.0f;
	line_side.v1.y = rect_center.v1.y;

	q = draw_line(q,context,&line_side);

	// corners
	// top right
	point_corner.v0.x = rect_center.v1.x;
	point_corner.v0.y = rect_center.v0.y;
	point_corner.color = rect->color;

	q = draw_arc_outline(q,context,&point_corner,corner_size,270.0f,360.0f);

	// bottom right
	point_corner.v0.x = rect_center.v1.x;
	point_corner.v0.y = rect_center.v1.y;
	point_corner.color = rect->color;

	q = draw_arc_outline(q,context,&point_corner,corner_size,0.0f,90.0f);

	// bottom left
	point_corner.v0.x = rect_center.v0.x;
	point_corner.v0.y = rect_center.v1.y;
	point_corner.color = rect->color;

	q = draw_arc_outline(q,context,&point_corner,corner_size,90.0f,180.0f);

	// top left
	point_corner.v0.x = rect_center.v0.x;
	point_corner.v0.y = rect_center.v0.y;
	point_corner.color = rect->color;

	q = draw_arc_outline(q,context,&point_corner,corner_size,180.0f,270.0f);

	return q;

}
