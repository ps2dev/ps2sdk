/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "kernel.h"
#include "libosd.h"
#include "tlbfunc.h"

void _InitSys(void)
{
	InitThread();
	InitExecPS2();
	InitTLBFunctions();
}
