/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Advanced I/O library.
*/

#include "types.h"
#include "defs.h"
#include "loadcore.h"
#include "iomanX.h"
#include "sysclib.h"
#include "intrman.h"
#include "sys/stat.h"

#define MODNAME "iomanx"
IRX_ID("IOX/File_Manager", 1, 1);

#include "errno.h"

#define MAX_DEVICES 32
#define MAX_FILES   32

static iop_device_t *dev_list[MAX_DEVICES];
iop_file_t file_table[MAX_FILES];

#define isnum(c) ((c) >= '0' && (c) <= '9')

struct irx_export_table _exp_iomanx;

extern int hook_ioman();
extern int unhook_ioman();

iop_device_t **GetDeviceList(void)
{
    return(dev_list);
}

int _start(int argc, char **argv)
{
	if(RegisterLibraryEntries(&_exp_iomanx) != 0)
    {
		return 1;
	}

    memset(dev_list, 0, sizeof(dev_list));
    memset(file_table, 0, sizeof(file_table));

    if(hook_ioman() != 0)
    {
        return 1;
    }

	return 0;
}

int shutdown()
{
    unhook_ioman();
	return 1;
}

int AddDrv(iop_device_t *device)
{
	int i, res = -1;
    int oldIntr;

    CpuSuspendIntr(&oldIntr);

	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (dev_list[i] == NULL)
			break;
	}

	if (i >= MAX_DEVICES)
	{
	    CpuResumeIntr(oldIntr);
		return res;
	}

	dev_list[i] = device;
    CpuResumeIntr(oldIntr);

    FlushIcache();

	if ((res = device->ops->init(device)) < 0)
	{
		dev_list[i] = NULL;
		return(-1);
	}

	return(0);
}

int DelDrv(const char *name)
{
	int i;

	for (i = 0; i < MAX_DEVICES; i++) {
		if (dev_list[i] != NULL && !strcmp(name, dev_list[i]->name)) {
			dev_list[i]->ops->deinit(dev_list[i]);
			dev_list[i] = NULL;
			return 0;
		}
	}

	return -1;
}


static char * find_iop_device(const char *dev, int *unit, iop_device_t **device)
{
	char canon[16];
	char *filename, *tail, *d = (char *)dev;
	int i, len, num = 0;

	if (*d == ' ') {
		while (*d == ' ')
			d++;
		d--;
	}

	if ((tail = index(d, ':')) == NULL)
		return (char *)-1;

	len = (int)(tail - d);
	strncpy(canon, d, len);
	canon[len] = '\0';

	/* This is the name passed to the device op.  */
	filename = d + len + 1;

	/* Search backward for the unit number.  */
	if (isnum(canon[len - 1])) {
		while (isnum(canon[len - 1]))
			len--;

		num = strtol(canon + len, 0, 10);
	}
	if (unit)
		*unit = num;

	/* Find the actual device.  */
	canon[len] = '\0';
	for (i = 0; i < MAX_DEVICES; i++) {
		if (dev_list[i] != NULL && !strcmp(canon, dev_list[i]->name)) {
			if (device)
				*device = dev_list[i];

			return filename;
		}
	}

	return (char *)-1;
}

iop_file_t *get_file(int fd)
{
	if (fd >= MAX_FILES)
		return NULL;

	if (file_table[fd].device != NULL)
		return &file_table[fd];

	return NULL;
}

iop_file_t *get_new_file(void)
{
	int i;
	iop_file_t *fd = NULL;
	int oldIntr;

	CpuSuspendIntr(&oldIntr);

	for (i = 0; i < MAX_FILES; i++)
	{
		if (!file_table[i].device)
		{
			fd = &file_table[i];

			// fill in "device" temporarily to mark the fd as allocated.
			fd->device = (iop_device_t *) 0xFFFFFFFF;
			break;
		}
	}

	CpuResumeIntr(oldIntr);

	return fd;
}

int open(const char *name, int flags, int mode)
{
	iop_file_t *f = get_new_file();
	char *filename;
	int res = -ENOSYS;

	if (!f)
	{
		return -EMFILE;
	}

	if ((filename = find_iop_device(name, &f->unit, &f->device)) == (char *)-1)
	{
        f->device = NULL;
		return -ENODEV;
	}

	f->mode = flags;
	if ((res = f->device->ops->open(f, filename, flags, mode)) >= 0)
	{
		res = (int)(f - file_table);
	}
	else
	{
        f->mode = 0;
        f->device = NULL;
	}

	return res;
}

