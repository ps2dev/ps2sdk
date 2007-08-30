/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: pad.c 1152 2005-06-12 17:49:50Z oopo $
*/

// Based on pad sample by pukko, check the pad samples for more advanced features.

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <string.h>

#include "graph_registers.h"
#include "libpad.h"
#include "libmtap.h"

static char* padBuf[2][4];
static u32 padConnected[2][4]; // 2 ports, 4 slots
static u32 padOpen[2][4];
static u32 mtapConnected[2];
static u32 maxslot[2];


 int wait_vsync(void) 
 {

  // Enable the vsync interrupt.
  GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

  // Wait for the vsync interrupt.
  while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // Disable the vsync interrupt.
  GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

  // End function.
  return 0;

 }

void loadmodules(int free)
{
    int ret;

	if(free == 1)
	{
		ret = SifLoadModule("host0:freesio2.irx", 0, NULL);

	    if (ret < 0) {
	        printf("SifLoadModule freesio2.irx failed: %d\n", ret);
	        SleepThread();
	    }    

		ret = SifLoadModule("host0:freemtap.irx", 0, NULL);

	    if (ret < 0) {
	        printf("SifLoadModule freemtap.irx failed: %d\n", ret);
	        SleepThread();
	    }

	    ret = SifLoadModule("host0:freepad.irx", 0, NULL);

	    if (ret < 0) {
	        printf("SifLoadModule freepad.irx failed: %d\n", ret);
	        SleepThread();
	    }	

	}
	else
	{

	    ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL);

	    if (ret < 0) {
	        printf("SifLoadModule XSIO2MAN failed: %d\n", ret);
	        SleepThread();
	    }    

		ret = SifLoadModule("rom0:XMTAPMAN", 0, NULL);

	    if (ret < 0) {
	        printf("SifLoadModule XMTAPMAN failed: %d\n", ret);
	        SleepThread();
	    }

	    ret = SifLoadModule("rom0:XPADMAN", 0, NULL);

	    if (ret < 0) {
	        printf("SifLoadModule XPADMAN failed: %d\n", ret);
	        SleepThread();
	    }

	}


}

void find_controllers()
{
	u32 port, slot;
	u32 mtapcon;

	// Look for multitaps and controllers on both ports
	for(port = 0; port < 2; port++)
	{
		//if(mtapConnected[port] == 0) mtapPortOpen(port);

		mtapcon = mtapGetConnection(port);

		// if(mtapcon == 0) mtapPortClose(port);

		if((mtapcon == 1) && (mtapConnected[port] == 0))
		{
			printf("Multitap (%i) connected\n", (int)port);			
		}

		if((mtapcon == 0) && (mtapConnected[port] == 1))
		{
			printf("Multitap (%i) disconnected(int argc, char **argv)\n", (int)port);
		}

		mtapConnected[port] = mtapcon;

		// Check for multitap
		if(mtapConnected[port] == 1) 
			maxslot[port] = 4;
		else
			maxslot[port] = 1;

		// Find any connected controllers
		for(slot=0; slot < maxslot[port]; slot++)
		{
			if(padOpen[port][slot] == 0)
			{
				padOpen[port][slot] = padPortOpen(port, slot, padBuf[port][slot]);
				//if(padOpen[port][slot])
				//	printf("padOpen(%i, %i) = %i\n", port, slot, padOpen[port][slot] );					
			}		
	
			if(padOpen[port][slot] == 1)
			{

			if(padGetState(port, slot) == PAD_STATE_STABLE)
			{
				if(padConnected[port][slot] == 0)
				{
					printf("Controller (%i,%i) connected\n", (int)port, (int)slot);
				}

				padConnected[port][slot] = 1;
			}
			else
			{
				if((padGetState(port, slot) == PAD_STATE_DISCONN) && (padConnected[port][slot] == 1))
				{
					printf("Controller (%i,%i) disconnected\n", (int)port, (int)slot);
					padConnected[port][slot] = 0;
				}
			}

			}
		}

		// Close controllers when multitap is disconnected

		if(mtapConnected[port] == 0)
		{
			for(slot=1; slot < 4; slot++)
			{
				if(padOpen[port][slot] == 1)
				{
					padPortClose(port, slot);
					padOpen[port][slot] = 0;
				}	
			}
		}
	}


}





