/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Advanced I/O library.
*/

#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "loadcore.h"
#include "iomanX.h"
#include "sysclib.h"
#include "sys/stat.h"

#define MODNAME "iomanx"
IRX_ID("IOX/File_Manager", 1, 1);

#include "errno.h"

#define MAX_DEVICES 32
#define MAX_FILES   32

static iop_device_t *dev_list[MAX_DEVICES];
static iop_file_t file_table[MAX_FILES];

#define isnum(c) ((c) >= '0' && (c) <= '9')

struct irx_export_table _exp_iomanx;

/* This bit of code allows iomanX to "steal" the original ioman's registered devices
   and use them as its own.  Although this functionality is now in iopmgr, this small
   bit was included here to avoid a dependency on iopmgr.  */

/* Module info entry.  */
typedef struct _smod_mod_info {
	struct _smod_mod_info *next;
	u8	*name;
	u16	version;
	u16	newflags;	/* For modload shipped with games.  */
	u16	id;
	u16	flags;		/* I believe this is where flags are kept for BIOS versions.  */
	u32	entry;		/* _start */
	u32	gp;
	u32	text_start;
	u32	text_size;
	u32	data_size;
	u32	bss_size;
	u32	unused1;
	u32	unused2;
} smod_mod_info_t;

static const char *ioman_modname = "IO/File_Manager";

static int smod_get_next_mod(smod_mod_info_t *cur_mod, smod_mod_info_t *next_mod)
{
	void *addr;

	/* If cur_mod is 0, return the head of the list (IOP address 0x800).  */
	if (!cur_mod) {
		addr = (void *)0x800;
	} else {
		if (!cur_mod->next)
			return 0;
		else
			addr = cur_mod->next;
	}

	memcpy(next_mod, addr, sizeof(smod_mod_info_t));
	return next_mod->id;
}

static int smod_get_mod_by_name(const char *name, smod_mod_info_t *info)
{
	int len = strlen(name) + 1; /* Thanks to adresd for this fix.  */

	if (!smod_get_next_mod(0, info))
		return 0;

	do {
		if (!memcmp(info->name, name, len))
			return info->id;
	} while (smod_get_next_mod(info, info) != 0);

	return 0;
}

static int (*p_AddDrv)(iop_device_t *) = 0;
static int (*p_DelDrv)(const char *) = 0;
static u32 *ioman_exports;

/* This is called by a module wanting to add a device to legacy ioman.  */
static int sbv_AddDrv(iop_device_t *device)
{
	int res;

	/* We get first dibs!  */
	res = AddDrv(device);

	if (p_AddDrv)
		return p_AddDrv(device);

	return res;
}

/* This is called by a module wanting to delete a device from legacy ioman.  */
static int sbv_DelDrv(const char *name)
{
	int res;

	res = DelDrv(name);

	if (p_DelDrv)
		return p_DelDrv(name);

	return res;
}

int _start(int argc, char **argv)
{
	iop_library_t ioman_library = { NULL, NULL, 0x102, 0, "ioman\0\0" };
	smod_mod_info_t info;
	iop_device_t **devinfo_table;
	int i;

	if (RegisterLibraryEntries(&_exp_iomanx) != 0)
		return 1;

	/* Steal the original ioman's registered devices.  */
	if (smod_get_mod_by_name(ioman_modname, &info)) {
		devinfo_table = (iop_device_t **)(info.text_start + info.text_size + info.data_size + 0x0c);

		/* There are a maximum of 16 entries in the original ioman.  */
		for (i = 0; i < 16; i++)
			if (devinfo_table[i])
				dev_list[i] = devinfo_table[i];
	}

	/* Patch ioman's AddDrv and DelDrv calls, so any modules loaded after us will get the patched
	   versions.  */
	if ((ioman_exports = (u32 *)QueryLibraryEntryTable(&ioman_library)) != NULL) {
		p_AddDrv = (void *)ioman_exports[20];
		p_DelDrv = (void *)ioman_exports[21];

		ioman_exports[20] = (u32)sbv_AddDrv;
		ioman_exports[21] = (u32)sbv_DelDrv;
	}

	return 0;
}

int shutdown()
{
	int i;

	/* Remove all registered devices.  */
	for (i = 0; i < MAX_DEVICES; i++) {
		if (dev_list[i] != NULL) {
			dev_list[i]->ops->deinit(dev_list[i]);
			dev_list[i] = NULL;
		}
	}

	/* Restore ioman's library exports.  */
	if (p_AddDrv && p_DelDrv) {
		ioman_exports[20] = (u32)p_AddDrv;
		ioman_exports[21] = (u32)p_DelDrv;
	}

	return 0;
}

int AddDrv(iop_device_t *device)
{
	int i, res = -1;

	for (i = 0; i < MAX_DEVICES; i++) {
		if (dev_list[i] == NULL)
			break;
	}

	if (i >= MAX_DEVICES)
		return res;

	if ((res = device->ops->init(device)) >= 0)
		dev_list[i] = device;

	return res;
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

iop_file_t *get_new_file()
{
	int i;

	for (i = 0; i < MAX_FILES; i++)
		if (!file_table[i].mode && !file_table[i].device)
			return &file_table[i];

	return NULL;
}
	
int open(const char *name, int flags, ...)
{
	va_list args;
	iop_file_t *f = get_new_file();
	char *filename;
	int mode, res = -ENOSYS;

	va_start(args, flags);
	mode = va_arg(args, int);
	va_end(args);

	if (!f)
		return -EMFILE;

	if ((filename = find_iop_device(name, &f->unit, &f->device)) == (char *)-1)
		return -ENODEV;

	f->mode = flags;
	if ((res = f->device->ops->open(f, filename, flags, mode)) >= 0)
		res = (int)(f - file_table);

	return res;
}

int close(int fd)
{
	iop_file_t *f;
	int res;

	if ((f = get_file(fd)) == NULL)
		return -EBADF;

	if (f->mode & 8)	/* Directory.  */
		res = f->device->ops->dclose(f);
	else
		res = f->device->ops->close(f);

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

int mkdir(const char *name, ...)
{
	va_list args;
	int mode;

	va_start(args, name);
	mode = va_arg(args, int);
	va_end(args);

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
		return -ENODEV;

	f->mode = 8;	/* Indicates a directory.  */
	if ((res = f->device->ops->dopen(f, filename)) >= 0)
		res = (int)(f - file_table);

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

int dread(int fd, void *buf)
{
	io_dirent_t *dirent = (io_dirent_t *)buf;
	iop_file_t *f = get_file(fd);
	int res;

	if (f == NULL ||  !(f->mode & 8))
		return -EBADF;

	res = f->device->ops->dread(f, buf);

	/* If this is a legacy device (such as mc:) then we need to convert the mode
	   variable of the stat structure to iomanX's extended format.  */
	if ((f->device->type & 0xf0000000) != IOP_DT_FSEXT)
		dirent->stat.mode = mode2modex(dirent->stat.mode);

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

int getstat(const char *name, void *buf)
{
	return stat_common(name, buf, 0, 2);
}

int chstat(const char *name, void *buf, unsigned int mask)
{
	return stat_common(name, buf, mask, 1);
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
