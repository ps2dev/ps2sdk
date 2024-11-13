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

static int cddrv_adddrv(iop_device_t *drv);
static int cddrv_deldrv(iop_device_t *drv);
static int cddrv_open(iop_file_t *io, const char *name, int flg);
static int cddrv_close(iop_file_t *io);
static int cddrv_read(iop_file_t *io, void *buf, int cnt);
static int cddrv_write(iop_file_t *io, void *buf, int cnt);
static int cddrv_lseek(iop_file_t *io, int offset, int whence);
static int cddrv_ioctl(iop_file_t *io, int cmd, void *arg);

static iop_device_ops_t Cddrv_ops = {
	&cddrv_adddrv, // init
	&cddrv_deldrv, // deinit
	NOT_SUPPORTED, // format
	&cddrv_open, // open
	&cddrv_close, // close
	&cddrv_read, // read
	&cddrv_write, // write
	&cddrv_lseek, // lseek
	&cddrv_ioctl, // ioctl
	NOT_SUPPORTED, // remove
	NOT_SUPPORTED, // mkdir
	NOT_SUPPORTED, // rmdir
	NOT_SUPPORTED, // dopen
	NOT_SUPPORTED, // dclose
	NOT_SUPPORTED, // dread
	NOT_SUPPORTED, // getstat
	NOT_SUPPORTED, // chstat
};

static iop_device_t Cddrv = {"cdrom", 16u, 0u, "ATAPI_C/DVD-ROM", &Cddrv_ops};

static int cddrv_adddrv(iop_device_t *drv)
{
	(void)drv;

	return 0;
}

static int cddrv_deldrv(iop_device_t *drv)
{
	(void)drv;

	return 0;
}

static int cddrv_open(iop_file_t *io, const char *name, int flg)
{
	int ret;
	struct cdfs_file *v9;
	struct cdfs_file *file;
	acUint32 d_size;
	struct cdfs_dirent dirent;

	ret = 1;
	while ( ret > 0 )
	{
		int ret_v2;

		if ( io->unit )
		{
			return -ENXIO;
		}
		if ( flg != 1 )
		{
			return -EINVAL;
		}
		ret_v2 = cdfs_lookup(&dirent, name, strlen(name));
		if ( ret_v2 >= 0 )
		{
			break;
		}
		ret = cdfs_recover(ret_v2);
	}
	if ( (dirent.d_ftype & 2) != 0 )
	{
		return -EISDIR;
	}
	v9 = (struct cdfs_file *)AllocSysMemory(1, 16, 0);
	file = v9;
	if ( !v9 )
	{
		return -ENOMEM;
	}
	io->privdata = v9;
	v9->f_lsn = dirent.d_lsn;
	d_size = dirent.d_size;
	file->f_pos = 0;
	file->f_size = d_size;
	return 0;
}

static int cddrv_close(iop_file_t *io)
{
	void *ptr;

	ptr = io->privdata;
	io->privdata = 0;
	if ( ptr )
		FreeSysMemory(ptr);
	return 0;
}

static int cddrv_read(iop_file_t *io, void *buf, int cnt)
{
	if ( !buf || !cnt )
		return 0;
	return cdfs_read((struct cdfs_file *)io->privdata, buf, cnt);
}

static int cddrv_write(iop_file_t *io, void *buf, int cnt)
{
	(void)io;
	(void)buf;
	(void)cnt;

	return -EROFS;
}

static int cddrv_lseek(iop_file_t *io, int offset, int whence)
{
	struct cdfs_file *file;
	acUint32 size;

	file = (struct cdfs_file *)io->privdata;
	size = file->f_size;
	if ( whence == 1 )
	{
		offset += file->f_pos;
	}
	else if ( whence == 2 )
	{
		offset += size;
	}
	if ( size < (unsigned int)offset )
		offset = file->f_size;
	file->f_pos = offset;
	return offset;
}

static int cddrv_ioctl(iop_file_t *io, int cmd, void *arg)
{
	(void)io;
	(void)cmd;
	(void)arg;

	return -EINVAL;
}

int cddrv_module_start(int argc, char **argv)
{
	int v2;
	int v3;

	(void)argc;
	(void)argv;
	v2 = AddDrv(&Cddrv);
	v3 = 0;
	if ( v2 < 0 )
		return -16;
	return v3;
}

int cddrv_module_stop()
{
	int v0;
	int v1;

	v0 = DelDrv(Cddrv.name);
	v1 = 0;
	if ( v0 < 0 )
		return -16;
	return v1;
}

int cddrv_module_restart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}

int cddrv_module_status()
{
	return 0;
}
