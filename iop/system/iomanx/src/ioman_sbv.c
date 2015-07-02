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

#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "loadcore.h"
#include "iomanX.h"
#include "sysclib.h"
#include "sys/stat.h"

#include "errno.h"

// define this to hook all IOMAN exports instead of just AddDrv and DelDrv
#define FULL_IOMAN

#define MAX_DEVICES 32
#define MAX_FILES   32

iop_device_t **dev_list;
extern iop_file_t file_table[MAX_FILES];

static const char *ioman_modname = "IO/File_Manager";

static int smod_get_next_mod(ModuleInfo_t *cur_mod, ModuleInfo_t *next_mod)
{
	void *addr;

	/* If cur_mod is 0, return the head of the list (IOP address 0x800).  */
	if (!cur_mod) {
		addr = GetLoadcoreInternalData()->image_info;
	} else {
		if (!cur_mod->next)
			return 0;
		else
			addr = cur_mod->next;
	}

	memcpy(next_mod, addr, sizeof(ModuleInfo_t));
	return next_mod->id;
}

static int smod_get_mod_by_name(const char *name, ModuleInfo_t *info)
{
	int len = strlen(name) + 1; /* Thanks to adresd for this fix.  */

	if (!smod_get_next_mod(NULL, info))
		return 0;

	do {
		if (!memcmp(info->name, name, len))
			return info->id;
	} while (smod_get_next_mod(info, info) != 0);

	return 0;
}

void fix_imports(iop_library_t *lib)
{
    struct irx_import_table *table;
    struct irx_import_stub *stub;

    FlushDcache();

    // go through each table that imports the library
    for(table = lib->caller; table != NULL; table = table->next)
    {
        // go through each import in the table
        for(stub = (struct irx_import_stub *) table->stubs; stub->jump != 0; stub++)
        {
            // patch the stub to jump to the address specified in the library export table for "fno"
            stub->jump = 0x08000000 | (((u32) lib->exports[stub->fno] << 4) >> 6);
        }
    }

    FlushIcache();
}

static u32 *ioman_exports;

static u32 Addr_IOMAN_open = 0;
static u32 Addr_IOMAN_close = 0;
static u32 Addr_IOMAN_read = 0;
static u32 Addr_IOMAN_write = 0;
static u32 Addr_IOMAN_lseek = 0;
static u32 Addr_IOMAN_ioctl = 0;
static u32 Addr_IOMAN_remove = 0;
static u32 Addr_IOMAN_mkdir = 0;
static u32 Addr_IOMAN_rmdir = 0;
static u32 Addr_IOMAN_dopen = 0;
static u32 Addr_IOMAN_dclose = 0;
static u32 Addr_IOMAN_dread = 0;
static u32 Addr_IOMAN_getstat = 0;
static u32 Addr_IOMAN_chstat = 0;
static u32 Addr_IOMAN_format = 0;

static u32 Addr_IOMAN_AddDrv = 0;
static u32 Addr_IOMAN_DelDrv = 0;

#ifndef FULL_IOMAN

/* This is called by a module wanting to add a device to legacy ioman.  */
static int sbv_AddDrv(iop_device_t *device)
{
	int res;

	/* We get first dibs!  */
	res = AddDrv(device);

	if (Addr_IOMAN_AddDrv)
        return(((int (*)(iop_device_t *device)) (Addr_IOMAN_AddDrv))(device));

	return res;
}

/* This is called by a module wanting to delete a device from legacy ioman.  */
static int sbv_DelDrv(const char *name)
{
	int res;

	res = DelDrv(name);

	if (Addr_IOMAN_DelDrv)
        return(((int (*)(const char *)) (Addr_IOMAN_DelDrv))(name));

	return res;
}
#endif

#ifdef FULL_IOMAN

// legacy ioman open and mkdir calls do not specify
// the "mode" arg.  use default of 0644 for both.

