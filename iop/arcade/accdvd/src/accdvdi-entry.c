/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accdvd_internal.h"

#define MODNAME "CD/DVD_Compatible"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 0530902ecc03cd3839dfd4b9d9367440
// Known titles:
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/cdvd-iop-0.1.9/src/
// /home/ueda/tmp/psalm-0.1.3/core-hdr-0.1.3/src/util/
// TODO: diff with module text hash 3b63ef6314122a3f5bf1ae695def643b

extern struct irx_export_table _exp_accdvd;

#define acCdvdEntry _start

static struct cdvd_modules Cdvd_modules[4] = {
	{&acd_module_restart, &acd_module_start, &acd_module_status, &acd_module_stop},
	{&cdfs_module_restart, &cdfs_module_start, &cdfs_module_status, &cdfs_module_stop},
	{&cddrv_module_restart, &cddrv_module_start, &cddrv_module_status, &cddrv_module_stop},
	{&cdc_module_restart, &cdc_module_start, &cdc_module_status, &cdc_module_stop}};

int acCdvdModuleRestart(int argc, char **argv)
{
	int v2;
	int index;

	v2 = argc;
	for ( index = 0; (unsigned int)index < 4; ++index )
	{
		int v5;
		int ret;

		v5 = Cdvd_modules[index].cm_restart(argc, argv);
		ret = v5;
		if ( v5 < 0 )
		{
			printf("accdvd:init_restart:%d: error %d\n", index, v5);
			return ret;
		}
		argc = v2;
	}
	return 0;
}

int acCdvdModuleStart(int argc, char **argv)
{
	int v2;
	int index;

	v2 = argc;
	for ( index = 0; (unsigned int)index < 4; ++index )
	{
		int v5;
		int ret;

		v5 = Cdvd_modules[index].cm_start(argc, argv);
		ret = v5;
		if ( v5 < 0 )
		{
			printf("accdvd:init_start:%d: error %d\n", index, v5);
			return ret;
		}
		argc = v2;
	}
	return 0;
}

int acCdvdModuleStatus()
{
	int status;
	int index;

	status = 0;
	index = 0;
	while ( (unsigned int)index < 4 )
	{
		int ret;

		ret = Cdvd_modules[index].cm_status();
		if ( ret < 0 )
		{
			printf("accdvd:init_status:%d: error %d\n", index, ret);
			return ret;
		}
		if ( status < ret )
			status = ret;
		index++;
	}
	return status;
}

int acCdvdModuleStop()
{
	int index;

	index = 3;
	while ( index >= 0 )
	{
		int v2;
		int ret;

		v2 = Cdvd_modules[index].cm_stop();
		ret = v2;
		if ( v2 < 0 )
		{
			printf("accdvd:init_stop:%d: error %d\n", index, v2);
			return ret;
		}
		index--;
	}
	return 0;
}

int acCdvdEntry(int argc, char **argv)
{
	int ret;

	ret = acCdvdModuleStart(argc, argv);
	DelayThread(1000);
	if ( ret < 0 )
		return ret;
	if ( RegisterLibraryEntries(&_exp_accdvd) != 0 )
		return -16;
	return 0;
}
