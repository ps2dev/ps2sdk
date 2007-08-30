#include <tamtypes.h>
#include <kernel.h>
#include <loadfile.h>
#include <stdio.h>
#include <sifrpc.h>
#include <graph_registers.h>
#include <libpad.h>
#include <string.h>

#define DEBUG_BGCOLOR(col) *((u64 *) 0x120000e0) = (u64) (col)

char* padTypeStr[] = {	"Unsupported controller", "Mouse", "Nejicon",
						"Konami Gun", "Digital", "Analog", "Namco Gun",
						"DualShock"};

static char *padBuf[2]; 
u32 portConnected[2];
u32 paddata[2];
u32 old_pad[2];
u32 new_pad[2];
u8 actDirect[2][6] = { {0,0,0,0,0,0}, {0,0,0,0,0,0}};

void wait_vsync() 
{
	// Enable the vsync interrupt.
	GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

	// Wait for the vsync interrupt.
	while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

	// Disable the vsync interrupt.
	GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

void loadmodules(int free)
{
	s32 ret;
	
	if(free == 1)
	{
		if((ret = SifLoadModule("host0:freesio2.irx", 0, NULL)) < 0)
		{
			printf("Failed to load freesio2.irx module (%d)\n", ret);
			SleepThread();
		}

		if((ret = SifLoadModule("host0:freepad.irx", 0, NULL)) < 0)
		{
			printf("Failed to load freepad.irx module (%d)\n", ret);
			SleepThread();
		}
	}
	else
	{
		if((ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL)) < 0)
		{
			printf("Failed to load XSIO2MAN module (%d)\n", ret);
			SleepThread();
		}
		
		if((ret = SifLoadModule("rom0:XPADMAN", 0, NULL)) < 0)
		{
			printf("Failed to load XPADMAN module (%d)\n", ret);
			SleepThread();
		}
	}	
}

void padWait(int port)
{
	/* Wait for request to complete. */
	while(padGetReqState(port, 0) != PAD_RSTAT_COMPLETE)
		wait_vsync();

	/* Wait for pad to be stable. */
	while(padGetState(port, 0) != PAD_STATE_STABLE)
		wait_vsync();
}

void padStartAct(int port, int act, int speed)
{
	if(actDirect[port][act] != speed)
	{
		actDirect[port][act] = speed;	
	
		padSetActDirect(port, 0, actDirect[port]);
		padWait(port);
	}
}

void padStopAct(int port, int act)
{
	padStartAct(port, act, 0);
}

