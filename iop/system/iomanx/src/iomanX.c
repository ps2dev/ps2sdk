/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Advanced I/O library.
 */

#ifdef _IOP
#include "irx_imports.h"
#else
#include <string.h>
#include <types.h>
#define index strchr
#include <intrman.h>
#include <stdio.h>
#define Kprintf printf
#endif
#include <iomanX.h>

#include <errno.h>
#include <stdarg.h>

#ifdef IOP
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
IRX_ID("IOX/File_Manager", 1, 1);
#else
IRX_ID("IO/File_Manager", 2, 3);
// Based on the module from SCE SDK 3.1.0.
#endif
#endif
#ifdef _IOP
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
extern struct irx_export_table _exp_iomanx;
#else
extern struct irx_export_table _exp_ioman;
#endif
#endif

#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
#define MAX_DEVICES 32
#define MAX_FILES 128
#else
#define MAX_DEVICES 16
#define MAX_FILES 32
#endif

#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
void iomanX_StdioInit(int mode);
static int open_tty_handles(const char *tty_name);
#endif
static int xx_stat(int op, const char *name, iox_stat_t *stat, unsigned int statmask);
static int xx_rename(int op, const char *oldname, const char *newname);
static int xx_dir(int op, const char *name, int mode);
static int _ioabort(const char *str1, const char *str2);
static iomanX_iop_file_t *new_iob(void);
static iomanX_iop_file_t *get_iob(int fd);
static iomanX_iop_device_t *lookup_dev(const char *name, int show_unkdev_msg);
static const char *parsefile(const char *path, iomanX_iop_device_t **p_device, int *p_unit);
#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
static int tty_noop(void);
unsigned int iomanX_GetDevType(int fd);
#endif
static void ShowDrv(void);
#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
static void register_tty(void);
static void register_dummytty(void);
#endif

#ifdef IOMANX_USE_DEVICE_LINKED_LIST
struct ioman_dev_listentry
{
	struct ioman_dev_listentry *next;
	iomanX_iop_device_t *device;
};
#endif

