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

static int flash_erase(flash_addr_t addr);
static int flash_program(flash_addr_t addr, const flash_data_t *buf, int size);
static int flash_reset(flash_addr_t addr);
static int flash_status(flash_addr_t addr);

static struct flash_ops ops_30 = {
	"Intel 28F640J5", 131072u, 64u, 0, &flash_erase, &flash_program, &flash_reset, &flash_status};

static int flash_wait(flash_ptr_t ptr, int tick, int count)
{
	int threshold;

	threshold = tick - 1;
	while ( threshold >= 0 )
	{
		int sr;

		*ptr = 0x70;
		sr = *ptr;
		if ( (sr & 0x80) == 0 )
			sr = -116;
		if ( sr >= 0 )
			return sr;
		threshold--;
	}
	do
	{
		int sr_v3;

		DelayThread(tick);
		*ptr = 0x70;
		sr_v3 = *ptr;
		if ( (sr_v3 & 0x80) == 0 )
			sr_v3 = -116;
		if ( sr_v3 >= 0 )
			return sr_v3;
		--count;
	} while ( count >= 0 );
	return -116;
}

static int flash_erase(flash_addr_t addr)
{
	*(volatile acUint16 *)addr = 0x20;
	*(volatile acUint16 *)addr = 0xD0;
	return ((acUint32)(~flash_wait((flash_ptr_t)addr, 400, 2)) >> 31) & 0x20000;
}

static int flash_program(flash_addr_t addr, const flash_data_t *buf, int size)
{
	int rest;
	int xlen;
	int count;
	int threshold;
	int v12;
	unsigned int v14;
	int xlen_v9;

	rest = size;
	while ( 1 )
	{
		int blen;

		if ( rest <= 0 )
			return size - rest;
		blen = 0x20000 - (addr & 0x1FFFF);
		if ( rest < blen )
			blen = rest;
		rest -= blen;
		while ( blen > 0 )
		{
			flash_addr_t v8;

			v8 = addr & 0xF;
			xlen = 16 - v8;
			count = 2;
			if ( blen < (int)(16 - v8) )
				xlen = blen;
			threshold = 399;
			while ( 1 )
			{
				*(volatile acUint16 *)addr = 0xE8;
				v12 = *(acUint16 *)addr;
				if ( (v12 & 0x80) != 0 )
					break;
				threshold--;
				if ( threshold < 0 )
				{
					while ( count >= 0 )
					{
						DelayThread(400);
						*(volatile acUint16 *)addr = 0xE8;
						v12 = *(volatile acUint16 *)addr;
						// cppcheck-suppress knownConditionTrueFalse
						if ( (v12 & 0x80) != 0 )
							break;
						--count;
						v12 = -116;
					}
					break;
				}
			}
			v14 = xlen + 1;
			if ( v12 < 0 )
				break;
			blen -= xlen;
			*(volatile acUint16 *)addr = v14 >> 1;
			xlen_v9 = (v14 >> 1) - 1;
			for ( *(volatile acUint16 *)addr = *buf; xlen_v9 >= 0; addr += 2 )
			{
				--xlen_v9;
				*(volatile acUint16 *)addr = *buf++;
			}
			*(volatile acUint16 *)(addr - 2) = 0xD0;
		}
		if ( flash_wait((flash_ptr_t)(addr - 2), 400, 2) < 0 || blen > 0 )
			return rest + blen - size;
	}
}

static int flash_reset(flash_addr_t addr)
{
	*(acUint16 *)addr = 0xFF;
	return 0x800000;
}

flash_ops_t flash_probe_i28f640f5(flash_addr_t addr)
{
	acUint16 vendor;
	acUint16 device;

	*(volatile acUint16 *)addr = 0xFF;
	*(volatile acUint16 *)addr = 0x90;
	vendor = *(volatile acUint16 *)addr;
	*(volatile acUint16 *)addr = 0x90;
	device = *(volatile acUint16 *)(addr + 2);
	*(volatile acUint16 *)addr = 0xFF;
	// cppcheck-suppress knownConditionTrueFalse
	if ( vendor != 0x89 || device != 0x15 )
		return 0;
	return &ops_30;
}

static int flash_status(flash_addr_t addr)
{
	*(volatile acUint16 *)addr = 0x70;
	// cppcheck-suppress knownConditionTrueFalse
	if ( (*(volatile acUint16 *)addr & 0x80) != 0 )
		return 1;
	return 2;
}
