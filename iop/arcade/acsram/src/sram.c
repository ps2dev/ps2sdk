/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acsram_internal.h"

int acSramRead(acSramAddr addr, void *buf, int size)
{
	volatile acUint16 *src;
	int i;

	src = (volatile acUint16 *)(2 * (addr & 0x7FFF) + 0xB2500000);
	for ( i = size; i > 0; buf = (acUint8 *)buf + 1 )
	{
		--i;
		*(acUint8 *)buf = *(volatile acUint8 *)src++;
	}
	return size - i;
}

int acSramWrite(acSramAddr addr, void *buf, int size)
{
	volatile acUint16 *dst;
	int i;

	dst = (volatile acUint16 *)(2 * (addr & 0x7FFF) + 0xB2500000);
	for ( i = size; i > 0; ++dst )
	{
		--i;
		*dst = *(acUint8 *)buf;
		buf = (acUint8 *)buf + 1;
	}
	return size - i;
}

int acSramModuleStart(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return 0;
}

int acSramModuleStop()
{
	return 0;
}

int acSramModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return 0;
}

int acSramModuleStatus()
{
	return 1;
}
