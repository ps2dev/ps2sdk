/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Standard file manager library.
 */

#include "types.h"
#ifdef _IOP
#include "sysmem.h"
#include "loadcore.h"
#endif
#include "ioman_mod.h"
#ifdef _IOP
#include "sysclib.h"
#else
#include <string.h>
#define index strchr
#endif
#include "stdarg.h"
#include "intrman.h"

#define MODNAME "ioman"
#ifdef _IOP
IRX_ID("IO/File_Manager", 1, 1);
#endif

#include "errno.h"

#define MAX_DEVICES 16
#define MAX_FILES   16

static iop_io_device_t *dev_list[MAX_DEVICES];
iop_io_file_t file_table[MAX_FILES];

static int should_print_known_devices = 1;

#define isnum(c) ((c) >= '0' && (c) <= '9')

#ifdef _IOP
extern struct irx_export_table _exp_ioman;
#endif

static int tty_noop()
{
	return 0;
}

iop_io_device_ops_t tty_dev_operations = {
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
	&tty_noop,
};

iop_io_device_t tty_device = {
	"tty",
	IOP_DT_CHAR,
	1,
	"CONSOLE",
	&tty_dev_operations,
};

static int add_tty_device(void)
{
	return io_AddDrv(&tty_device);
}

static int open_tty_handles(void)
{
	int result;

	io_close(0);
	io_close(1);
	result = io_open("tty00:", 3);
	if (result == 0)
	{
		return io_open("tty00:", 2);
	}
	return result;
}

static int setup_tty_device(int should_not_add_tty_device)
{
	int result;

	result = io_DelDrv("tty");
	if (should_not_add_tty_device == 0)
	{
		add_tty_device();
		return open_tty_handles();
	}
	return result;
}

static int print_abort_error_io(const char *str1, const char *str2)
{
	return Kprintf("ioabort exit:%s %s\n", str1, str2);
}

#if 0
// Unused code.
static int print_unsupported_operation(void)
{
	{
		const char *unsupported_operation_str = "io request for unsupported operation\r\n";
		io_write(1, unsupported_operation_str, strlen(unsupported_operation_str));
	}
	return -1;
}
#endif

int _start(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

#ifdef _IOP
	if(RegisterLibraryEntries(&_exp_ioman) != 0)
    {
		return MODULE_NO_RESIDENT_END;
	}
#endif

    memset(dev_list, 0, sizeof(dev_list));
    memset(file_table, 0, sizeof(file_table));

    setup_tty_device(0);

	return MODULE_RESIDENT_END;
}

int io_AddDrv(iop_io_device_t *device)
{
	int i;

	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (dev_list[i] == NULL)
			break;
	}

	if (i >= MAX_DEVICES)
	{
		return(-1);
	}

	dev_list[i] = device;

#ifdef _IOP
    FlushIcache();
#endif

	if (device->ops->io_init(device) < 0)
	{
		dev_list[i] = NULL;
		return(-1);
	}

	should_print_known_devices = 1;

	return(0);
}

int io_DelDrv(const char *name)
{
	int i;

	for (i = 0; i < MAX_DEVICES; i++) {
		if (dev_list[i] != NULL && !strcmp(name, dev_list[i]->name)) {
			dev_list[i]->ops->io_deinit(dev_list[i]);
			dev_list[i] = NULL;
			return 0;
		}
	}

	return -1;
}

static void print_known_devices(void)
{
	if (should_print_known_devices)
	{
		int i;

		Kprintf("Known devices are ");
		for (i = 0; i < MAX_DEVICES; i++)
		{
			if (dev_list[i] != NULL && dev_list[i]->name != NULL)
			{
				Kprintf(" %s:(%s) ", dev_list[i]->name, dev_list[i]->desc);
			}
		}
		Kprintf("\n");
	}
	should_print_known_devices = 0;
}


static char * find_iop_device(const char *dev, int *unit, iop_io_device_t **device)
{
	char canon[32];
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

	Kprintf("Unknown device '%s'\n", canon);
	print_known_devices();

	return (char *)-1;
}

iop_io_file_t *get_file(int fd)
{
	if (fd >= MAX_FILES)
		return NULL;

	if (file_table[fd].device != NULL)
		return &file_table[fd];

	return NULL;
}

iop_io_file_t *get_new_file(void)
{
	int i;
	iop_io_file_t *fd = NULL;
	int oldIntr;

	CpuSuspendIntr(&oldIntr);

	for (i = 0; i < MAX_FILES; i++)
	{
		if (!file_table[i].device)
		{
			fd = &file_table[i];

			// fill in "device" temporarily to mark the fd as allocated.
			fd->device = (iop_io_device_t *) 0xFFFFFFFF;
			break;
		}
	}

	CpuResumeIntr(oldIntr);

	if (i >= MAX_DEVICES)
	{
		print_abort_error_io("out of file descriptors", "");
		return NULL;
	}

	return fd;
}

