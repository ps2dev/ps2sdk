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
VU_CVECTOR		vu_light_ambient	={128,128,128,0x80};
float			vu_fog_near			= 25000.0f;
float			vu_fog_far			= 45000.0f;














/*EOF*/