int close(int fd)
{
	iop_file_t *f;
	int res;

	if ((f = get_file(fd)) == NULL)
	{
		return -EBADF;
	}

	if (f->mode & 8)
	{	/* Directory.  */
		res = f->device->ops->dclose(f);
	}
	else
	{
		res = f->device->ops->close(f);
	}

	f->mode = 0;
	f->device = NULL;
	return res;
}

int read(int fd, void *ptr, size_t size)
{
	iop_file_t *f = get_file(fd);

	if (f == NULL || !(f->mode & O_RDONLY))
		return -EBADF;

	return f->device->ops->read(f, ptr, size);
}

int write(int fd, void *ptr, size_t size)
{
	iop_file_t *f = get_file(fd);

	if (f == NULL || !(f->mode & O_WRONLY))
		return -EBADF;

	return f->device->ops->write(f, ptr, size);
}

int lseek(int fd, int offset, int whence)
{
	iop_file_t *f = get_file(fd);

	if (f == NULL)
		return -EBADF;

	if (whence < SEEK_SET || whence > SEEK_END)
		return -EINVAL;

	return f->device->ops->lseek(f, offset, whence);
}

int ioctl(int fd, unsigned long cmd, void *arg)
{
	iop_file_t *f = get_file(fd);

	if (f == NULL)
		return -EBADF;

	return f->device->ops->ioctl(f, cmd, arg);
}

