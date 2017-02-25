/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <kernel.h>
#include <libgs.h>

typedef struct
{
	int x_pos;		// x position of sprite
	int y_pos;		// y position of sprite

	char x_dir;		// x direction 0=left,1=right
	char y_dir;		// y direction 0=up,1=down;

	char x_speed;	// speed moving in x direction
	char y_speed;	// speed moving in y direction

	GS_RGBAQ color;
}MAVING_SPRITE;

const static char x_randspeeds[10] = {1,4,6,5,2,3,8,6,6,7};

const static GS_RGBAQ randcolor[10] ={	{28 ,200,200,0x80,0.0f},
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


#define MAX_SPRITES		1000

#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		448

static short int ScreenOffsetX, ScreenOffsetY;

#define GIF_PACKET_MAX		10

static GS_DRAWENV		draw_env;
static GS_DISPENV		disp_env;

static GS_GIF_PACKET		packets[GIF_PACKET_MAX];
static GS_PACKET_TABLE		giftable;

static int InitGraphics(void);
static int InitSprites(void);
static int MoveSprites(void);
static int DrawSprites(GS_PACKET_TABLE *table);

int main(int argc, char *argv[])
{
	InitGraphics();

	giftable.packet_count	= GIF_PACKET_MAX;
	giftable.packets	= packets;

	InitSprites();

	while(1)
	{
		GsGifPacketsClear(&giftable);		// clear the area that we are going to put the sprites/triangles/....

		MoveSprites();
		DrawSprites(&giftable);			//add stuff to the packet area

		GsDrawSync(0);
		GsVSync(0);
		GsClearDrawEnv1(&draw_env);		// clear the draw environment before we draw stuff on it
		GsGifPacketsExecute(&giftable, 1);	// set to '1' becuse we want to wait for drawing to finish. if we dont wait we will write on packets that is currently writing to the gif
	}

	return 0;
}

int InitGraphics(void)
{
	unsigned int FrameBufferVRAMAddress;

	GsResetGraph(GS_INIT_RESET, GS_INTERLACED, GS_MODE_NTSC, GS_FFMD_FIELD);

	FrameBufferVRAMAddress=GsVramAllocFrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, GS_PIXMODE_32);
	GsSetDefaultDrawEnv(&draw_env, GS_PIXMODE_32, SCREEN_WIDTH, SCREEN_HEIGHT);
	//Retrieve screen offset parameters.
	ScreenOffsetX=draw_env.offset_x;
	ScreenOffsetY=draw_env.offset_y;
	GsSetDefaultDrawEnvAddress(&draw_env, FrameBufferVRAMAddress);

	GsSetDefaultDisplayEnv(&disp_env, GS_PIXMODE_32, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	GsSetDefaultDisplayEnvAddress(&disp_env, FrameBufferVRAMAddress);

	//execute draw/display environment(s)  (context 1)
	GsPutDrawEnv1(&draw_env);
	GsPutDisplayEnv1(&disp_env);

	//Set common primitive-drawing settings (Refer to documentation on PRMODE and PRMODECONT registers).
	GsOverridePrimAttributes(GS_DISABLE, 0, 0, 0, 0, 0, 0, 0, 0);

	//Set transparency settings for context 1 (Refer to documentation on TEST and TEXA registers).
	//Alpha test = enabled, pass if >= alpha reference, alpha reference = 1, fail method = no update
	GsEnableAlphaTransparency1(GS_ENABLE, GS_ALPHA_GEQUAL, 0x01, GS_ALPHA_NO_UPDATE);
	//Enable global alpha blending
	GsEnableAlphaBlending1(GS_ENABLE);

	//set transparency settings for context 2 (Refer to documentation on TEST and TEXA registers).
	//Alpha test = enabled, pass if >= alpha reference, alpha reference = 1, fail method = no update
	GsEnableAlphaTransparency2(GS_ENABLE, GS_ALPHA_GEQUAL, 0x01, GS_ALPHA_NO_UPDATE);
	//Enable global alpha blending
	GsEnableAlphaBlending2(GS_ENABLE);

	return 0;
}

static MAVING_SPRITE sprites[MAX_SPRITES];

static int InitSprites(void)
{
	int i,j;

	for(i=0,j=0;i<MAX_SPRITES;i++,j++)
	{
		if(j>9)j=0;

		sprites[i].x_pos	=i;
		sprites[i].y_pos	=i/2;

		sprites[i].x_dir	=0;
		sprites[i].y_dir	=0;

		sprites[i].x_speed	=i/x_randspeeds[j];
		sprites[i].y_speed	=x_randspeeds[j]+1;

		if(sprites[i].x_speed<=0)sprites[i].x_speed=2;
		if(sprites[i].y_speed<=0)sprites[i].y_speed=1;


		//set color
		sprites[i].color = randcolor[j];
	}

	return 0;
}

static int MoveSprites(void)
{
	int i;

	for(i=0;i<MAX_SPRITES;i++)
	{
		// mod a little
	//	sprites[i].x_speed++;
	//	sprites[i].y_speed++;

		if(sprites[i].x_speed>10)sprites[i].x_speed		=sprites[i].x_speed-10;
		if(sprites[i].y_speed>10)sprites[i].y_speed		=sprites[i].y_speed-10;

		//x
		if(sprites[i].x_dir==0)
		{
			sprites[i].x_pos	-= sprites[i].x_speed;
			if(sprites[i].x_pos<0)
			{
				sprites[i].x_pos=0;
				sprites[i].x_dir=1;
			}
		}
		else if(sprites[i].x_dir==1)
		{
			sprites[i].x_pos	+= sprites[i].x_speed;
			if(sprites[i].x_pos>SCREEN_WIDTH)
			{
				sprites[i].x_pos = SCREEN_WIDTH;
				sprites[i].x_dir=0;
			}
		}

		//y
		if(sprites[i].y_dir==0)
		{
			sprites[i].y_pos	-= sprites[i].y_speed;
			if(sprites[i].y_pos<0)
			{
				sprites[i].y_pos=0;
				sprites[i].y_dir=1;
			}
		}
		else if(sprites[i].y_dir==1)
		{
			sprites[i].y_pos	+= sprites[i].y_speed;
			if(sprites[i].y_pos>SCREEN_HEIGHT)
			{
				sprites[i].y_pos=SCREEN_HEIGHT;
				sprites[i].y_dir=0;
			}
		}
	}

	return 0;
}

static int DrawSprites(GS_PACKET_TABLE *table)
{
	int i;
	QWORD *p;

	for(i=0;i<MAX_SPRITES;i++)
	{
		//Use the uncached segment, to avoid needing to flush the data cache.
		p = (QWORD*)UNCACHED_SEG(GsGifPacketsAlloc(table, 5)); //Allocate 5 qword for 1 untextured strite

		/*	For this GIF packet, the EOP flag is set to 1.
			Rightfully, it should only be set for only the final packet so that the GIF knows when it can safely switch paths,
			but to keep things simple, it's set to 1 for every packet.

			The packets are all in the PACKED format.	*/
		gs_setGIF_TAG(((GS_GIF_TAG	*)&p[0]), 4,1,0,0,GS_GIF_PACKED,1,gif_rd_ad);
		gs_setR_PRIM(((GS_R_PRIM	*)&p[1]), GS_PRIM_SPRITE,0, 0, 0, 1, 0, 0, 0, 0);
		gs_setR_RGBAQ(((GS_R_RGBAQ	*)&p[2]), sprites[i].color.r, sprites[i].color.g, sprites[i].color.b, sprites[i].color.a, sprites[i].color.q);
		gs_setR_XYZ2(((GS_R_XYZ		*)&p[3]), (ScreenOffsetX+sprites[i].x_pos)<<4,	(ScreenOffsetY+sprites[i].y_pos)<<4, 0x00000000);
		gs_setR_XYZ2(((GS_R_XYZ		*)&p[4]), (ScreenOffsetX+sprites[i].x_pos+10)<<4,	(ScreenOffsetY+sprites[i].y_pos+10)<<4, 0x00000000);
	}

	return 0;

}

/*EOF*/
