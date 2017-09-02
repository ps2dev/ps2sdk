/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * initsys - basic initialization/termination functions for libkernel.
 */

#include "kernel.h"
#include "string.h"

#ifdef F__InitSys
void _InitSys(void)
{
#ifndef KERNEL_NO_PATCHES
	InitAlarm();
	InitThread();
	InitExecPS2();
	InitTLBFunctions();
#endif
}
#endif

#ifdef F_TerminateLibrary
void TerminateLibrary(void)
{
#ifndef KERNEL_NO_PATCHES
	InitTLB();
#endif
}
#endif