int remove(const char *name)
{
	iop_file_t file;
	char *filename;

	if ((filename = find_iop_device(name, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	return file.device->ops->remove(&file, filename);
}

/* Because mkdir, rmdir, chdir, and sync have similiar arguments (each starts
   with a path followed by an optional integer), we use a common routine to
   handle all of them.  */
static int path_common(const char *name, int arg, int code)
{
	iop_file_t file;
	iop_device_ops_t *dops;
	char *filename;

	if ((filename = find_iop_device(name, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	if (code & 0x100)
		if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
			return -48;

	dops = (iop_device_ops_t *)file.device->ops;
	switch (code) {
		case 4:		/* mkdir() */
			return dops->mkdir(&file, filename, arg);
		case 5:		/* rmdir() */
			return dops->rmdir(&file, filename);
		case 0x103:	/* chdir() */
			return dops->chdir(&file, filename);
		case 0x106:
			return dops->sync(&file, filename, arg);
	}

	return -EINVAL;
}

int mkdir(const char *name, int mode)
{
	return path_common(name, mode, 4);
}

int rmdir(const char *name)
{
	return path_common(name, 0, 5);
}

int dopen(const char *name)
{
	iop_file_t *f = get_new_file();
	char *filename;
	int res;

	if (!f)
		return -EMFILE;

	if ((filename = find_iop_device(name, &f->unit, &f->device)) == (char *)-1)
	{
        f->device = NULL;
		return -ENODEV;
	}

	f->mode = 8;	/* Indicates a directory.  */
	if ((res = f->device->ops->dopen(f, filename)) >= 0)
		res = (int)(f - file_table);
	else
	{
        f->mode = 0;
        f->device = NULL;
	}

	return res;
}

static int mode2modex(int mode)
{
	int modex = 0;

	if (FIO_SO_ISLNK(mode))
		modex |= FIO_S_IFLNK;
	if (FIO_SO_ISREG(mode))
		modex |= FIO_S_IFREG;
	if (FIO_SO_ISDIR(mode))
		modex |= FIO_S_IFDIR;

	/* Convert the file access modes.  */
	if (mode & FIO_SO_IROTH)
		modex |= FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH;
	if (mode & FIO_SO_IWOTH)
		modex |= FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH;
	if (mode & FIO_SO_IXOTH)
		modex |= FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH;

	return modex;
}

int dread(int fd, iox_dirent_t *iox_dirent)
{
    iop_file_t *f = get_file(fd);
    int res;

    if (f == NULL ||  !(f->mode & 8))
            return -EBADF;

    /* If this is a legacy device (such as mc:) then we need to convert the mode
       variable of the stat structure to iomanX's extended format.  */
    if ((f->device->type & 0xf0000000) != IOP_DT_FSEXT)
    {
        typedef int	io_dread_t(iop_file_t *, io_dirent_t *);
        io_dirent_t io_dirent;
        io_dread_t *io_dread = (io_dread_t*) f->device->ops->dread;
        res = io_dread(f, &io_dirent);

        iox_dirent->stat.mode = mode2modex(io_dirent.stat.mode);

        iox_dirent->stat.attr = io_dirent.stat.attr;
        iox_dirent->stat.size = io_dirent.stat.size;
        memcpy(iox_dirent->stat.ctime, io_dirent.stat.ctime, sizeof(io_dirent.stat.ctime));
        memcpy(iox_dirent->stat.atime, io_dirent.stat.atime, sizeof(io_dirent.stat.atime));
        memcpy(iox_dirent->stat.mtime, io_dirent.stat.mtime, sizeof(io_dirent.stat.mtime));
        iox_dirent->stat.hisize = io_dirent.stat.hisize;

        strncpy(iox_dirent->name, io_dirent.name, sizeof(iox_dirent->name));
    }
    else
        res = f->device->ops->dread(f, iox_dirent);

    return res;
}

static int stat_common(const char *name, void *buf, int mask, int code)
{
	iop_file_t file;
	io_stat_t *stat = (io_stat_t *)buf;
	char *filename;
	int res;

	if ((filename = find_iop_device(name, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	if (code == 1)	{ /* chstat() */
		return file.device->ops->chstat(&file, filename, buf, mask);
	}

	res = file.device->ops->getstat(&file, filename, buf);

	/* If this is a legacy device (such as mc:) then we need to convert the mode
	   variable to iomanX's extended format.  */
	if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
		stat->mode = mode2modex(stat->mode);

	return res;
}

int getstat(const char *name, iox_stat_t *stat)
{
	return stat_common(name, stat, 0, 2);
}

int chstat(const char *name, iox_stat_t *stat, unsigned int mask)
{
	return stat_common(name, stat, mask, 1);
}

int format(const char *dev, const char *blockdev, void *arg, size_t arglen)
{
	iop_file_t file;
	char *filename;

	if ((filename = find_iop_device(dev, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	return file.device->ops->format(&file, filename, blockdev, arg, arglen);
}

static int link_common(const char *old, const char *new, int code)
{
	iop_file_t file;
	iop_device_t *new_device;
	char *filename, *new_filename = (char *)new;
	int new_unit;

	if ((filename = find_iop_device(old, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	/* Make sure the user isn't attempting to link across devices.  */
	if (index(new, ':') != NULL) {
		new_filename = find_iop_device(new, &new_unit, &new_device);
		if ((new_filename == (char *)-1) || (new_unit != file.unit) ||
				(new_device != file.device))
			return -ENXIO;
	}

	/* The filesystem must support these ops.  */
	if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
		return -48;

	if (code == 7)	/* rename() */
		return file.device->ops->rename(&file, filename, new_filename);

	return file.device->ops->symlink(&file, filename, new_filename);
}

int rename(const char *old, const char *new)
{
	return link_common(old, new, 7);
}

int chdir(const char *name)
{
	return path_common(name, 0, 0x103);
}

int sync(const char *dev, int flag)
{
	return path_common(dev, flag, 0x106);
}

int mount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen)
{
	iop_file_t file;
	char *filename;

	if ((filename = find_iop_device(fsname, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	/* The filesystem must support these ops.  */
	if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
		return -48;

	return file.device->ops->mount(&file, filename, devname, flag, arg, arglen);
}

int umount(const char *fsname)
{
	iop_file_t file;
	char *filename;

	if ((filename = find_iop_device(fsname, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	/* The filesystem must support these ops.  */
	if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
		return -48;

	return file.device->ops->umount(&file, filename);
}

int lseek64(int fd, long long offset, int whence)
{
	iop_file_t *f = get_file(fd);

	if (f == NULL)
		return -EBADF;

	if (whence < SEEK_SET || whence > SEEK_END)
		return -EINVAL;

	if ((f->device->type & 0xf0000000) != IOP_DT_FSEXT)
		return -48;

	return f->device->ops->lseek64(f, offset, whence);
}

int devctl(const char *name, int cmd, void *arg, size_t arglen, void *buf, size_t buflen)
{
	iop_file_t file;
	char *filename;

	if ((filename = find_iop_device(name, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	/* The filesystem must support these ops.  */
	if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
		return -48;

	return file.device->ops->devctl(&file, filename, cmd, arg, arglen, buf, buflen);
}

int symlink(const char *old, const char *new)
{
	return link_common(old, new, 8);
}

int readlink(const char *name, char *buf, size_t buflen)
{
	iop_file_t file;
	char *filename;

	if ((filename = find_iop_device(name, &file.unit, &file.device)) == (char *)-1)
		return -ENODEV;

	/* The filesystem must support these ops.  */
	if ((file.device->type & 0xf0000000) != IOP_DT_FSEXT)
		return -48;

	return file.device->ops->readlink(&file, filename, buf, buflen);
}


int ioctl2(int fd, int command, void *arg, size_t arglen, void *buf, size_t buflen)
{
	iop_file_t *f;

	if ((f = get_file(fd)) == NULL)
		return -EBADF;

	return f->device->ops->ioctl2(f, command, arg, arglen, buf, buflen);
}
