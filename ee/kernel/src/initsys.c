/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "kernel.h"
#include "string.h"
#include "alarm.h"
#include "libosd.h"
#include "tlbfunc.h"

extern char **_kExecArg;

#ifdef F__InitSys
void _InitSys(void)
{
	InitAlarm();
	InitThread();
	InitExecPS2();
	InitTLBFunctions();
}
#endif

#ifdef F_TerminateLibrary
void TerminateLibrary(void)
{
	InitTLB();
}
#endif

#ifdef F__initsys_internals
char **_kExecArg;
#endif
