/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kernel.h>
#include <loadfile.h>
#include <iopcontrol.h>
#include <sifrpc.h>
#include <errno.h>

#define GS_BGCOLOUR *((volatile unsigned long int *)0x120000E0)

#define WHITE_BG 0xFFFFFF // start main
#define CYAN_BG 0xFFFF00 // proper argc count
#define RED_BG  0x0000FF // wrong argc count
#define GREEN_BG 0x00FF00 // before SifLoadELF
#define BLUE_BG 0xFF0000 // after SifLoadELF
#define YELLOW_BG 0x00FFFF // good SifLoadELF return
#define MAGENTA_BG 0xFF00FF // wrong SifLoadELF return
#define BROWN_BG 0x2A2AA5  // after reset IOP
#define PURPBLE_BG 0x800080  // before ExecPS2


//--------------------------------------------------------------
// Redefinition of init/deinit libc:
//--------------------------------------------------------------
// DON'T REMOVE is for reducing binary size. 
// These funtios are defined as weak in /libc/src/init.c
//--------------------------------------------------------------
   void _ps2sdk_libc_init() {}
   void _ps2sdk_libc_deinit() {}

   DISABLE_PATCHED_FUNCTIONS();
   DISABLE_EXTRA_TIMERS_FUNCTIONS();

//--------------------------------------------------------------
//Start of function code:
//--------------------------------------------------------------
// Clear user memory
// PS2Link (C) 2003 Tord Lindstrom (pukko@home.se)
//         (C) 2003 adresd (adresd_ps2dev@yahoo.com)
//--------------------------------------------------------------
static void wipeUserMem(void)
{
	int i;
	for (i = 0x100000; i < GetMemorySize(); i += 64) {
		asm volatile(
			"\tsq $0, 0(%0) \n"
			"\tsq $0, 16(%0) \n"
			"\tsq $0, 32(%0) \n"
			"\tsq $0, 48(%0) \n" ::"r"(i));
	}
}

//--------------------------------------------------------------
//End of func:  void wipeUserMem(void)
//--------------------------------------------------------------
// *** MAIN ***
// 
//--------------------------------------------------------------
int main(int argc, char *argv[])
{
	static t_ExecData elfdata;
	int ret, i;

	elfdata.epc = 0;

	GS_BGCOLOUR = WHITE_BG;
	// arg[0] partition if exists, otherwise is ""
	// arg[1]=path to ELF
	if (argc < 2) {  
		GS_BGCOLOUR = RED_BG;
		return -EINVAL;
	}

	char *new_argv[argc - 1];
	int fullPath_length = 1 + strlen(argv[0]) + strlen(argv[1]);
	char fullPath[fullPath_length];
	strcpy(fullPath, argv[0]);
	strcat(fullPath, argv[1]);
	// final new_argv[0] is partition + path to elf
	new_argv[0] = fullPath;
	for (i = 2; i < argc; i++) {
		new_argv[i - 1] = argv[i];
	}

	GS_BGCOLOUR = CYAN_BG;

	// Initialize
	SifInitRpc(0);
	wipeUserMem();

	//Writeback data cache before loading ELF.
	FlushCache(0);
	GS_BGCOLOUR = GREEN_BG;
	SifLoadFileInit();
	ret = SifLoadElf(argv[1], &elfdata);
	SifLoadFileExit();
	GS_BGCOLOUR = BLUE_BG;
	if (ret == 0 && elfdata.epc != 0) {
		GS_BGCOLOUR = YELLOW_BG;

		// Let's reset IOP because ELF was already loaded in memory
		while(!SifIopReset(NULL, 0)){};
		while (!SifIopSync()) {};

		GS_BGCOLOUR = RED_BG;

        SifInitRpc(0);
        // Load modules.
        SifLoadFileInit();
        SifLoadModule("rom0:SIO2MAN", 0, NULL);
        SifLoadModule("rom0:MCMAN", 0, NULL);
        SifLoadModule("rom0:MCSERV", 0, NULL);
        SifLoadFileExit();
        SifExitRpc();

		GS_BGCOLOUR = BROWN_BG;

		FlushCache(0);
		FlushCache(2);

		GS_BGCOLOUR = PURPBLE_BG;
		
		return ExecPS2((void *)elfdata.epc, (void *)elfdata.gp, argc-1, new_argv);
	} else {
		GS_BGCOLOUR = MAGENTA_BG;
		SifExitRpc();
		return -ENOENT;
	}
}

//--------------------------------------------------------------
//End of func:  int main(int argc, char *argv[])
//--------------------------------------------------------------
//End of file:  loader.c
//--------------------------------------------------------------