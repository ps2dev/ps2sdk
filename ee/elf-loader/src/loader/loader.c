/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <kernel.h>
#include <loadfile.h>
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
#define PURPBLE_BG 0x800080  // before ExecPS2


//--------------------------------------------------------------
// Redefinition of init/deinit libc:
//--------------------------------------------------------------
// DON'T REMOVE is for reducing binary size. 
// These funtios are defined as weak in /libc/src/init.c
//--------------------------------------------------------------
   void _ps2sdk_libc_init() {}
   void _ps2sdk_libc_deinit() {}

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
//--------------------------------------------------------------
int main(int argc, char *argv[])
{
	static t_ExecData elfdata;
	int ret;

	GS_BGCOLOUR = WHITE_BG;
	if (argc < 1) {  // arg1=path to ELF
		GS_BGCOLOUR = RED_BG;
		return -EINVAL;
	}
	GS_BGCOLOUR = CYAN_BG;

	// Initialize
	SifInitRpc(0);
	wipeUserMem();

	//Writeback data cache before loading ELF.
	FlushCache(0);
	GS_BGCOLOUR = GREEN_BG;
	ret = SifLoadElf(argv[0], &elfdata);
	GS_BGCOLOUR = BLUE_BG;
	if (ret == 0) {
		GS_BGCOLOUR = YELLOW_BG;
		SifExitRpc();
		FlushCache(0);
		FlushCache(2);

		GS_BGCOLOUR = PURPBLE_BG;
		// Following the standard the first parameter of a argv is the executable itself
		return ExecPS2((void *)elfdata.epc, (void *)elfdata.gp, argc, argv);
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