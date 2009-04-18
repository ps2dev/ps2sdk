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







typedef struct
{

	int		x_pos;		// x position of sprite
	int		y_pos;		// y position of sprite

	char	x_dir;		// x direction 0=left,1=right
	char	y_dir;		// y direction 0=up,1=down;

	char	x_speed;	// speed moving in x direction
	char	y_speed;	// speed moving in y direction

	GS_RGBAQ	color;

}MAVING_SPRITE;






char	x_randspeeds[10] = {1,4,6,5,2,3,8,6,6,7};


GS_RGBAQ	randcolor[10] ={	{28 ,200,200,0x80,0.0f},
								{255,  0,128,0x80,0.0f},
								{255,128,0,0x80,0.0f},
								{255,255,255,0x80,0.0f},
								{0  ,128,255,0x80,0.0f},
								{255,255,128,0x80,0.0f},
								{128,128,255,0x80,0.0f},
								{226,137,245,0x80,0.0f},
								{172,177,210,0x80,0.0f},
								{221,180,162,0x80,0.0f}


							};