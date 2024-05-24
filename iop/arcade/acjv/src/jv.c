/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acjv_internal.h"

int acJvRead(acJvAddr addr, void *buf, int size)
{
	acJvReg src;
	int i;

	src = (acJvReg)(2 * (addr & 0x3FFF) + 0xB2400000);
	for ( i = size; i > 0; buf = (acUint8 *)buf + 1 )
	{
		--i;
		*(acUint8 *)buf = *src++;
	}
	return size - i;
}

int acJvWrite(acJvAddr addr, void *buf, int size)
{
	acJvReg dst;
	int i;

	dst = (acJvReg)(2 * (addr & 0x3FFF) + 0xB2400000);
	for ( i = size; i > 0; ++dst )
	{
		--i;
		*dst = *(acUint8 *)buf;
		buf = (acUint8 *)buf + 1;
	}
	return size - i;
}

int acJvGet(acJvAddr addr)
{
	return ((*(volatile acUint16 *)(2 * (addr & 0x3FFF) + 0xB2400000 + 2) << 8) & 0xFF)
			 | ((*(volatile acUint16 *)(2 * (addr & 0x3FFF) + 0xB2400000)) & 0xFF);
}

int acJvPut(acJvAddr addr, int value)
{
	volatile acUint16 *v3;

	v3 = (volatile acUint16 *)(2 * (addr & 0x3FFF) + 0xB2400000);
	*v3 = value;
	v3[1] = value >> 8;
	return value;
}

int acJvModuleStart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	*((volatile acUint16 *)0xB2416002) = 0;
	return 0;
}

int acJvModuleStop()
{
	*((volatile acUint16 *)0xB2416000) = 0;
	return 0;
}

int acJvModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}

int acJvModuleStatus()
{
	return -88;
}