int io_open(const char *name, int mode)
{
	iop_io_file_t *f = get_new_file();
	char *filename;
	int res;

	if (!f)
	{
		return -EMFILE;
	}

	if ((filename = find_iop_device(name, &f->unit, &f->device)) == (char *)-1)
	{
        f->device = NULL;
		return -ENODEV;
	}

	f->mode = mode;
	if ((res = f->device->ops->io_open(f, filename, mode)) >= 0)
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

int io_close(int fd)
{
	iop_io_file_t *f;
	int res;

	if ((f = get_file(fd)) == NULL)
	{
		return -EBADF;
	}

	res = f->device->ops->io_close(f);

	f->mode = 0;
	f->device = NULL;
	return res;
}

int io_read(int fd, void *ptr, size_t size)
{
	iop_io_file_t *f = get_file(fd);

	if (f == NULL || !(f->mode & FIO_O_RDONLY))
		return -EBADF;

	return f->device->ops->io_read(f, ptr, size);
}

int io_write(int fd, void *ptr, size_t size)
{
	iop_io_file_t *f = get_file(fd);

	if (f == NULL || !(f->mode & FIO_O_WRONLY))
		return -EBADF;

	return f->device->ops->io_write(f, ptr, size);
}

int io_lseek(int fd, int offset, int whence)
{
	iop_io_file_t *f = get_file(fd);

	if (f == NULL)
		return -EBADF;

	if (whence < FIO_SEEK_SET || whence > FIO_SEEK_END)
	{
		{
			const char *invalid_lssek_address_str = "invalid lseek arg\r\n";
			io_write(1, (void *)invalid_lssek_address_str, strlen(invalid_lssek_address_str));
		}
		return -EINVAL;
	}

	return f->device->ops->io_lseek(f, offset, whence);
}

int io_ioctl(int fd, unsigned long arg, void *param)
{
	iop_io_file_t *f = get_file(fd);

	if (f == NULL)
		return -EBADF;

	return f->device->ops->io_ioctl(f, arg, param);
}

int io_remove(const char *name)
{
	iop_io_file_t *f = get_new_file();
	char *filename;
	int res;

	if (!f)
	{
		return -EMFILE;
	}

	if ((filename = find_iop_device(name, &(f->unit), &(f->device))) == (char *)-1)
	{
		f->device = NULL;
		return -ENODEV;
	}

	res = f->device->ops->io_remove(f, filename);

	f->unit = 0;
	f->device = NULL;

	return res;
}

/* Because mkdir, rmdir, chdir, and sync have similiar arguments (each starts
   with a path followed by an optional integer), we use a common routine to
   handle all of them.  */
static int path_common(const char *name, int arg, int code)
{
	iop_io_file_t *f = get_new_file();
	iop_io_device_ops_t *dops;
	char *filename;
	int res = -EINVAL;

	if (!f)
	{
		return -EMFILE;
	}

	if ((filename = find_iop_device(name, &(f->unit), &(f->device))) == (char *)-1)
	{
		f->device = NULL;
		return -ENODEV;
	}

	dops = (iop_io_device_ops_t *)f->device->ops;
	switch (code) {
		case 4:		/* mkdir() */
			res = dops->io_mkdir(f, filename);
			break;
		case 5:		/* rmdir() */
			res = dops->io_rmdir(f, filename);
			break;
		default:
			break;
	}

	f->unit = 0;
	f->device = NULL;

	return res;
}

int io_mkdir(const char *name)
{
	return path_common(name, 0, 4);
}

int io_rmdir(const char *name)
{
	return path_common(name, 0, 5);
}

int io_dopen(const char *name, int mode)
{
	iop_io_file_t *f = get_new_file();
	char *filename;
	int res;

	(void)mode;

	if (!f)
		return -EMFILE;

	if ((filename = find_iop_device(name, &f->unit, &f->device)) == (char *)-1)
	{
        f->device = NULL;
		return -ENODEV;
	}

	f->mode = 0;
	if ((res = f->device->ops->io_dopen(f, filename)) >= 0)
		res = (int)(f - file_table);
	else
	{
        f->mode = 0;
        f->device = NULL;
	}

	return res;
}

int io_dclose(int fd)
{
	iop_io_file_t *f;
	int res;

	if ((f = get_file(fd)) == NULL)
	{
		return -EBADF;
	}

	res = f->device->ops->io_dclose(f);

	f->mode = 0;
	f->device = NULL;
	return res;
}

int io_dread(int fd, io_dirent_t *io_dirent)
{
    iop_io_file_t *f;
    int res;

	if ((f = get_file(fd)) == NULL)
	{
		return -EBADF;
	}

    res = f->device->ops->io_dread(f, io_dirent);

    return res;
}

int io_getstat(const char *name, io_stat_t *stat)
{
	iop_io_file_t *f = get_new_file();
	char *filename;
	int res;

	if (!f)
	{
		return -EMFILE;
	}

	if ((filename = find_iop_device(name, &(f->unit), &(f->device))) == (char *)-1)
	{
		f->device = NULL;
		return -ENODEV;
	}

	res = f->device->ops->io_getstat(f, filename, stat);

	f->unit = 0;
	f->device = NULL;

	return res;
}

int io_chstat(const char *name, io_stat_t *stat, unsigned int mask)
{
	iop_io_file_t *f = get_new_file();
	char *filename;
	int res;

	if (!f)
	{
		return -EMFILE;
	}

	if ((filename = find_iop_device(name, &(f->unit), &(f->device))) == (char *)-1)
	{
		f->device = NULL;
		return -ENODEV;
	}

	res = f->device->ops->io_chstat(f, filename, stat, mask);

	f->unit = 0;
	f->device = NULL;

    return res;
}

int io_format(const char *dev)
{
	iop_io_file_t *f = get_new_file();
	int res;

	if (!f)
	{
		return -EMFILE;
	}

	if ((find_iop_device(dev, &(f->unit), &(f->device))) == (char *)-1)
	{
		f->device = NULL;
		return -ENODEV;
	}

	res = f->device->ops->io_format(f);

	f->unit = 0;
	f->device = NULL;

	return res;
}