int main(int argc, char **argv)
{
	u32 i;
	
    struct padButtonStatus buttons;
    u32 paddata;
    u32 old_pad[2][4];
    u32 new_pad[2][4];
	s32 ret;
	
	SifInitRpc(0);

    printf("libmtap sample");

	if((argc == 2) && (strncmp(argv[1], "free", 4) == 0))
	{
		printf(" - Using PS2SDK freesio2.irx, freemtap.irx and freepad.irx modules.\n");
		loadmodules(1);	
	}
	else
	{
		printf(" - Using ROM XSIO2MAN, XMTAP and XPADMAN modules.\n");
		printf("Start this sample with 'free' as an argument to load freesio2.irx, freemtap.irx and freepad.irx\n");
		printf("Example: ps2client execee host:mtap_sample.elf free\n");
		loadmodules(0);
	}
	

  

	mtapInit();
    padInit(0);
	
	mtapConnected[0] = 0;
	mtapConnected[1] = 0;

	mtapPortOpen(0);
	mtapPortOpen(1);

	for(i = 0; i < 4; i++)
	{
		padConnected[0][i] = 0;
		padConnected[1][i] = 0;
		padOpen[0][i] = 0;
		padOpen[1][i] = 0;
		old_pad[0][i] = 0;
		old_pad[1][i] = 0;
		new_pad[0][i] = 0;
		new_pad[1][i] = 0;

		padBuf[0][i] = memalign(64, 256);
		padBuf[1][i] = memalign(64, 256);
	}

	while(1)
	{
		u32 port, slot;

		find_controllers();

		for(port = 0; port < 2 ; port++)
		{
			for(slot=0; slot < maxslot[port]; slot++)
			{
				if(padOpen[port][slot] && padConnected[port][slot])
				{
					ret = padRead(port, slot, &buttons);
            
   	 	    		if (ret != 0) 
					{
            			paddata = 0xffff ^ buttons.btns;
	
						new_pad[port][slot] = paddata & ~old_pad[port][slot];
            			old_pad[port][slot] = paddata;
                
						if(new_pad[port][slot]) printf("Controller (%i,%i) button(s) pressed: ", (int)port, (int)slot);

            			if(new_pad[port][slot] & PAD_LEFT)		printf("LEFT ");
						if(new_pad[port][slot] & PAD_RIGHT) 	printf("RIGHT ");
						if(new_pad[port][slot] & PAD_UP) 		printf("UP ");		
						if(new_pad[port][slot] & PAD_DOWN) 		printf("DOWN ");
						if(new_pad[port][slot] & PAD_START) 	printf("START ");
						if(new_pad[port][slot] & PAD_SELECT) 	printf("SELECT ");
						if(new_pad[port][slot] & PAD_SQUARE) 	printf("SQUARE ");
						if(new_pad[port][slot] & PAD_TRIANGLE)	printf("TRIANGLE ");
						if(new_pad[port][slot] & PAD_CIRCLE)	printf("CIRCLE ");
						if(new_pad[port][slot] & PAD_CROSS)		printf("CROSS ");
						if(new_pad[port][slot] & PAD_L1)		printf("L1 ");
						if(new_pad[port][slot] & PAD_L2)		printf("L2 ");
						if(new_pad[port][slot] & PAD_L3)		printf("L3 ");
						if(new_pad[port][slot] & PAD_R1)		printf("R1 ");
						if(new_pad[port][slot] & PAD_R2)		printf("R2 ");
						if(new_pad[port][slot] & PAD_R3)		printf("R3 ");

						if(new_pad[port][slot]) printf("\n");


					}
				}
			}
		}

		wait_vsync();
	}
    return 0;
}