int ioman_open(const char *name, u32 flags)
{
    return(open(name, flags, 0644));
}

int ioman_mkdir(const char *name)
{
    return(mkdir(name, 0644));
}

// legacy format only takes one arg
int ioman_format(const char *dev)
{
    return(format(dev, NULL, NULL, 0));
}

iop_file_t *get_file(int fd);

int mode2modex(int mode);
int modex2mode(int mode);

static void statx2stat(iox_stat_t *iox_stat, io_stat_t* stat)
{
    stat->mode = modex2mode(iox_stat->mode);
    stat->attr = iox_stat->attr;
    stat->size = iox_stat->size;
    memcpy(stat->ctime, iox_stat->ctime, sizeof(iox_stat->ctime));
    memcpy(stat->atime, iox_stat->atime, sizeof(iox_stat->atime));
    memcpy(stat->mtime, iox_stat->mtime, sizeof(iox_stat->mtime));
    stat->hisize = iox_stat->hisize;
}

static void stat2statx(io_stat_t* stat, iox_stat_t *iox_stat)
{
    iox_stat->mode = mode2modex(stat->mode);
    iox_stat->attr = stat->attr;
    iox_stat->size = stat->size;
    memcpy(iox_stat->ctime, stat->ctime, sizeof(stat->ctime));
    memcpy(iox_stat->atime, stat->atime, sizeof(stat->atime));
    memcpy(iox_stat->mtime, stat->mtime, sizeof(stat->mtime));
    iox_stat->hisize = stat->hisize;
}

int ioman_dread(int fd, io_dirent_t *io_dirent)
{
    iop_file_t *f = get_file(fd);
    int res;

    if (f == NULL ||  !(f->mode & 8))
            return -EBADF;

    /* If this is a new device (such as pfs:) then we need to convert the mode
       variable of the stat structure to ioman's format.  */
    if ((f->device->type & 0xf0000000) == IOP_DT_FSEXT)
    {
        iox_dirent_t iox_dirent;
        res = f->device->ops->dread(f, &iox_dirent);

        statx2stat(&iox_dirent.stat, &io_dirent->stat);

        strncpy(io_dirent->name, iox_dirent.name, sizeof(iox_dirent.name));
    }
    else
    {
        typedef int	io_dread_t(iop_file_t *, io_dirent_t *);
        io_dread_t *io_dread = (io_dread_t*) f->device->ops->dread;
        res = io_dread(f, io_dirent);
    }

    return res;
}

int ioman_getstat(const char *name, io_stat_t *stat)
{
    iox_stat_t iox_stat;
    int res = getstat(name, &iox_stat);
    if (res == 0)
        statx2stat(&iox_stat, stat);
    return res;
}

int ioman_chstat(const char *name, io_stat_t *stat, unsigned int mask)
{
    iox_stat_t iox_stat;
    stat2statx(stat, &iox_stat);
    return chstat(name, &iox_stat, mask);
}

#endif

