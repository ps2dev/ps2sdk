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

#include <libvux.h>

#include "vux.h"







VU_MATRIX	VuWorldMatrix		  =	{{
										{1.0f, 0.0f, 0.0f, 0.0f},
										{0.0f, 1.0f, 0.0f, 0.0f},
										{0.0f, 0.0f, 1.0f, 0.0f},
										{0.0f, 0.0f, 0.0f, 1.0f}
									}};

VU_MATRIX	VuViewMatrix		  =	{{
										{1.0f, 0.0f, 0.0f, 0.0f},
										{0.0f, 1.0f, 0.0f, 0.0f},
										{0.0f, 0.0f, 1.0f, 0.0f},
										{0.0f, 0.0f, 0.0f, 1.0f}
									}};

VU_MATRIX	VuPrjectionMatrix =		{{
										{1.0f, 0.0f, 0.0f, 0.0f},
										{0.0f, 1.0f, 0.0f, 0.0f},
										{0.0f, 0.0f, 1.0f, 0.0f},
										{0.0f, 0.0f, 0.0f, 1.0f}
									}};

VU_MATRIX	VuLocalScreenMatrix =	{{
										{1.0f, 0.0f, 0.0f, 0.0f},
										{0.0f, 1.0f, 0.0f, 0.0f},
										{0.0f, 0.0f, 1.0f, 0.0f},
										{0.0f, 0.0f, 0.0f, 1.0f}
									}};





float			vu_projection		= 500.0f;
int				vu_projection_type	= 0;			//0=vu_projection  ,   1=VuPrjectionMatrix
unsigned short	vu_offset_x			= 2048;
unsigned short	vu_offset_y			= 2048;
VU_FCVECTOR		vu_light_ambient	= {0.2f, 0.2f, 0.2f, 1.0f};
float			vu_fog_near			= 25000.0f;
float			vu_fog_far			= 45000.0f;
float			vu_near_plane_w		= 300.0f;
float			vu_near_plane_h		= 300.0f;








/**/

void VuInit(void)
{


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




void VuSetProjectionMatrix(VU_MATRIX *projection)
{
	VuPrjectionMatrix	= *projection;

	vu_projection_type	= 1; // use projection matrix
}




void VuSetProjectionType(unsigned int type)
{
	vu_projection_type = type;
}




void VuSetWorldMatrix(VU_MATRIX *world)
{

	VuWorldMatrix =		*world;
}




void VuSetViewMatrix(VU_MATRIX *view)
{

	VuViewMatrix =		*view;
}





void VuSetLocalScreenMatrix(VU_MATRIX *m)
{

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





/*EOF*/
