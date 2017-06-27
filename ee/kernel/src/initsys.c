#include "kernel.h"
#include "libosd.h"
#include "tlbfunc.h"

void _InitSys(void)
{
	InitThread();
	InitExecPS2();
	InitTLBFunctions();
}
