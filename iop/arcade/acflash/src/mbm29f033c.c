/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acflash_internal.h"

static int flash_erase_0(flash_addr_t addr);
static int flash_program_0(flash_addr_t addr, const flash_data_t *buf, int size);
static int flash_reset_0(flash_addr_t addr);
static int flash_status_0(flash_addr_t addr);

static struct flash_ops ops_22 = {
	"Fujitsu 29F033C", 131072u, 64u, 0, &flash_erase_0, &flash_program_0, &flash_reset_0, &flash_status_0};

static void flash_delay(int x)
{
	int v1;

	v1 = x - 1;
	while ( v1-- >= 0 )
		;
}

static int flash_erase_0(flash_addr_t addr)
{
	int count;
	int pass;
	int i;
	unsigned int v8;
	int v9;

	count = 0;
	pass = 0;
	i = 1;
	*(volatile acUint16 *)addr = 0xAAAA;
	*(volatile acUint16 *)addr = 0x5555;
	*(volatile acUint16 *)addr = 0x8080;
	*(volatile acUint16 *)addr = 0xAAAA;
	*(volatile acUint16 *)addr = 0x5555;
	*(volatile acUint16 *)addr = 0x3030;
	while ( i >= 0 )
	{
		int threshold;
		int shift;
		acUint16 tmp_var;

		threshold = 200;
		shift = 8 * i;
		tmp_var = *(volatile acUint16 *)addr >> (8 * i);
		do
		{
			v8 = tmp_var ^ (*(volatile acUint16 *)addr >> (shift & 0xFF));
			tmp_var = *(volatile acUint16 *)addr >> (shift & 0xFF);
			if ( ((v8 >> 6) & 1) == 0 )
				break;
			if ( threshold <= 0 )
				DelayThread(200);
			else
				--threshold;
			++count;
		} while ( ((uiptr)tmp_var & 0x20) == 0 );
		if ( ((tmp_var ^ ((*(volatile acUint16 *)addr >> (shift & 0xFF)) & 0xFF)) & 0x40) == 0 )
			++pass;
		--i;
	}
	v9 = -116;
	if ( pass == 2 )
		v9 = count;
	if ( v9 < 0 )
		return -116;
	return 0x20000;
}

static int flash_program_0(flash_addr_t addr, const flash_data_t *buf, int size)
{
	int v4;
	int rest;
	acUint8 d;
	int threshold;
	int v15;
	acUint8 x;

	v4 = size;
	rest = size;
	while ( 1 )
	{
		int count;
		int pass;
		int i;
		flash_data_t data;
		int v11;
		int shift;
		int tmp_v12;

		if ( rest <= 0 )
			return v4 - rest;
		count = 0;
		pass = 0;
		i = 1;
		data = *buf;
		*(volatile acUint16 *)addr = 0xAAAA;
		*(volatile acUint16 *)addr = 0x5555;
		*(volatile acUint16 *)addr = 0xA0A0;
		v11 = data;
		*(volatile acUint16 *)addr = data;
		shift = 8;
		while ( i >= 0 )
		{
			d = (v11 >> (shift & 0xFF)) & 0xFF;
			threshold = 200;
			v15 = (v11 >> (shift & 0xFF)) & 0xFF;
			do
			{
				x = ((int)*(volatile acUint16 *)addr >> (shift & 0xFF)) & 0xFF;
				if ( v15 == x )
					break;
				if ( threshold <= 0 )
				{
					DelayThread(200);
				}
				else
				{
					--threshold;
				}
				++count;
			} while ( (x & 0x20) == 0 );
			if ( d == (((int)*(volatile acUint16 *)addr >> (shift & 0xFF)) & 0xFF) )
				++pass;
			shift = 8 * --i;
		}
		tmp_v12 = -116;
		if ( pass == 2 )
			tmp_v12 = count;
		if ( tmp_v12 < 0 )
			return rest - v4;
		++buf;
		rest -= 2;
	}
}

static int flash_reset_0(flash_addr_t addr)
{
	*(acUint16 *)addr = 0xF0F0;
	flash_delay(100);
	return 0x800000;
}

static int flash_status_0(flash_addr_t addr)
{
	int pass;
	int i;

	(void)addr;
	pass = 0;
	for ( i = 1; i >= 0; --i )
		++pass;
	if ( pass == 2 )
		return 1;
	return 2;
}

flash_ops_t flash_probe_mbm29f033c(flash_addr_t addr)
{
	int vendor;
	int device;

	flash_reset_0(addr);
	*(volatile acUint16 *)addr = 0xAAAA;
	*(volatile acUint16 *)addr = 0x5555;
	*(volatile acUint16 *)addr = 0x9090;
	DelayThread(500);
	vendor = *(volatile acUint16 *)addr;
	device = *(volatile acUint16 *)(addr + 2);
	flash_reset_0(addr);
	// cppcheck-suppress knownConditionTrueFalse
	if ( vendor != 0x404 || device != 0xD4D4 )
	{
		return 0;
	}
	return &ops_22;
}