int main(int argc, char **argv)
{
	u32 port;
	struct padButtonStatus buttons;
	int dualshock[2];
	int acts[2];

	SifInitRpc(0);
 
	printf("libpadx sample");
	
	if((argc == 2) && (strncmp(argv[1], "free", 4) == 0))
	{
		printf(" - Using PS2SDK freesio2.irx and freepad.irx modules.\n");
		loadmodules(1);	
	}
	else
	{
		printf(" - Using ROM XSIO2MAN and XPADMAN modules.\n");
		printf("Start this sample with 'free' as an argument to load freesio2.irx and freepad.irx\n");
		printf("Example: ps2client execee host:padx_sample.elf free\n");
		loadmodules(0);
	}


	

	padInit(0);
	
	padBuf[0] = memalign(64, 256);
	padBuf[1] = memalign(64, 256);

	old_pad[0] = 0;
	old_pad[1] = 0;	

	portConnected[0] = 0;
	portConnected[1] = 0;

	dualshock[0] = 0;
	dualshock[1] = 0;

	acts[0] = 0;
	acts[1] = 0;

	padPortOpen(0, 0, padBuf[0]);
	padPortOpen(1, 0, padBuf[1]);

	while(1)
	{	
		for(port=0; port < 2; port++)
		{
			s32 state = padGetState(port, 0);

			if((state == PAD_STATE_STABLE) && (portConnected[port] == 0)) 
			{
				u32 i;
				u8 mTable[8];
				u32 ModeCurId;
				u32 ModeCurOffs;
				u32 ModeCurExId;
				u32 ModeTableNum = padInfoMode(port, 0, PAD_MODETABLE, -1);
				
				printf("Controller (%i) connected\n", port);

				/* Check if dualshock and if so, activate analog mode */
				for(i = 0; i < ModeTableNum; i++)
					mTable[i] = padInfoMode(port, 0, PAD_MODETABLE, i);
				
				/* Works for dualshock2 */
				if((mTable[0] == 4) && (mTable[1] == 7) && (ModeTableNum == 2))
					dualshock[port] = 1;

				/* Active and lock analog mode */
				if(dualshock[port] == 1)
				{
					padSetMainMode(port, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
					padWait(port);
				}
				
				ModeCurId = padInfoMode(port, 0, PAD_MODECURID, 0);
				ModeCurOffs = padInfoMode(port, 0, PAD_MODECUROFFS, 0);
				ModeCurExId = padInfoMode(port, 0, PAD_MODECUREXID, 0);
				ModeTableNum = padInfoMode(port, 0, PAD_MODETABLE, -1);
				acts[port] = padInfoAct(port, 0, -1, 0);		

				printf("  ModeCurId      : %i (%s)\n", (int)ModeCurId, padTypeStr[ModeCurId]);
				printf("  ModeCurExId    : %i\n", (int)ModeCurExId);
				printf("  ModeTable      : ");
		
				for(i = 0; i < ModeTableNum; i++)
				{
					mTable[i] = padInfoMode(port, 0, PAD_MODETABLE, i);
					printf("%i ", (int)mTable[i]);
				}	

				printf("\n");
				printf("  ModeTableNum   : %i\n", (int)ModeTableNum);
				printf("  ModeCurOffs    : %i\n", (int)ModeCurOffs);
				printf("  NumOfAct       : %i\n", (int)acts[port]);
				printf("  PressMode      : %i\n", (int)padInfoPressMode(port, 0));

	
				if(acts[port] > 0)
				{
					u8 actAlign[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
					u32 i;

					/* Set offsets for motor parameters for SetActDirect. */
					for(i=0; i < acts[port]; i++)
						actAlign[i] = i;

					padSetActAlign(port, 0, actAlign);
					padWait(port);
				}

				printf("  EnterPressMode : %i\n", (int)padEnterPressMode(port, 0));
				padWait(port);

				printf("Ready\n");

				portConnected[port] = 1;
			}

			if((state == PAD_STATE_DISCONN) && (portConnected[port] == 1))
			{ 
				printf("Controller (%i) disconnected\n", port);
				portConnected[port] = 0;
			}

			if(portConnected[port] == 1)
			{
				s32 ret = padRead(port, 0, &buttons);

				if(ret != 0)
				{
					paddata[port] = 0xffff ^ buttons.btns;
	
					new_pad[port] = paddata[port] & ~old_pad[port];
					old_pad[port] = paddata[port];

					// Values 50 and 200 used because my controllers are worn out :-)
					if((buttons.ljoy_h <= 50) || (buttons.ljoy_h >= 200)) printf("Left Analog  X: %i\n", (int)buttons.ljoy_h);
					if((buttons.ljoy_v <= 50) || (buttons.ljoy_v >= 200)) printf("Left Analog  Y: %i\n", (int)buttons.ljoy_v);
					if((buttons.rjoy_h <= 50) || (buttons.rjoy_h >= 200)) printf("Right Analog X: %i\n", (int)buttons.rjoy_h);
					if((buttons.rjoy_v <= 50) || (buttons.rjoy_v >= 200)) printf("Right Analog Y: %i\n", (int)buttons.rjoy_v);

				
					if(new_pad[port]) printf("Controller (%i) button(s) pressed: ", (int)port);
	            	if(new_pad[port] & PAD_LEFT)		printf("LEFT ");
					if(new_pad[port] & PAD_RIGHT) 		printf("RIGHT ");
					if(new_pad[port] & PAD_UP) 			printf("UP ");		
					if(new_pad[port] & PAD_DOWN) 		printf("DOWN ");
					if(new_pad[port] & PAD_START) 		printf("START ");
					if(new_pad[port] & PAD_SELECT) 		printf("SELECT ");
					if(new_pad[port] & PAD_SQUARE) 		printf("SQUARE (Pressure: %i) ", (int)buttons.square_p);
					if(new_pad[port] & PAD_TRIANGLE)	printf("TRIANGLE (Pressure: %i) ", (int)buttons.triangle_p);
					if(new_pad[port] & PAD_CIRCLE)		printf("CIRCLE (Pressure: %i) ", (int)buttons.circle_p);
					if(new_pad[port] & PAD_CROSS)		printf("CROSS (Pressure: %i) ", (int)buttons.cross_p);
					if(new_pad[port] & PAD_L1)			
					{	
						printf("L1 (Start Little Motor) ");	
						padStartAct(port, 0, 1); 
					}
					if(new_pad[port] & PAD_L2)			
					{	
						printf("L2 (Stop Little Motor) ");	
						padStartAct(port, 0, 0); 
					}
					if(new_pad[port] & PAD_L3)			printf("L3 ");
					if(new_pad[port] & PAD_R1)
					{
						printf("R1 (Start Big Motor) ");
						padStartAct(port, 1, 255);
					}
					if(new_pad[port] & PAD_R2)			
					{
						printf("R2 (Stop Big Motor) ");
						padStopAct(port, 1);
					}
					if(new_pad[port] & PAD_R3)			printf("R3 ");

					if(new_pad[port]) printf("\n");
				}	
	
			}
		}
		wait_vsync();
	}

	return 0;
}

