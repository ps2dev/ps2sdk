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




// with double buffering the DMA can be transferig data to
// the GS while you are making/calculating data for the next
// frame which speed things up. you dont have to wait on DMA to finish
// transfering data to the GS



#define	SCREEN_WIDTH			640	
#define	SCREEN_HEIGHT			448

#define SCREEN_OFFSET_X			2048 - (SCREEN_WIDTH/2)
#define SCREEN_OFFSET_Y			2048 - ((SCREEN_HEIGHT/2))

#define GIF_PACKET_MAX			4




GS_DRAWENV						draw_env[2];	// 2 display environment
GS_DISPENV						disp_env[2];	// 2 draw	 environment


GS_GIF_PACKET					packets[2][GIF_PACKET_MAX]; //we need 2 for double buffering

GS_PACKET_TABLE					packet_table[2];				//we need 2 for double buffering


int	 InitGraphics();
void SelectDisplayContext(int context_id);
void ClearDrawingContext(int context_id);
int  DrawTriangles(GS_PACKET_TABLE *table, int context_index);
void MovePoint();

int active_buffer=0; // the buffer that we are currently writiing stuff to


int main()
{
	

	InitGraphics();



	packet_table[0].packet_count	= GIF_PACKET_MAX;
	packet_table[0].packet			= &packets[0][0];

	packet_table[1].packet_count	= GIF_PACKET_MAX;
	packet_table[1].packet			= &packets[1][0];


/*
	// change Background color

	draw_env[0].bg_color.b = 255;
	draw_env[0].bg_color.r = 255;
	draw_env[0].bg_color.g = 255;

	draw_env[1].bg_color.b = 255;
	draw_env[1].bg_color.r = 255;
	draw_env[1].bg_color.g = 255;
*/	

	

	while(1)
	{
		active_buffer = GsDbGetDrawBuffer();

		GsGifPacketsClear(&packet_table[active_buffer]);		// clear the area that we are going to put the sprites/triangles/....

		

		MovePoint();
		DrawTriangles(&packet_table[active_buffer], active_buffer);				//add stuff to the packet area
	
		














		GsDrawSync(0);	//wait for the previous buffer to finish drawing
		GsVSync(0);

		//display the previous drawn buffer
		SelectDisplayContext(   GsDbGetDisplayBuffer()   ); //tell CRTC which context we want to see on our tv
		
		// clear the draw environment before we draw stuff on it
		ClearDrawingContext(active_buffer);
				
		GsGifPacketsExecute(&packet_table[active_buffer], 0); // '0' we dont have to wait because we have 2 buffers (GsDrawSync(0) will do the wait)
		GsDbSwapBuffer();
	}


	return 0;
}



















