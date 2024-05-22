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

static flash_probe_t probes_22[2] = {&flash_probe_i28f640f5, &flash_probe_mbm29f033c};
static struct flash_softc Flashc;

int acFlashStart()
{
	if ( Flashc.status == 2 )
		return -11;
	if ( Flashc.status != 1 )
	{
		return -6;
	}
	*((volatile acUint16 *)0xB2416006) = 0;
	Flashc.status = 2;
	return 0;
}

int acFlashStop()
{
	flash_ops_t ops;
	int bsize;
	int blocks;
	acFlashAddr addr;
	acFlashAddr v5;

	if ( Flashc.status == 1 )
		return 0;
	if ( Flashc.status != 2 )
	{
		return -6;
	}
	ops = Flashc.ops;
	bsize = Flashc.ops->fo_bsize;
	blocks = Flashc.ops->fo_blocks - 1;
	addr = 0xB0000000;
	v5 = 0xB0000000;
	while ( blocks >= 0 )
	{
		addr += bsize;
		--blocks;
		ops->fo_reset(v5);
		v5 = addr;
	}
	Flashc.status = 1;
	*((volatile acUint16 *)0xB2416004) = 0;
	return 0;
}

int acFlashErase(acFlashAddr addr)
{
	if ( Flashc.status == 1 )
		return -13;
	if ( Flashc.status != 2 )
	{
		return -6;
	}
	if ( addr >= (acUint32)Flashc.size )
		return -34;
	return Flashc.ops->fo_erase(addr + 0xB0000000);
}

int acFlashProgram(acFlashAddr addr, void *buf, int count)
{
	if ( Flashc.status == 1 )
		return -13;
	if ( Flashc.status != 2 )
	{
		return -6;
	}
	if ( (addr & 1) != 0 )
	{
		return -14;
	}
	if ( addr >= (acUint32)Flashc.size )
		return -34;
	return Flashc.ops->fo_program(addr + 0xB0000000, (flash_data_t *)buf, count);
}

int acFlashRead(acFlashAddr addr, void *buf, int count)
{
	void *v3;
	flash_addr_t v6;
	int size;
	signed int v8;
	struct flash_softc *flashc;

	v3 = buf;
	if ( Flashc.status == 0 )
	{
		return -6;
	}
	if ( (addr & 1) != 0 )
	{
		return -14;
	}
	if ( addr + count >= (acUint32)Flashc.size )
	{
		return -34;
	}
	v6 = (flash_addr_t)(addr + 0xB0000000);
	if ( Flashc.status >= 2 )
	{
		Flashc.ops->fo_reset(v6);
		buf = v3;
	}
	size = count;
	v8 = (unsigned int)count >> 1;
	for ( flashc = (struct flash_softc *)v6; v8 > 0; buf = (char *)buf + 2 )
	{
		--v8;
		*(acUint16 *)buf = flashc->status;
		flashc = (struct flash_softc *)((char *)flashc + 2);
	}
	return size - 2 * v8;
}

int acFlashVerify(acFlashAddr addr, void *buf, int count)
{
	void *v3;
	flash_addr_t v6;
	int size;
	signed int v8;
	struct flash_softc *flashc;

	v3 = buf;
	if ( Flashc.status == 0 )
	{
		return -6;
	}
	if ( (addr & 1) != 0 )
	{
		return -14;
	}
	if ( addr + count >= (acUint32)Flashc.size )
	{
		return -34;
	}
	v6 = (flash_addr_t)(addr + 0xB0000000);
	if ( Flashc.status >= 2 )
	{
		Flashc.ops->fo_reset(v6);
		buf = v3;
	}
	size = count;
	v8 = (unsigned int)count >> 1;
	for ( flashc = (struct flash_softc *)v6; v8 > 0; --v8 )
	{
		int status_low;
		int v11;

		status_low = flashc->status & 0xFFFF;
		flashc = (struct flash_softc *)((char *)flashc + 2);
		v11 = *(acUint16 *)buf;
		buf = (char *)buf + 2;
		if ( v11 != status_low )
			break;
	}
	return size - 2 * v8;
}

int acFlashStatus(acFlashAddr addr)
{
	if ( Flashc.status == 1 )
		return 1;
	if ( Flashc.status == 2 )
	{
		if ( addr >= (acUint32)Flashc.size )
			return -34;
		return Flashc.ops->fo_status(addr + 0xB0000000);
	}
	return -6;
}

int acFlashInfo(acFlashInfoData *info)
{
	flash_ops_t ops;
	struct flash_softc *flashc;

	if ( info == NULL )
	{
		return -22;
	}
	if ( Flashc.status == 0 )
	{
		return -6;
	}
	ops = Flashc.ops;
	info->fi_blocks = Flashc.ops->fo_blocks;
	flashc = (struct flash_softc *)ops->fo_bsize;
	info->fi_bsize = (acUint32)flashc;
	return 0;
}

int acFlashModuleStatus()
{
	return Flashc.status;
}

int acFlashModuleStart(int argc, char **argv)
{
	acInt32 v4;
	int v5;
	int index;
	struct flash_ops *v9;

	(void)argc;
	(void)argv;
	if ( acFlashModuleStatus() != 0 )
	{
		return -16;
	}
	*((volatile acUint16 *)0xB2416006) = 0;
	DelayThread(100000);
	index = 0;
	while ( (unsigned int)index < 2 )
	{
		if ( *(probes_22[index]) )
		{
			v9 = (probes_22[index])(0xB0000000);
			if ( v9 )
			{
				Flashc.status = 1;
				v4 = v9->fo_bsize * v9->fo_blocks;
				Flashc.ops = v9;
				v5 = 0;
				Flashc.size = v4;
				break;
			}
		}
		++index;
	}
	if ( (unsigned int)index >= 2 )
	{
		printf("acflash: no flash\n");
		v5 = -6;
	}
	*((volatile acUint16 *)0xB2416004) = 0;
	return v5;
}

int acFlashModuleStop()
{
	if ( acFlashModuleStatus() != 0 )
	{
		acFlashStop();
	}
	return 0;
}

int acFlashModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}