static int showdrvflag = 1;
#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
static iomanX_iop_device_ops_t dev_tty_dev_operations = {
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	(void *)&tty_noop,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static iomanX_iop_device_t dev_tty = {
	"tty",
	IOP_DT_CHAR,
	1,
	"CONSOLE",
	&dev_tty_dev_operations,
};
static iomanX_iop_device_t dev_dummytty = {
	"dummytty",
	IOP_DT_CHAR,
	1,
	"CONSOLE",
	&dev_tty_dev_operations,
};
#endif
static int adddeldrv_in_process;
#ifdef IOMANX_USE_ERRNO
static int errno_local;
#endif
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
static struct ioman_dev_listentry *device_entry_empty_list_head;
static struct ioman_dev_listentry *device_entry_used_list_head;
#endif
#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
static
#endif
	iomanX_iop_file_t file_table[MAX_FILES];
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
static struct ioman_dev_listentry device_entry_list[MAX_DEVICES];
#else
static iomanX_iop_device_t *device_table[MAX_DEVICES];
#endif

#ifndef isnum
#define isnum(c) ((c) >= '0' && (c) <= '9')
#endif

#ifndef EUNSUP
#ifdef ENOTSUP
#define EUNSUP ENOTSUP
#else
#define EUNSUP 48
#endif
#endif

#define HANDLE_RESULT_CLEAR_INFO 1
#define HANDLE_RESULT_CLEAR_INFO_ON_ERROR 2
#define HANDLE_RESULT_RETURN_ZERO 4
#define HANDLE_RESULT_RETURN_FD 8

static inline void write_str_to_stdout(const char *in_str)
{
	iomanX_write(1, (void *)in_str, strlen(in_str));
}

static inline int set_errno(int in_errno)
{
#ifdef IOMANX_USE_ERRNO
	errno_local = in_errno;
#endif
	return -in_errno;
}

static inline void handle_result_pre(int in_result, iomanX_iop_file_t *f, int op)
{
	if ( (op & HANDLE_RESULT_CLEAR_INFO) )
	{
		if ( f )
		{
			// Unofficial: don't clear mode
			f->device = NULL;
		}
	}
	if ( (op & HANDLE_RESULT_CLEAR_INFO_ON_ERROR) )
	{
		if ( f && (in_result < 0) )
		{
			f->device = NULL;
		}
	}
}

static inline int handle_result(int in_result, iomanX_iop_file_t *f, int op)
{
	handle_result_pre(in_result, f, op);
	if ( in_result < 0 )
		return set_errno(-in_result);
	if ( (op & HANDLE_RESULT_RETURN_ZERO) )
		return 0;
	if ( (op & HANDLE_RESULT_RETURN_FD) )
		return f - file_table;
	return in_result;
}

static inline s64 handle_result64(s64 in_result, iomanX_iop_file_t *f, int op)
{
	handle_result_pre(in_result, f, op);
	if ( in_result < 0 )
		return set_errno(-(int)in_result);
	if ( (op & HANDLE_RESULT_RETURN_ZERO) )
		return 0;
	if ( (op & HANDLE_RESULT_RETURN_FD) )
		return f - file_table;
	return in_result;
}

#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
extern int hook_ioman();
extern int unhook_ioman();
#endif

#ifndef IOMANX_ENTRYPOINT
#ifdef _IOP
#define IOMANX_ENTRYPOINT _start
#else
#define IOMANX_ENTRYPOINT iomanX_start
#endif
#endif

#ifndef IOMANX_CLEANUP
#define IOMANX_CLEANUP shutdown
#endif

int IOMANX_ENTRYPOINT(int ac, char **av)
{
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	unsigned int i;
#endif

	(void)ac;
	(void)av;

#ifdef _IOP
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
	if ( RegisterLibraryEntries(&_exp_iomanx) )
		return MODULE_NO_RESIDENT_END;
#else
	if ( RegisterLibraryEntries(&_exp_ioman) )
		return MODULE_NO_RESIDENT_END;
#if 0
	SetRebootTimeLibraryHandlingMode(&_exp_ioman, 2);
#else
	// Call termination before disabling interrupts
	_exp_ioman.mode &= ~6;
	_exp_ioman.mode |= 2;
#endif
#endif
#endif
	adddeldrv_in_process = 0;
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	// Unofficial: memset instead of bzero
	memset(device_entry_list, 0, sizeof(device_entry_list));
	device_entry_used_list_head = NULL;
	device_entry_empty_list_head = device_entry_list;
	// Unofficial: link forwards instead of backwards
	for ( i = 0; i < ((sizeof(device_entry_list) / sizeof(device_entry_list[0])) - 1); i += 1 )
		device_entry_list[i].next = &device_entry_list[i + 1];
#else
	memset(device_table, 0, sizeof(device_table));
#endif
	// Unofficial: memset instead of bzero
	memset(file_table, 0, sizeof(file_table));
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
	if ( hook_ioman() )
		return MODULE_NO_RESIDENT_END;
#else
	iomanX_StdioInit(0);
#endif
	return MODULE_RESIDENT_END;
}

int IOMANX_CLEANUP(int arg)
{
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
	unhook_ioman();
	return MODULE_NO_RESIDENT_END;
#else
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	struct ioman_dev_listentry *i;
#else
	unsigned int i;
#endif

	if ( !arg )
	{
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
		for ( i = device_entry_used_list_head; i; i = i->next )
		{
			i->device->ops->deinit(i->device);
			i->device = NULL;
		}
#else
		for ( i = 0; i < (sizeof(device_table) / sizeof(device_table[0])); i += 1 )
		{
			if ( device_table[i] )
			{
				device_table[i]->ops->deinit(device_table[i]);
				device_table[i] = NULL;
			}
		}
#endif
	}
	return MODULE_RESIDENT_END;
#endif
}

#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
iomanX_iop_device_t **iomanX_GetDeviceList(void)
{
	return device_table;
}

int mode2modex(int mode)
{
	int modex = 0;

	if ( FIO_SO_ISLNK(mode) )
		modex |= FIO_S_IFLNK;
	if ( FIO_SO_ISREG(mode) )
		modex |= FIO_S_IFREG;
	if ( FIO_SO_ISDIR(mode) )
		modex |= FIO_S_IFDIR;

	/* Convert the file access modes.  */
	if ( mode & FIO_SO_IROTH )
		modex |= FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH;
	if ( mode & FIO_SO_IWOTH )
		modex |= FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH;
	if ( mode & FIO_SO_IXOTH )
		modex |= FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH;

	return modex;
}

int modex2mode(int modex)
{
	int mode = 0;

	if ( FIO_S_ISLNK(modex) )
		mode |= FIO_SO_IFLNK;
	if ( FIO_S_ISREG(modex) )
		mode |= FIO_SO_IFREG;
	if ( FIO_S_ISDIR(modex) )
		mode |= FIO_SO_IFDIR;

	/* Convert the file access modes.  */
	if ( modex & (FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH) )
		mode |= FIO_SO_IROTH;
	if ( modex & (FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH) )
		mode |= FIO_SO_IWOTH;
	if ( modex & (FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH) )
		mode |= FIO_SO_IXOTH;

	return mode;
}

iomanX_iop_file_t *get_file(int fd)
{
	return get_iob(fd);
}
#endif

#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
void iomanX_StdioInit(int mode)
{
#ifdef _IOP
	const int *BootMode;
	iop_thread_info_t thinfo;
#endif

#ifdef _IOP
	BootMode = QueryBootMode(3);
	if ( BootMode && (BootMode[1] & 4) )
		return;
	ReferThreadStatus(0, &thinfo);
	ChangeThreadPriority(0, 4);
#endif
#ifdef _IOP
	switch ( mode )
	{
		case 0:
		{
			iomanX_close(0);
			iomanX_close(1);
			register_tty();
			open_tty_handles("tty:");
			break;
		}
		case 1:
		{
			iomanX_close(0);
			iomanX_close(1);
			register_dummytty();
			open_tty_handles("dummytty:");
			break;
		}
		default:
			break;
	}
#else
	iomanX_close(0);
	iomanX_close(1);
	register_tty();
	open_tty_handles("tty:");
#endif
#ifdef _IOP
	ChangeThreadPriority(0, thinfo.currentPriority);
#endif
}

static int open_tty_handles(const char *tty_name)
{
	if ( iomanX_open(tty_name, 3) != 0 || iomanX_open(tty_name, 2) != 1 )
		return -1;
	return 0;
}
#endif

int iomanX_open(const char *name, int flags, ...)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
	int mode;
	va_list va;

	va_start(va, flags);
	mode = va_arg(va, int);
	va_end(va);
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	parsefile_res = parsefile(name, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	f->mode = flags;
	return handle_result(
		f->device->ops->open(f, parsefile_res, flags, mode),
		f,
		HANDLE_RESULT_CLEAR_INFO_ON_ERROR | HANDLE_RESULT_RETURN_FD);
}

int iomanX_lseek(int fd, int offset, int mode)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f )
		return handle_result(-EBADF, f, 0);
	switch ( mode )
	{
		case FIO_SEEK_SET:
		case FIO_SEEK_CUR:
		case FIO_SEEK_END:
			return handle_result(f->device->ops->lseek(f, offset, mode), f, 0);
		default:
			write_str_to_stdout("invalid lseek arg\r\n");
			return handle_result(-EINVAL, f, 0);
	}
}

