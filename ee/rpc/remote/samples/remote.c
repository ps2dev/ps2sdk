/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# ReMote control Manager RPC sample
*/

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <iopcontrol.h>
#include <loadfile.h>
#ifndef USE_ROM_MODULES
#include <sbv_patches.h>
#endif
#include <debug.h>
#include <librm.h>

#ifndef USE_ROM_MODULES
extern unsigned char SIO2MAN_irx[];
extern unsigned int size_SIO2MAN_irx;

extern unsigned char RMMAN_irx[];
extern unsigned int size_RMMAN_irx;
#endif

static int VblankStartSema, VblankEndSema;

static s32 VblankStartHandler(s32 cause)
{
	iSignalSema(VblankStartSema);

	//As per the SONY documentation, call ExitHandler() at the very end of your interrupt handler.
	ExitHandler();
	return 0;
}

static s32 VblankEndHandler(s32 cause)
{
	iSignalSema(VblankEndSema);

	//As per the SONY documentation, call ExitHandler() at the very end of your interrupt handler.
	ExitHandler();
	return 0;
}

static const char *getRmStatus(u32 status)
{
	switch(status)
	{
		case RM_INIT:
			return "INITIALIZING";
		case RM_READY:
			return "READY";
		case RM_KEYPRESSED:
			return "PRESSED";
		case RM_NOREMOTE:
			return "DISCONNECTED";
		default:
			return "UNKNOWN";
	}
}

static const char *getRmButton(u32 button)
{
	switch(button)
	{
		case RM_AUDIO:
			return "AUDIO";
		case RM_SHUFFLE:
			return "SHUFFLE";
		case RM_ANGLE:
			return "ANGLE";
		case RM_PROGRAM:
			return "PROGRAM";
		case RM_SUBTITLE:
			return "SUBTITLE";
		case RM_REPEAT:
			return "REPEAT";
		case RM_SLOW_BACK:
			return "SLOW-BACK";
		case RM_SLOW_FORW:
			return "SLOW-FORWARD";
		case RM_SCAN_BACK:
			return "SCAN-BACK";
		case RM_SCAN_FORW:
			return "SCAN-FORWARD";
		case RM_ONE:
			return "ONE";
		case RM_TWO:
			return "TWO";
		case RM_THREE:
			return "THREE";
		case RM_FOUR:
			return "FOUR";
		case RM_FIVE:
			return "FIVE";
		case RM_SIX:
			return "SIX";
		case RM_SEVEN:
			return "SEVEN";
		case RM_EIGHT:
			return "EIGHT";
		case RM_NINE:
			return "NINE";
		case RM_ZERO:
			return "ZERO";
		case RM_CLEAR:
			return "CLEAR";
		case RM_TIME:
			return "TIME";
		case RM_PREV:
			return "PREV";
		case RM_NEXT:
			return "NEXT";
		case RM_ATOB:
			return "ATOB";
		case RM_PLAY:
			return "PLAY";
		case RM_PAUSE:
			return "PAUSE";
		case RM_STOP:
			return "STOP";
		case RM_DISPLAY:
			return "DISPLAY";
		case RM_TITLE:
			return "TITLE";
		case RM_MENU:
			return "MENU";
		case RM_RETURN:
			return "RETURN";
		case RM_TRIANGLE:
			return "TRIANGLE";
		case RM_SQUARE:
			return "SQUARE";
		case RM_CIRCLE:
			return "CIRCLE";
		case RM_CROSS:
			return "CROSS";
		case RM_UP:
			return "UP";
		case RM_DOWN:
			return "DOWN";
		case RM_LEFT:
			return "LEFT";
		case RM_RIGHT:
			return "RIGHT";
		case RM_ENTER:
			return "ENTER";
		case RM_L1:
			return "L1";
		case RM_L2:
			return "L2";
		case RM_L3:
			return "L3";
		case RM_R1:
			return "R1";
		case RM_R2:
			return "R3";
		case RM_START:
			return "START";
		case RM_SELECT:
			return "SELECT";
		case RM_EJECT:
			return "EJECT";
		case RM_RESET:
			return "RESET";
		case RM_NONE:
			return "---";
		default:
			return "UNKNOWN";
	}
}

