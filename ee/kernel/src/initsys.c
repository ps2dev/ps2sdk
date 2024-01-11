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
#include "timer.h"
#include "string.h"
#include <timer_alarm.h>

#ifdef F__InitSys
void _InitSys(void)
{
    InitAlarm();
    InitTimer(2);
    StartTimerSystemTime();
    InitThread();
    InitExecPS2();
    InitTLBFunctions();
}
#endif

#ifdef F_TerminateLibrary
void TerminateLibrary(void)
{
    StopTimerSystemTime();
    EndTimer();
    InitTLB();
}
#endif