int InitGraphics()
{
	static int	env0_address;
	static int	env1_address;

	GsInit();

	GsSetVideoMode(1, 2, 0);
	GsSetCRTCSettings(CRTC_SETTINGS_DEFAULT1, 255); //display contex 1



	//alloc 2 buffers in vram
	env0_address =  GsVramAllocFrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT,   GS_PIXMODE_32);
	env1_address =  GsVramAllocFrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT,   GS_PIXMODE_32);



	/*********SETUP CONTEX 1 ENVIRONMENT*************/

	GsSetDefaultDrawEnv		  (&draw_env[0], SCREEN_OFFSET_X, SCREEN_OFFSET_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
	GsSetDefaultDrawEnvAddress(&draw_env[0], env0_address, SCREEN_WIDTH/64, GS_PIXMODE_32);

	GsSetDefaultDisplayEnv		 (&disp_env[0], 656, 36+(36/2), SCREEN_WIDTH, SCREEN_HEIGHT);
	GsSetDefaultDisplayEnvAddress(&disp_env[0], env1_address, SCREEN_WIDTH/64, GS_PIXMODE_32);


	/*********SETUP CONTEX 2 ENVIRONMENT*************/

	GsSetDefaultDrawEnv		  (&draw_env[1], SCREEN_OFFSET_X, SCREEN_OFFSET_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
	GsSetDefaultDrawEnvAddress(&draw_env[1], env1_address, SCREEN_WIDTH/64, GS_PIXMODE_32);

	GsSetDefaultDisplayEnv		 (&disp_env[1], 656, 36+(36/2), SCREEN_WIDTH, SCREEN_HEIGHT);
	GsSetDefaultDisplayEnvAddress(&disp_env[1], env0_address, SCREEN_WIDTH/64, GS_PIXMODE_32);






	//execute draw/display environment(s)  (contex 1)
	GsPutDrawEnv1	(&draw_env[0]);
	GsPutDisplayEnv1(&disp_env[0]);

	//execute draw/display environment(s)  (contex 2)
	GsPutDrawEnv2	(&draw_env[1]);
	GsPutDisplayEnv2(&disp_env[1]);

	


	//set some common stuff
	GsOverridePrimAttributes(GS_DISABLE, 0, 0, 0, 0, 0, 0, 0, 0);

	// contex 1
	GsSetPixelTest1(GS_ENABLE, GS_ALPHA_GEQUAL, 0x01, 0x00, // alpha transparent
					GS_DISABLE, 0,		 // dest alpha test
					GS_DISABLE, 0);		 // disable z buffer

	// contex 2
	GsSetPixelTest2(GS_ENABLE, GS_ALPHA_GEQUAL, 0x01, 0x00, // alpha transparent
					GS_DISABLE, 0,		 // dest alpha test
					GS_DISABLE, 0);		 // disable z buffer



	return 0;
}












void SelectDisplayContext(int context_id)
{
	// the CRTC is used to select which contex we see on our TV/VGA/HDTV 


	if(context_id==0)
		GsSetCRTCSettings(CRTC_SETTINGS_DEFAULT1, 255);

	if(context_id==1)
		GsSetCRTCSettings(CRTC_SETTINGS_DEFAULT2, 255);

}


void ClearDrawingContext(int context_id)
{


	if(context_id==0)
		GsClearDrawEnv1(&draw_env[0]);		
	else
		GsClearDrawEnv2(&draw_env[1]);
	
}







int		point_x=100,point_y=100;
int		dir_x=0, dir_y=0;

int DrawTriangles(GS_PACKET_TABLE *table, int context_index)
{

	QWORD	*p;


	
	// top left
		
	p = GsGifPacketsAlloc(table, 6); 


	gs_setGIF_TAG(((GS_GIF_TAG	*)&p[0]), 5,1,0,0,0,0,1,0x0e);
	gs_setR_PRIM(((GS_R_PRIM	*)&p[1]), GS_PRIM_TRI,0, 0, 0, 1, 0, 0, context_index, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ	*)&p[2]), 200, 100, 100, 0x80, 0.0f);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[3]), (SCREEN_OFFSET_X+0)<<4,		(SCREEN_OFFSET_Y+0)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[4]), (SCREEN_OFFSET_X+200)<<4,		(SCREEN_OFFSET_Y+0)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[5]), (SCREEN_OFFSET_X+point_x)<<4,	(SCREEN_OFFSET_Y+point_y)<<4, 0x00000000);




	//top right
	p = GsGifPacketsAlloc(table, 6); 


	gs_setGIF_TAG(((GS_GIF_TAG	*)&p[0]), 5,1,0,0,0,0,1,0x0e);
	gs_setR_PRIM(((GS_R_PRIM	*)&p[1]), GS_PRIM_TRI,0, 0, 0, 1, 0, 0, context_index, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ	*)&p[2]), 100, 200, 100, 0x80, 0.0f);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[3]), (SCREEN_OFFSET_X+SCREEN_WIDTH-200)<<4,		(SCREEN_OFFSET_Y+0)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[4]), (SCREEN_OFFSET_X+SCREEN_WIDTH)<<4,		(SCREEN_OFFSET_Y+0)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[5]), (SCREEN_OFFSET_X+point_x)<<4,				(SCREEN_OFFSET_Y+point_y)<<4, 0x00000000);




	//bottom right
	p = GsGifPacketsAlloc(table, 6); 


	gs_setGIF_TAG(((GS_GIF_TAG	*)&p[0]), 5,1,0,0,0,0,1,0x0e);
	gs_setR_PRIM(((GS_R_PRIM	*)&p[1]), GS_PRIM_TRI,0, 0, 0, 1, 0, 0, context_index, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ	*)&p[2]), 100, 100, 200, 0x80, 0.0f);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[3]), (SCREEN_OFFSET_X+SCREEN_WIDTH-200)<<4,		(SCREEN_OFFSET_Y+SCREEN_HEIGHT)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[4]), (SCREEN_OFFSET_X+SCREEN_WIDTH)<<4,		(SCREEN_OFFSET_Y+SCREEN_HEIGHT)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[5]), (SCREEN_OFFSET_X+point_x)<<4,				(SCREEN_OFFSET_Y+point_y)<<4, 0x00000000);



	//bottom left
	p = GsGifPacketsAlloc(table, 6); 


	gs_setGIF_TAG(((GS_GIF_TAG	*)&p[0]), 5,1,0,0,0,0,1,0x0e);
	gs_setR_PRIM(((GS_R_PRIM	*)&p[1]), GS_PRIM_TRI,0, 0, 0, 1, 0, 0, context_index, 0);
	gs_setR_RGBAQ(((GS_R_RGBAQ	*)&p[2]), 100, 200, 200, 0x80, 0.0f);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[3]), (SCREEN_OFFSET_X+0)<<4,		(SCREEN_OFFSET_Y+SCREEN_HEIGHT)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[4]), (SCREEN_OFFSET_X+200)<<4,		(SCREEN_OFFSET_Y+SCREEN_HEIGHT)<<4, 0x00000000);
	gs_setR_XYZ2(((GS_R_XYZ		*)&p[5]), (SCREEN_OFFSET_X+point_x)<<4,	(SCREEN_OFFSET_Y+point_y)<<4, 0x00000000);


	return 0;

}








void MovePoint()
{

	if(dir_x==0)point_x -= 6;
	if(dir_x==1)point_x += 6;

	if(dir_y==0)point_y -= 8;
	if(dir_y==1)point_y += 8;



	if(point_x >SCREEN_WIDTH)
	{
		point_x= SCREEN_WIDTH;
		dir_x=0;
	}

	if(point_x <0)
	{
		point_x= 0;
		dir_x=1;
	}



	if(point_y >SCREEN_HEIGHT)
	{
		point_y= SCREEN_HEIGHT;
		dir_y=0;
	}

	if(point_y <0)
	{
		point_y= 0;
		dir_y=1;
	}


}











/*EOF*/
