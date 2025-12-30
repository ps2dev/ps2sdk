/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include <errno.h>
#include <s147mdev.h>

IRX_ID("S147MDEV", 2, 1);
// Text section hash:
// bda8a15d6a4c9a560598c37fd397d263

static int register_atfile_ioman_device(const char *atfile_name, const char *atfile_desc);
static int atfile_drv_op_nulldev(const iop_file_t *f);
static int atfile_drv_op_init(iop_device_t *dev);
static int atfile_drv_op_deinit(iop_device_t *dev);
static int atfile_drv_op_open(iop_file_t *f, const char *name, int flags);
static int atfile_drv_op_close(iop_file_t *f);
static int atfile_drv_op_read(iop_file_t *f, void *ptr, int size);
static int atfile_drv_op_write(iop_file_t *f, void *ptr, int size);
static int atfile_drv_op_lseek(iop_file_t *f, int offset, int mode);

extern struct irx_export_table _exp_s147mdev;
static iop_device_ops_t atfile_drv_ops = {
	&atfile_drv_op_init,
	&atfile_drv_op_deinit,
	(void *)&atfile_drv_op_nulldev,
	&atfile_drv_op_open,
	&atfile_drv_op_close,
	&atfile_drv_op_read,
	&atfile_drv_op_write,
	&atfile_drv_op_lseek,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
	(void *)&atfile_drv_op_nulldev,
};
static iop_device_t g_atfile_device;
static iop_device_t *g_atfile_unit_info[10];

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;
	Kprintf("\ns147mdev.irx: System147 Multi Device File System Manager v%d.%d\n", 2, 1);
	register_atfile_ioman_device("atfile", "Multi Device File System");
	if ( RegisterLibraryEntries(&_exp_s147mdev) )
	{
		Kprintf("s147mdev.irx: RegisterLibraryEntries - Failed.\n");
		return MODULE_NO_RESIDENT_END;
	}
	Kprintf("s147mdev.irx: RegisterLibraryEntries - OK.\n");
	return MODULE_RESIDENT_END;
}

static int register_atfile_ioman_device(const char *atfile_name, const char *atfile_desc)
{
	int i;

	for ( i = 0; i < 10; i += 1 )
		g_atfile_unit_info[i] = 0;
	g_atfile_device.name = atfile_name;
	g_atfile_device.type = IOP_DT_FS;
	g_atfile_device.version = 0;
	g_atfile_device.desc = atfile_desc;
	g_atfile_device.ops = &atfile_drv_ops;
	DelDrv(atfile_name);
	AddDrv(&g_atfile_device);
	return 0;
}

int s147mdev_4_addfs(iop_device_t *drv, int unit10)
{
	int retres;

	if ( unit10 < 0 || unit10 >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -1;
	}
	if ( !drv )
	{
		Kprintf("s147mdev.irx: Invalid device table\n");
		return -1;
	}
	retres = drv->ops->init(drv);
	if ( retres >= 0 )
		g_atfile_unit_info[unit10 / 10] = drv;
	return retres;
}

int s147mdev_5_delfs(int unit10)
{
	int retres;

	if ( unit10 < 0 || unit10 >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -1;
	}
	retres = (g_atfile_unit_info[unit10 / 10]) ?
						 g_atfile_unit_info[unit10 / 10]->ops->deinit(g_atfile_unit_info[unit10 / 10]) :
						 0;
	if ( retres >= 0 )
		g_atfile_unit_info[unit10 / 10] = 0;
	return retres;
}

static int atfile_drv_op_nulldev(const iop_file_t *f)
{
	int unit;

	unit = f->unit;
	// Unofficial: add bounds check here
	if ( unit < 0 || unit >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -ENODEV;
	}
	if ( !g_atfile_unit_info[unit / 10] )
	{
		Kprintf("s147mdev.irx: Undefined unit number (%d), do nothing\n", unit);
		return -ENODEV;
	}
	return 0;
}

static int atfile_drv_op_init(iop_device_t *dev)
{
	(void)dev;
	return 0;
}

static int atfile_drv_op_deinit(iop_device_t *dev)
{
	(void)dev;
	return 0;
}

static int atfile_drv_op_open(iop_file_t *f, const char *name, int flags)
{
	iop_file_t fstk;
	int unit;
	int retres;

	unit = f->unit;
	if ( unit < 0 || unit >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -ENODEV;
	}
	fstk.mode = f->mode;
	fstk.unit = f->unit % 10;
	fstk.device = f->device;
	if ( !g_atfile_unit_info[unit / 10] )
	{
		Kprintf("s147mdev.irx: Undefined unit number (%d), do nothing\n", unit);
		return -ENODEV;
	}
	retres = g_atfile_unit_info[unit / 10]->ops->open(&fstk, name, flags);
	f->privdata = fstk.privdata;
	return retres;
}

static int atfile_drv_op_close(iop_file_t *f)
{
	iop_file_t fstk;
	int unit;

	unit = f->unit;
	if ( unit < 0 || unit >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -ENODEV;
	}
	fstk.mode = f->mode;
	fstk.unit = f->unit % 10;
	fstk.device = f->device;
	fstk.privdata = f->privdata;
	if ( !g_atfile_unit_info[unit / 10] )
	{
		Kprintf("s147mdev.irx: Undefined unit number (%d), do nothing\n", unit);
		return -ENODEV;
	}
	return g_atfile_unit_info[unit / 10]->ops->close(&fstk);
}

static int atfile_drv_op_read(iop_file_t *f, void *ptr, int size)
{
	iop_file_t fstk;
	int unit;

	unit = f->unit;
	if ( unit < 0 || unit >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -ENODEV;
	}
	fstk.mode = f->mode;
	fstk.unit = f->unit % 10;
	fstk.device = f->device;
	fstk.privdata = f->privdata;
	if ( !g_atfile_unit_info[unit / 10] )
	{
		Kprintf("s147mdev.irx: Undefined unit number (%d), do nothing\n", unit);
		return -ENODEV;
	}
	return g_atfile_unit_info[unit / 10]->ops->read(&fstk, ptr, size);
}

static int atfile_drv_op_write(iop_file_t *f, void *ptr, int size)
{
	iop_file_t fstk;
	int unit;

	unit = f->unit;
	if ( unit < 0 || unit >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -ENODEV;
	}
	fstk.mode = f->mode;
	fstk.unit = f->unit % 10;
	fstk.device = f->device;
	fstk.privdata = f->privdata;
	if ( !g_atfile_unit_info[unit / 10] )
	{
		Kprintf("s147mdev.irx: Undefined unit number (%d), do nothing\n", unit);
		return -ENODEV;
	}
	return g_atfile_unit_info[unit / 10]->ops->write(&fstk, ptr, size);
}

static int atfile_drv_op_lseek(iop_file_t *f, int offset, int mode)
{
	iop_file_t fstk;
	int unit;

	unit = f->unit;
	if ( unit < 0 || unit >= 100 )
	{
		Kprintf("s147mdev.irx: Invalid unit number\n");
		return -ENODEV;
	}
	fstk.mode = f->mode;
	fstk.unit = f->unit % 10;
	fstk.device = f->device;
	fstk.privdata = f->privdata;
	if ( !g_atfile_unit_info[unit / 10] )
	{
		Kprintf("s147mdev.irx: Undefined unit number (%d), do nothing\n", unit);
		return -ENODEV;
	}
	return g_atfile_unit_info[unit / 10]->ops->lseek(&fstk, offset, mode);
}