s64 iomanX_lseek64(int fd, s64 offset, int whence)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f )
		return handle_result(-EBADF, f, 0);
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, 0);
	switch ( whence )
	{
		case FIO_SEEK_SET:
		case FIO_SEEK_CUR:
		case FIO_SEEK_END:
			return handle_result64(f->device->ops->lseek64(f, offset, whence), f, 0);
		default:
			write_str_to_stdout("invalid lseek arg\r\n");
			return handle_result(-EINVAL, f, 0);
	}
}

int iomanX_read(int fd, void *ptr, int size)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f || !(f->mode & FIO_O_RDONLY) )
		return handle_result(-EBADF, f, 0);
	return handle_result(f->device->ops->read(f, ptr, size), f, 0);
}

int iomanX_write(int fd, void *ptr, int size)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f || !(f->mode & FIO_O_WRONLY) )
		return handle_result(-EBADF, f, 0);
	return handle_result(f->device->ops->write(f, ptr, size), f, 0);
}

int iomanX_close(int fd)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f )
		return handle_result(-EBADF, f, 0);
	return handle_result(
		(f->mode & FIO_O_DIROPEN) ? f->device->ops->dclose(f) : f->device->ops->close(f),
		f,
		HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_FD);
}

int iomanX_ioctl(int fd, int cmd, void *param)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f )
		return handle_result(-EBADF, f, 0);
	return handle_result(f->device->ops->ioctl(f, cmd, param), f, 0);
}