int hook_ioman(void)
{
	iop_library_t ioman_library = { NULL, NULL, 0x102, 0, "ioman\0\0" };
	ModuleInfo_t info;

    dev_list = GetDeviceList();

	if (smod_get_mod_by_name(ioman_modname, &info))
	{
        // steal the original IOMAN's 16 registered device entries
        memcpy(dev_list, (void *) (info.text_start + info.text_size + info.data_size + 0x10), sizeof(iop_device_t *) * 16);

        // steal the original IOMAN's 16 file descriptors
        memcpy(file_table, (void *) (info.text_start + info.text_size + info.data_size + 0x50), sizeof(iop_file_t) * 16);
	}
	else { return(-1); }

    // patch the IOMAN export library table to call iomanX functions
	if ((ioman_exports = (u32 *)QueryLibraryEntryTable(&ioman_library)) != NULL)
	{
    	/* Preserve ioman's library exports.  */
        Addr_IOMAN_open = ioman_exports[4];
        Addr_IOMAN_close = ioman_exports[5];
        Addr_IOMAN_read = ioman_exports[6];
        Addr_IOMAN_write = ioman_exports[7];
        Addr_IOMAN_lseek = ioman_exports[8];
        Addr_IOMAN_ioctl = ioman_exports[9];
        Addr_IOMAN_remove = ioman_exports[10];
        Addr_IOMAN_mkdir = ioman_exports[11];
        Addr_IOMAN_rmdir = ioman_exports[12];
        Addr_IOMAN_dopen = ioman_exports[13];
        Addr_IOMAN_dclose = ioman_exports[14];
        Addr_IOMAN_dread = ioman_exports[15];
        Addr_IOMAN_getstat = ioman_exports[16];
        Addr_IOMAN_chstat = ioman_exports[17];
        Addr_IOMAN_format = ioman_exports[18];
        Addr_IOMAN_AddDrv = ioman_exports[20];
        Addr_IOMAN_DelDrv = ioman_exports[21];

#ifdef FULL_IOMAN
		ioman_exports[4] = (u32) ioman_open;
		ioman_exports[5] = (u32) close;
		ioman_exports[6] = (u32) read;
		ioman_exports[7] = (u32) write;
		ioman_exports[8] = (u32) lseek;
		ioman_exports[9] = (u32) ioctl;
		ioman_exports[10] = (u32) remove;
		ioman_exports[11] = (u32) ioman_mkdir;
		ioman_exports[12] = (u32) rmdir;
		ioman_exports[13] = (u32) dopen;
		ioman_exports[14] = (u32) close;
		ioman_exports[15] = (u32) ioman_dread;
		ioman_exports[16] = (u32) ioman_getstat;
		ioman_exports[17] = (u32) ioman_chstat;
		ioman_exports[18] = (u32) ioman_format;
		ioman_exports[20] = (u32) AddDrv;
		ioman_exports[21] = (u32) DelDrv;
#else
		ioman_exports[20] = (u32) sbv_AddDrv;
		ioman_exports[21] = (u32) sbv_DelDrv;
#endif

        // repair all the tables that import the ioman library
        fix_imports((iop_library_t *) (((u32) ioman_exports) - 0x14));
	}
	else { return(-2); }

	return(0);
}

int unhook_ioman()
{
	int i;

    dev_list = GetDeviceList();

	/* Remove all registered devices.  */
	for (i = 0; i < MAX_DEVICES; i++) {
		if (dev_list[i] != NULL) {
			dev_list[i]->ops->deinit(dev_list[i]);
			dev_list[i] = NULL;
		}
	}

	/* Restore ioman's library exports.  */
    ioman_exports[4] = Addr_IOMAN_open;
    ioman_exports[5] = Addr_IOMAN_close;
    ioman_exports[6] = Addr_IOMAN_read;
    ioman_exports[7] = Addr_IOMAN_write;
    ioman_exports[8] = Addr_IOMAN_lseek;
    ioman_exports[9] = Addr_IOMAN_ioctl;
    ioman_exports[10] = Addr_IOMAN_remove;
    ioman_exports[11] = Addr_IOMAN_mkdir;
    ioman_exports[12] = Addr_IOMAN_rmdir;
    ioman_exports[13] = Addr_IOMAN_dopen;
    ioman_exports[14] = Addr_IOMAN_dclose;
    ioman_exports[15] = Addr_IOMAN_dread;
    ioman_exports[16] = Addr_IOMAN_getstat;
    ioman_exports[17] = Addr_IOMAN_chstat;
    ioman_exports[18] = Addr_IOMAN_format;

    ioman_exports[20] = Addr_IOMAN_AddDrv;
    ioman_exports[21] = Addr_IOMAN_DelDrv;

    // repair all the tables that import the ioman library
    fix_imports((iop_library_t *) (((u32) ioman_exports) - 0x14));

	return 0;
}