int main(int argc, char *argv[])
{
	/*	Buffers for receiving input from remote controllers. A 256-byte region is required for each possible remote.
		Buffers must be each aligned to a 64-byte boundary due to how the EE data cache works. */
	static u8 rmData[256] __attribute__((aligned(64)));
	struct remote_data data, olddata;
	int startY, wrap;
	ee_sema_t sema;

	//Initialize RPC services
	SifInitRpc(0);
	SifIopReset(NULL, 0);
	while(!SifIopSync()){};
	SifInitRpc(0);

	//Initialize graphics library
	init_scr();

	scr_printf(	"Welcome to the RMMAN sample!\n");
	scr_printf(	"For this demo, the remote should be plugged into port 2.\n");
	scr_printf(	"Loading modules...\n");

	//Load modules
	SifLoadFileInit();

#ifdef USE_ROM_MODULES
	if(SifLoadModule("rom0:ADDDRV", 0, NULL) < 0)
	{
		scr_printf("Failed to load ADDDRV!\n");
		SleepThread();
	}

	if(SifLoadModule("rom1:SIO2MAN", 0, NULL) < 0)
	{
		scr_printf("Failed to load SIO2MAN!\n");
		SleepThread();
	}

	if(SifLoadModule("rom1:RMMAN", 0, NULL) < 0)
	{
		scr_printf("Failed to load RMMAN!\n");
		SleepThread();
	}
#else
	sbv_patch_enable_lmb();

	if(SifExecModuleBuffer(SIO2MAN_irx, size_SIO2MAN_irx, 0, NULL, NULL) < 0)
	{
		scr_printf("Failed to load SIO2MAN!\n");
		SleepThread();
	}
	if(SifExecModuleBuffer(RMMAN_irx, size_RMMAN_irx, 0, NULL, NULL) < 0)
	{
		scr_printf("Failed to load RMMAN!\n");
		SleepThread();
	}
#endif

	SifLoadFileExit();

	scr_printf(	"Initializing...\n");

	//Prepare semaphores, for detecting Vertical-Blanking events.
	sema.count = 0;
	sema.max_count = 1;
	sema.attr = 0;

	VblankStartSema = CreateSema(&sema);
	VblankEndSema = CreateSema(&sema);

	//Register VBlank start and end interrupt handlers.
	AddIntcHandler(INTC_VBLANK_S, &VblankStartHandler, 0);
	AddIntcHandler(INTC_VBLANK_E, &VblankEndHandler, 0);

	//Initialize the RMMAN RPC service
	RMMan_Init();
	scr_printf(	"Module version: 0x%04x\n", RMMan_GetModuleVersion());

	scr_printf(	"Opening ports...");

	/*	The remote can only be connected to slot 0 of any port (multitaps are not supported).
		For this demo, assume that the remote controller dongle is connected to
		controller port 2 (port = 1).	*/
	RMMan_Open(1, 0, rmData);

	scr_printf(	"done!\n");
	scr_printf(	"New input will be displayed here:\n");

	//Enable interrupt handlers
	_EnableIntc(INTC_VBLANK_S);
	_EnableIntc(INTC_VBLANK_E);

	//In order to preserve the messages above, preserve the current Y coordinate.
	startY = scr_getY();
	wrap = 0;

	//Erase old remote state
	memset(&olddata, 0, sizeof(olddata));

	//Enter the main loop
	while(1)
	{	//Like with PADMAN, RMMAN only sends updates once every 1/60th (NTSC) or 1/50th (PAL) second.
		//Hence, wait for a VBlank cycle (1/50th or 1/60th second).
		WaitSema(VblankStartSema);
		WaitSema(VblankEndSema);

		//Read data that RMMAN has sent.
		RMMan_Read(1, 0, &data);

		//If there was a difference, print it.
		if((olddata.status != data.status) || (olddata.button != data.button))
		{
			olddata = data;

			/*	Do not draw past the end of the screen. If this is the last line, prepare to wrap around. */
			if(scr_getY() + 1 >= 27)
				wrap = 1;

			scr_printf("\t%08x (%s)\t%08x (%s)\n", data.status, getRmStatus(data.status), data.button, getRmButton(data.button));

			if(wrap)	//From libdebug itself
			{
				scr_setXY(0, startY);
				wrap = 0;
			}
		}
	}
	
	scr_printf("Shutting down...\n");

	//Prepare for shutdown

	//Disable interrupts
	_DisableIntc(INTC_VBLANK_S);
	_DisableIntc(INTC_VBLANK_S);

	//Unregister interrupt handlers

	//Delete semaphores
	DeleteSema(VblankStartSema);
	DeleteSema(VblankEndSema);

	//Close opened ports
	RMMan_Close(1, 0);

	//Terminate RPC services
	RMMan_End();
	SifExitRpc();

	return 0;
}
