#include <gif_tags.h>

#include <gs_gp.h>
#include <gs_privileged.h>

#include <draw.h>

// Starting position of primitive
static QWORD *__prim_start = NULL;

// Amount of vertex loops
static int __vertex_loops = 0;

static float __vertex_qwords = 0;

QWORD *draw_prim_start(QWORD *q, int context, PRIMITIVE *prim, COLOR *color)
{

	// Set the primitive register in packed mode, but don't end the packet
	PACK_GIFTAG(q,GIF_SET_TAG(2,0,0,0,GIF_FLG_PACKED,1),GIF_REG_AD);
	q++;
	PACK_GIFTAG(q,GS_SET_PRIM(prim->type,prim->shading,prim->mapping,prim->fogging,
							  prim->blending,prim->antialiasing,prim->mapping_type,
							  context,prim->colorfix),GS_REG_PRIM);
	q++;
	PACK_GIFTAG(q,color->rgbaq,GIF_REG_RGBAQ);
	q++;

	// Save the position for the reglist giftag that draw_primitive_end will add
	__prim_start = q;
	q++;

	// Return the current qword pointer
	return q;

}

QWORD *draw_prim_end(QWORD *q,int nreg, unsigned long reglist, float lpq)
{

	// Determine number qwords that were needed minus the reglist giftag
	__vertex_qwords = q - __prim_start - 1;

	// Determine number of loops based on qwords used and loops per qword
	__vertex_loops = (int)(__vertex_qwords * lpq);

	// Now set the giftag of the vertex reglist chain
	__prim_start->dw[0] = GIF_SET_TAG(__vertex_loops,1,0,0,GIF_FLG_REGLIST,nreg);

	// Set the higher 64bits to the reglist
	__prim_start->dw[1] = reglist;

	// Return the current qword pointer
	return q;

}

int draw_convert_rgbq(COLOR *output, int count, VERTEXF *vertices, COLORF *colours, unsigned char alpha)
{

	int i;
	float q = 1.00f;

	// For each colour...
	for (i=0;i<count;i++)
	{

		// Calculate the Q value.
		if (vertices[i].w != 0)
		{

			q = 1 / vertices[i].w;

		}

		// Calculate the RGBA values.
		output[i].r = (int)(colours[i].r * 128.0f);
		output[i].g = (int)(colours[i].g * 128.0f);
		output[i].b = (int)(colours[i].b * 128.0f);
		output[i].a = alpha;
		output[i].q = q;

	}

	// End function.
	return 0;

}

int draw_convert_rgbaq(COLOR *output, int count, VERTEXF *vertices, COLORF *colours)
{

	int i;
	float q = 1.00f;

	// For each colour...
	for (i=0;i<count;i++)
	{

		// Calculate the Q value.
		if (vertices[i].w != 0)
		{

			q = 1 / vertices[i].w;

		}

		// Calculate the RGBA values.
		output[i].r = (int)(colours[i].r * 128.0f);
		output[i].g = (int)(colours[i].g * 128.0f);
		output[i].b = (int)(colours[i].b * 128.0f);
		output[i].a = (int)(colours[i].a * 128.0f);
		output[i].q = q;

	}

	// End function.
	return 0;

}

int draw_convert_st(TEXEL *output, int count, VERTEXF *vertices, TEXELF *coords)
{

	int i = 0;
	float q = 1.00f;

	// For each coordinate...
	for (i=0;i<count;i++)
	{

		// Calculate the Q value.
		if (vertices[i].w != 0)
		{
			q = 1 / vertices[i].w;
		}

		// Calculate the S and T values.
		output[i].s = coords[i].s * q;
		output[i].t = coords[i].t * q;

	}

	// End function.
	return 0;

}

int draw_convert_xyz(XYZ *output, float x, float y, int z, int count, VERTEXF *vertices)
{

	int i;

	int center_x;
	int center_y;

	unsigned int max_z;

	center_x = ftoi4(x);
	center_y = ftoi4(y);

	max_z = 1 << (z - 1);

	// For each colour...
	for (i=0;i<count;i++)
	{

		// Calculate the XYZ values.
		output[i].x = (short)((vertices[i].x + 1.0f) * center_x);
		output[i].y = (short)((vertices[i].y + 1.0f) * -center_y);
		output[i].z = (unsigned int)((vertices[i].z + 1.0f) * max_z);

	}

	// End function.
	return 0;

}