int iomanX_ioctl2(int fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f )
		return handle_result(-EBADF, f, 0);
	// The filesystem must support these ops.
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, 0);
	return handle_result(f->device->ops->ioctl2(f, cmd, arg, arglen, buf, buflen), f, 0);
}

int iomanX_dopen(const char *path)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;

	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	parsefile_res = parsefile(path, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	f->mode = FIO_O_DIROPEN;
	return handle_result(
		f->device->ops->dopen(f, parsefile_res), f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR | HANDLE_RESULT_RETURN_FD);
}

int iomanX_dread(int fd, iox_dirent_t *buf)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f || !(f->mode & FIO_O_DIROPEN) )
		return handle_result(-EBADF, f, 0);
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
	/* If this is a legacy device (such as mc:) then we need to convert the mode
		 variable of the stat structure to iomanX's extended format.  */
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
	{
		int res;
		typedef int io_dread_t(iomanX_iop_file_t *, io_dirent_t *);
		io_dirent_t io_dirent;
		io_dread_t *io_dread;

		io_dread = (io_dread_t *)f->device->ops->dread;
		res = io_dread(f, &io_dirent);

		buf->stat.mode = mode2modex(io_dirent.stat.mode);

		buf->stat.attr = io_dirent.stat.attr;
		buf->stat.size = io_dirent.stat.size;
		memcpy(buf->stat.ctime, io_dirent.stat.ctime, sizeof(io_dirent.stat.ctime));
		memcpy(buf->stat.atime, io_dirent.stat.atime, sizeof(io_dirent.stat.atime));
		memcpy(buf->stat.mtime, io_dirent.stat.mtime, sizeof(io_dirent.stat.mtime));
		buf->stat.hisize = io_dirent.stat.hisize;

		strncpy(buf->name, io_dirent.name, sizeof(buf->name));
		return handle_result(res, f, 0);
	}
#endif
	return handle_result(f->device->ops->dread(f, buf), f, 0);
}

