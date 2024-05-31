/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <acdev.h>
#include <irx_imports.h>

#define MODNAME "arcade_device_service"
IRX_ID(MODNAME, 1, 1);

int SetAcMemDelayReg(unsigned int value)
{
	return SetDelay(10, value);
}

int GetAcMemDelayReg(void)
{
	return GetDelay(10);
}

int SetAcIoDelayReg(unsigned int value)
{
	return SetDelay(11, value);
}

int GetAcIoDelayReg(void)
{
	return GetDelay(11);
}

extern struct irx_export_table _exp_acdev;

int _start(int ac, char **av)
{
	int state;

	(void)ac;
	(void)av;
	CpuSuspendIntr(&state);
	if ( RegisterLibraryEntries(&_exp_acdev) )
	{
		CpuResumeIntr(state);
		return 1;
	}
	SetAcMemDelayReg(0x1A30FFu);
	if ( romAddDevice(1, (const void *)0xB0000000) )
		Kprintf("Arcade ROM directory not found\n");
	CpuResumeIntr(state);
	return 0;
}
