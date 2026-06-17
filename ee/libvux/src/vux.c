/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <libvux.h>
#include <stdlib.h>

#include "vux.h"






static const VU_MATRIX VuInitMatrix = {{
											{1.0f, 0.0f, 0.0f, 0.0f},
											{0.0f, 1.0f, 0.0f, 0.0f},
											{0.0f, 0.0f, 1.0f, 0.0f},
											{0.0f, 0.0f, 0.0f, 1.0f}
										}};
VU_MATRIX	VuWorldMatrix;
VU_MATRIX	VuViewMatrix;
VU_MATRIX	VuPrjectionMatrix;
VU_MATRIX	VuLocalScreenMatrix;

float			vu_projection;
int				vu_projection_type;			//0=vu_projection  ,   1=VuPrjectionMatrix
unsigned short	vu_offset_x;
unsigned short	vu_offset_y;
VU_FCVECTOR		vu_light_ambient;
float			vu_fog_near;
float			vu_fog_far;
float			vu_near_plane_w;
float			vu_near_plane_h;

__attribute__((constructor))
static void __VuInit_ctor(void)
{
	VuInit();
}

void VuInit(void)
{
	VuSetWorldMatrix(NULL);
	VuSetViewMatrix(NULL);
	VuSetProjectionMatrix(NULL);
	VuSetLocalScreenMatrix(NULL);
	VuSetGeometryXYOffset(2048, 2048);
	VuSetProjection(500.0f);
	VuSetAmbientLight(0.2f, 0.2f, 0.2f);
	vu_fog_near = 25000.0f;
	vu_fog_far = 45000.0f;
	VuSetProjectionNearPlaneWH(300.0f, 300.0f);
}



void VuSetGeometryXYOffset(unsigned short x, unsigned short y)
{

	vu_offset_x = x;
	vu_offset_y = y;
}




void VuSetProjection(float z)
{

	vu_projection = z;
	vu_projection_type = 0;
}




void VuSetProjectionMatrix(const VU_MATRIX *projection)
{
	if (!projection)
	{
		VuPrjectionMatrix = VuInitMatrix;
	}
	else
	{
		VuPrjectionMatrix	= *projection;
	}

	vu_projection_type	= 1; // use projection matrix
}




void VuSetProjectionType(unsigned int type)
{
	vu_projection_type = type;
}




void VuSetWorldMatrix(const VU_MATRIX *world)
{
	if (!world)
	{
		VuWorldMatrix		  = VuInitMatrix;
		return;
	}
	VuWorldMatrix =		*world;
}




void VuSetViewMatrix(const VU_MATRIX *view)
{
	if (!view)
	{
		VuViewMatrix		  = VuInitMatrix;
		return;
	}
	VuViewMatrix =		*view;
}





void VuSetLocalScreenMatrix(const VU_MATRIX *m)
{
	if (!m)
	{
		VuLocalScreenMatrix = VuInitMatrix;
		return;
	}
	VuLocalScreenMatrix = *m;
}




void VuSetProjectionNearPlaneWH(unsigned int w, unsigned int h)
{

	vu_near_plane_w = w;
	vu_near_plane_h = h;
}




void VuSetAmbientLight(float r, float g, float b)
{

	vu_light_ambient.r = r;
	vu_light_ambient.g = g;
	vu_light_ambient.b = b;
	vu_light_ambient.a = 1.0f;
}
