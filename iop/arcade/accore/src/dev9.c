/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accore_internal.h"

int acDev9ModuleStart(int argc, char **argv)
{
	int AcMemDelayReg;
	int AcIoDelayReg;

	(void)argc;
	(void)argv;
	AcMemDelayReg = GetAcMemDelayReg();
	SetAcMemDelayReg((AcMemDelayReg & 0xFFFFFF00) | 0x66);
	AcIoDelayReg = GetAcIoDelayReg();
	SetAcIoDelayReg((AcIoDelayReg & 0x80FFD700) | 0x6F000855);
	*((volatile acUint16 *)0xB241510C) = 0;
	*((volatile acUint16 *)0xB241511C) = 0;
	*((volatile acUint16 *)0xB2416000) = 0;
	*((volatile acUint16 *)0xB2416032) = 0;
	*((volatile acUint16 *)0xB2416036) = 0;
	*((volatile acUint16 *)0xB241603A) = 0;
	*((volatile acUint16 *)0xB2417000) = 6;
	*((volatile acUint16 *)0xB241601E) = 0;
	return 0;
}