int iomanX_remove(const char *name)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(name, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	return handle_result(
		f->device->ops->remove(f, parsefile_res), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
}

int iomanX_mkdir(const char *path, int mode)
{
	return xx_dir(4, path, mode);
}

int iomanX_rmdir(const char *path)
{
	return xx_dir(5, path, 0);
}

static int xx_stat(int op, const char *name, iox_stat_t *stat, unsigned int statmask)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(name, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	switch ( op )
	{
		case 1:
		{
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
			/* If this is a legacy device (such as mc:) then we need to convert the mode
				 variable to iomanX's extended format.  */
			if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
			{
				iox_stat_t stat_tmp;

				memcpy(&stat_tmp, stat, sizeof(stat_tmp));
				stat_tmp.mode = modex2mode(stat->mode);
				return handle_result(
					f->device->ops->chstat(f, parsefile_res, &stat_tmp, statmask),
					f,
					HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
			}
#endif
			return handle_result(
				f->device->ops->chstat(f, parsefile_res, stat, statmask),
				f,
				HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		}
		case 2:
		{
#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
			/* If this is a legacy device (such as mc:) then we need to convert the mode
				 variable to iomanX's extended format.  */
			if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
			{
				int res;

				res = f->device->ops->getstat(f, parsefile_res, stat);
				if ( res == 0 )
					stat->mode = mode2modex(stat->mode);
				return handle_result(res, f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
			}
#endif
			return handle_result(
				f->device->ops->getstat(f, parsefile_res, stat), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		}
		default:
			// Unofficial: return negative instead of positive if op not found
			return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	}
}

int iomanX_getstat(const char *name, iox_stat_t *stat)
{
	return xx_stat(2, name, stat, 0);
}

int iomanX_chstat(const char *name, iox_stat_t *stat, unsigned int statmask)
{
	return xx_stat(1, name, stat, statmask);
}

int iomanX_format(const char *dev, const char *blockdev, void *arg, int arglen)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(dev, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	return handle_result(
		f->device->ops->format(f, parsefile_res, blockdev, arg, arglen),
		f,
		HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
}

static int xx_rename(int op, const char *oldname, const char *newname)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
	const char *parsefile_res_new;
	iomanX_iop_device_t *device_new;
	int unit_new;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(oldname, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// Unofficial: initialize variables and check if newname is not NULL
	parsefile_res_new = newname;
	device_new = f->device;
	unit_new = f->unit;
	if ( newname && index(newname, ':') )
		parsefile_res_new = parsefile(newname, &device_new, &unit_new);
	// Make sure the user isn't attempting to link across devices.
	if ( !parsefile_res_new || (device_new != f->device) || (unit_new != f->unit) )
		return handle_result(-EXDEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// The filesystem must support these ops.
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	switch ( op )
	{
		case 7:
			return handle_result(
				f->device->ops->rename(f, parsefile_res, parsefile_res_new),
				f,
				HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		case 8:
			return handle_result(
				f->device->ops->symlink(f, parsefile_res, parsefile_res_new),
				f,
				HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		default:
			// Unofficial: return negative instead of positive if op not found
			return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	}
}

// cppcheck-suppress funcArgNamesDifferent
int iomanX_rename(const char *oldname, const char *newname)
{
	return xx_rename(7, oldname, newname);
}

// cppcheck-suppress funcArgNamesDifferent
int iomanX_symlink(const char *oldname, const char *newname)
{
	return xx_rename(8, oldname, newname);
}

int iomanX_chdir(const char *name)
{
	return xx_dir(0x103, name, 0);
}

/* Because mkdir, rmdir, chdir, and sync have similiar arguments (each starts
	 with a path followed by an optional integer), we use a common routine to
	 handle all of them.  */
static int xx_dir(int op, const char *name, int mode)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(name, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// The filesystem must support these ops.
	if ( (op & 0x100) && ((f->device->type & 0xF0000000) != IOP_DT_FSEXT) )
		return handle_result(-EUNSUP, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	switch ( op )
	{
		case 4:
			return handle_result(
				f->device->ops->mkdir(f, parsefile_res, mode), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		case 5:
			return handle_result(
				f->device->ops->rmdir(f, parsefile_res), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		case 0x103:
			return handle_result(
				f->device->ops->chdir(f, parsefile_res), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		case 0x106:
			return handle_result(
				f->device->ops->sync(f, parsefile_res, mode), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
		default:
			// Unofficial: return negative instead of positive if op not found
			return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	}
}

int iomanX_sync(const char *dev, int flag)
{
	return xx_dir(0x106, dev, flag);
}

int iomanX_mount(const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(fsname, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// The filesystem must support these ops.
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	return handle_result(
		f->device->ops->mount(f, parsefile_res, devname, flag, arg, arglen),
		f,
		HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
}

int iomanX_umount(const char *fsname)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(fsname, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// The filesystem must support these ops.
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	return handle_result(
		f->device->ops->umount(f, parsefile_res), f, HANDLE_RESULT_CLEAR_INFO | HANDLE_RESULT_RETURN_ZERO);
}

int iomanX_devctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(name, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// The filesystem must support these ops.
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	return handle_result(
		f->device->ops->devctl(f, parsefile_res, cmd, arg, arglen, buf, buflen), f, HANDLE_RESULT_CLEAR_INFO);
}

int iomanX_readlink(const char *path, char *buf, unsigned int buflen)
{
	iomanX_iop_file_t *f;
	const char *parsefile_res;
#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	iomanX_iop_file_t f_stk;
#endif

#ifdef IOMAN_USE_FILE_STRUCT_TEMP_STACK
	f = &f_stk;
#else
	f = new_iob();
	if ( !f )
		return handle_result(-EMFILE, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
#endif
	parsefile_res = parsefile(path, &(f->device), &(f->unit));
	if ( !parsefile_res )
		return handle_result(-ENODEV, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	// The filesystem must support these ops.
	if ( (f->device->type & 0xF0000000) != IOP_DT_FSEXT )
		return handle_result(-EUNSUP, f, HANDLE_RESULT_CLEAR_INFO_ON_ERROR);
	return handle_result(f->device->ops->readlink(f, parsefile_res, buf, buflen), f, HANDLE_RESULT_CLEAR_INFO);
}

static int _ioabort(const char *str1, const char *str2)
{
	return Kprintf("ioabort exit:%s %s\n", str1, str2);
}

static iomanX_iop_file_t *new_iob(void)
{
	iomanX_iop_file_t *file_table_entry;
	int state;

	CpuSuspendIntr(&state);
	file_table_entry = file_table;
	while ( (file_table_entry < &file_table[sizeof(file_table) / sizeof(file_table[0])]) && file_table_entry->device )
		file_table_entry += 1;
	if ( file_table_entry >= &file_table[sizeof(file_table) / sizeof(file_table[0])] )
		file_table_entry = NULL;
	// fill in "device" temporarily to mark the fd as allocated.
	if ( file_table_entry )
		file_table_entry->device = (iomanX_iop_device_t *)(uiptr)0xFFFFFFEC;
	CpuResumeIntr(state);
	if ( !file_table_entry )
		_ioabort("out of file descriptors", "[too many open]");
	return file_table_entry;
}

static iomanX_iop_file_t *get_iob(int fd)
{
	if ( (fd < 0) || ((unsigned int)fd >= (sizeof(file_table) / sizeof(file_table[0]))) || (!file_table[fd].device) )
		return NULL;
	return &file_table[fd];
}

static iomanX_iop_device_t *lookup_dev(const char *name, int show_unkdev_msg)
{
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	struct ioman_dev_listentry *entry;
#else
	iomanX_iop_device_t *device;
	unsigned int i;
#endif
	int state;

	CpuSuspendIntr(&state);
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	entry = device_entry_used_list_head;
	while ( entry && strcmp(name, entry->device->name) )
		entry = entry->next;
	if ( !entry && show_unkdev_msg )
	{
		Kprintf("Unknown device '%s'\n", name);
		ShowDrv();
	}
#else
	device = NULL;
	for ( i = 0; i < (sizeof(device_table) / sizeof(device_table[0])); i += 1 )
	{
		if ( device_table[i] && !strcmp(name, device_table[i]->name) )
		{
			device = device_table[i];
			break;
		}
	}
	if ( !device && show_unkdev_msg )
	{
		Kprintf("Unknown device '%s'\n", name);
		ShowDrv();
	}
#endif
	CpuResumeIntr(state);
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	return entry ? entry->device : NULL;
#else
	return device;
#endif
}

static const char *parsefile(const char *path, iomanX_iop_device_t **p_device, int *p_unit)
{
	const char *path_trimmed;
	char *colon_index;
	size_t devname_len;
	iomanX_iop_device_t *device;
	int unit;
	char canon[32];

	path_trimmed = path;
	while ( *path_trimmed == ' ' )
		path_trimmed += 1;
	colon_index = index(path_trimmed, ':');
	// Unofficial: On error, return NULL instead of -1
	if ( !colon_index )
	{
		Kprintf("Unknown device '%s'\n", path_trimmed);
		return NULL;
	}
	devname_len = colon_index - path_trimmed;
	// Unofficial: bounds check
	if ( devname_len > (sizeof(canon) - 1) )
		return NULL;
	strncpy(canon, path_trimmed, devname_len);
	unit = 0;
	// Search backward for the unit number.
	while ( isnum(canon[devname_len - 1]) )
		devname_len -= 1;
	if ( isnum(canon[devname_len]) )
		unit = strtol(&canon[devname_len], 0, 10);
	canon[devname_len] = 0;
	// Find the actual device.
	device = lookup_dev(canon, 1);
	// Unofficial: On error, return NULL instead of -1
	if ( !device )
		return NULL;
	// Unofficial: set unit and device only after success
	*p_unit = unit;
	*p_device = device;
	// This is the name passed to the device op.
	return colon_index + 1;
}

// Unofficial: unused "io request for unsupported operation" func removed

#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
static int tty_noop(void)
{
	return 0;
}
#endif

int iomanX_AddDrv(iomanX_iop_device_t *device)
{
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	struct ioman_dev_listentry *entry;
	struct ioman_dev_listentry *old_head;
#else
	unsigned int i;
#endif
	int state;

	CpuSuspendIntr(&state);
	if ( adddeldrv_in_process )
	{
		Kprintf("AddDrv()/DelDrv() recursive/mutithread call error !!");
		CpuResumeIntr(state);
		return -1;
	}
	// Unofficial: move list check out of interrupt disabled area
	adddeldrv_in_process = 1;
	CpuResumeIntr(state);
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	entry = device_entry_empty_list_head;
	// Unofficial: check if entry exists first
	if ( !entry || lookup_dev(device->name, 0) )
	{
		adddeldrv_in_process = 0;
		return -1;
	}
	entry->device = device;
	device_entry_empty_list_head = entry->next;
	if ( device->ops->init(device) < 0 )
	{
		old_head = device_entry_empty_list_head;
		entry->device = NULL;
		device_entry_empty_list_head = entry;
		entry->next = old_head;
		adddeldrv_in_process = 0;
		return -1;
	}
	old_head = device_entry_used_list_head;
	device_entry_used_list_head = entry;
	entry->next = old_head;
#else
	for ( i = 0; i < (sizeof(device_table) / sizeof(device_table[0])); i += 1 )
	{
		if ( !device_table[i] )
			break;
	}

	if ( i >= (sizeof(device_table) / sizeof(device_table[0])) )
	{
		adddeldrv_in_process = 0;
		return -1;
	}

	device_table[i] = device;
#ifdef _IOP
	FlushIcache();
#endif
	if ( device->ops->init(device) < 0 )
	{
		device_table[i] = NULL;
		adddeldrv_in_process = 0;
		return -1;
	}
#endif
	showdrvflag = 1;
	adddeldrv_in_process = 0;
	return 0;
}

int iomanX_DelDrv(const char *name)
{
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	struct ioman_dev_listentry *entry;
	struct ioman_dev_listentry **p_next;
	struct ioman_dev_listentry *old_head;
#else
	unsigned int i;
#endif
	int state;

	CpuSuspendIntr(&state);
	if ( adddeldrv_in_process )
	{
		Kprintf("AddDrv()/DelDrv() recursive/mutithread call error !!");
		CpuResumeIntr(state);
		return -1;
	}
	adddeldrv_in_process = 1;
	CpuResumeIntr(state);
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	entry = device_entry_used_list_head;
	p_next = &device_entry_used_list_head;
	while ( entry && strcmp(name, entry->device->name) )
	{
		p_next = &entry->next;
		entry = entry->next;
	}
	if ( !entry || entry->device->ops->deinit(entry->device) < 0 )
	{
		adddeldrv_in_process = 0;
		return -1;
	}
	old_head = device_entry_empty_list_head;
	entry->device = NULL;
	device_entry_empty_list_head = entry;
	*p_next = entry->next;
	entry->next = old_head;
	adddeldrv_in_process = 0;
	return 0;
#else
	for ( i = 0; i < (sizeof(device_table) / sizeof(device_table[0])); i += 1 )
	{
		if ( device_table[i] && !strcmp(name, device_table[i]->name) )
		{
			device_table[i]->ops->deinit(device_table[i]);
			device_table[i] = NULL;
			adddeldrv_in_process = 0;
			return 0;
		}
	}

	adddeldrv_in_process = 0;
	return -1;
#endif
}

#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
unsigned int iomanX_GetDevType(int fd)
{
	iomanX_iop_file_t *f;

	f = get_iob(fd);
	if ( !f )
		return handle_result(-EBADF, f, 0);
	return f->device->type;
}
#endif

static void ShowDrv(void)
{
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	struct ioman_dev_listentry *i;
#else
	unsigned int i;
#endif

	if ( !showdrvflag )
		return;
	Kprintf("Known devices are ");
#ifdef IOMANX_USE_DEVICE_LINKED_LIST
	for ( i = device_entry_used_list_head; i; i = i->next )
		Kprintf(" %s:(%s) ", i->device->name, i->device->desc);
#else
	for ( i = 0; i < (sizeof(device_table) / sizeof(device_table[0])); i += 1 )
		if ( device_table[i] != NULL && device_table[i]->name != NULL )
			Kprintf(" %s:(%s) ", device_table[i]->name, device_table[i]->desc);
#endif
	Kprintf("\n");
	showdrvflag = 0;
}

#ifndef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
static void register_tty(void)
{
	iomanX_DelDrv(dev_tty.name);
	iomanX_AddDrv(&dev_tty);
}

static void register_dummytty(void)
{
	iomanX_DelDrv(dev_dummytty.name);
	iomanX_AddDrv(&dev_dummytty);
}
#endif
