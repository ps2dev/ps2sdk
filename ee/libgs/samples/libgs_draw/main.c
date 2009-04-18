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

#include <libgs.h>

#include "main.h"


#define MAX_SPRITES		1000






#define	SCREEN_WIDTH			640	
#define	SCREEN_HEIGHT			448

#define SCREEN_OFFSET_X		2048 - (SCREEN_WIDTH/2)
#define SCREEN_OFFSET_Y		2048 - ((SCREEN_HEIGHT/2))

#define GIF_PACKET_MAX		10




GS_DRAWENV			draw_env;
GS_DISPENV			disp_env;


GS_GIF_PACKET		packets[GIF_PACKET_MAX];

GS_PACKET_TABLE		giftable;


int InitGraphics();
int AddSomPrims(GS_PACKET_TABLE *table);
int InitSprites();
int MoveSprites();
int DrawSprites(GS_PACKET_TABLE *table);

int main()
{
	

	
	InitGraphics();



	giftable.packet_count	= GIF_PACKET_MAX;
	giftable.packet			= &packets[0];



	

	InitSprites();

	

	

	while(1)
	{
		GsGifPacketsClear(&giftable);		// clear the area that we are going to put the sprites/triangles/....

		

		MoveSprites();
		DrawSprites(&giftable);				//add stuff to the packet area
	
		


		GsDrawSync(0);
		GsVSync(0);
		GsClearDrawEnv1(&draw_env);			// clear the draw environment before we draw stuff on it
		GsGifPacketsExecute(&giftable, 1);	// set to '1' becuse we want to wait for drawing to finish. if we dont wait we will write on packets that is currently writing to the gif
		
	}


	return 0;
}



















int InitGraphics()
{
	GsInit();

	GsSetVideoMode(1, 2, 0);
	GsSetCRTCSettings(CRTC_SETTINGS_DEFAULT1, 255); //display context 1 on tv



	GsSetDefaultDrawEnv(&draw_env, SCREEN_OFFSET_X, SCREEN_OFFSET_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
	GsSetDefaultDrawEnvAddress(&draw_env, 0, SCREEN_WIDTH/64, GS_PIXMODE_32);

	GsSetDefaultDisplayEnv(&disp_env, 656, 36+(36/2), SCREEN_WIDTH, SCREEN_HEIGHT);
	GsSetDefaultDisplayEnvAddress(&disp_env, 0, SCREEN_WIDTH/64, GS_PIXMODE_32);



	//execute draw/display environment(s)  (context 1)
	GsPutDrawEnv1(&draw_env);
	GsPutDisplayEnv1(&disp_env);

	


	//set some common stuff
	GsOverridePrimAttributes(GS_DISABLE, 0, 0, 0, 0, 0, 0, 0, 0);

	// contex 1
	GsEnableAlphaTransparency1(GS_ENABLE, GS_ALPHA_GEQUAL, 0x01, 0x00);
	// contex 2
	GsEnableAlphaTransparency2(GS_ENABLE, GS_ALPHA_GEQUAL, 0x01, 0x00);


	
	GsEnableAlphaBlending1(GS_ENABLE, 0);
	GsEnableAlphaBlending2(GS_ENABLE, 0);


	return 0;
}













MAVING_SPRITE	sprites[MAX_SPRITES];


int InitSprites()
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






int MoveSprites()
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








int DrawSprites(GS_PACKET_TABLE *table)
{
	int i;

	QWORD	*p;


	


	for(i=0;i<MAX_SPRITES;i++)
	{
		
		p = GsGifPacketsAlloc(table, 5); //alloc 5 qword for 1 untextured strite


		gs_setGIF_TAG(((GS_GIF_TAG	*)&p[0]), 4,1,0,0,0,0,1,0x0e);
		gs_setR_PRIM(((GS_R_PRIM	*)&p[1]), GS_PRIM_SPRITE,0, 0, 0, 1, 0, 0, 0, 0);
		gs_setR_RGBAQ(((GS_R_RGBAQ	*)&p[2]), sprites[i].color.r, sprites[i].color.g, sprites[i].color.b, sprites[i].color.a, sprites[i].color.q);
		gs_setR_XYZ2(((GS_R_XYZ		*)&p[3]), (SCREEN_OFFSET_X+sprites[i].x_pos)<<4,		(SCREEN_OFFSET_Y+sprites[i].y_pos)<<4, 0x00000000);
		gs_setR_XYZ2(((GS_R_XYZ		*)&p[4]), (SCREEN_OFFSET_X+sprites[i].x_pos+10)<<4,	(SCREEN_OFFSET_Y+sprites[i].y_pos+10)<<4, 0x00000000);

	}

	return 0;

}








/*EOF*/
