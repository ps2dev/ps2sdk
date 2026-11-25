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
 * Advanced I/O library.
 */

#ifdef IOMANX_ENABLE_LEGACY_IOMAN_HOOK
#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "loadcore.h"
#include "iomanX.h"
#include "sysclib.h"

#include "errno.h"

// define this to hook all IOMAN exports instead of just AddDrv and DelDrv
#define FULL_IOMAN

#define MAX_DEVICES 32
#define MAX_FILES   32

iomanX_iop_device_t **dev_list;
extern iomanX_iop_file_t file_table[MAX_FILES];

static const char *ioman_modname = "IO/File_Manager";

static int smod_get_next_mod(ModuleInfo_t *cur_mod, ModuleInfo_t *next_mod)
{
	void *addr;

	/* If cur_mod is 0, return the head of the list (IOP address 0x800).  */
	addr = !cur_mod ? GetLoadcoreInternalData()->image_info : cur_mod->next;
	if (!addr)
		return 0;

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

#ifndef FULL_IOMAN
static u32 Addr_IOMAN_AddDrv = 0;
static u32 Addr_IOMAN_DelDrv = 0;
#endif

#ifndef FULL_IOMAN

/* This is called by a module wanting to add a device to legacy ioman.  */
static int sbv_AddDrv(iomanX_iop_device_t *device)
{
	int res;

	/* We get first dibs!  */
	res = AddDrv(device);

	if (Addr_IOMAN_AddDrv)
        return(((int (*)(iomanX_iop_device_t *device)) (Addr_IOMAN_AddDrv))(device));

	return res;
}

/* This is called by a module wanting to delete a device from legacy ioman.  */
static int sbv_DelDrv(const char *name)
{
	int res;

	res = DelDrv(name);

	if (Addr_IOMAN_DelDrv)
        return(((int (*)(const char *name)) (Addr_IOMAN_DelDrv))(name));

	return res;
}
#endif

#ifdef FULL_IOMAN

// legacy ioman open and mkdir calls do not specify
// the "mode" arg.  use default of 0644 for open and 0755 for mkdir.

int ioman_open(const char *name, u32 flags)
{
    return(iomanX_open(name, flags, 0644));
}

int ioman_mkdir(const char *name)
{
    return(iomanX_mkdir(name, 0755));
}

// legacy format only takes one arg
int ioman_format(const char *dev)
{
    return(iomanX_format(dev, NULL, NULL, 0));
}

iomanX_iop_file_t *get_file(int fd);

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
    iomanX_iop_file_t *f = get_file(fd);
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
        typedef int	io_dread_t(iomanX_iop_file_t *, io_dirent_t *);
        io_dread_t *io_dread = (io_dread_t*) f->device->ops->dread;
        res = io_dread(f, io_dirent);
    }

    return res;
}

int ioman_getstat(const char *name, io_stat_t *stat)
{
    iox_stat_t iox_stat;
    int res = iomanX_getstat(name, &iox_stat);
    if (res == 0)
        statx2stat(&iox_stat, stat);
    return res;
}

int ioman_chstat(const char *name, io_stat_t *stat, unsigned int mask)
{
    iox_stat_t iox_stat;
    stat2statx(stat, &iox_stat);
    return iomanX_chstat(name, &iox_stat, mask);
}

#endif

extern struct irx_export_table _exp_ioman;

int hook_ioman(void)
{
	iop_library_t ioman_library = { NULL, NULL, 0x102, 0, "ioman\0\0" };
	ModuleInfo_t info;

    /* If we already hooked, don't hook again */
    if (ioman_exports)
        return -1;

    /* Query if ioman libary exists, otherwise register our own */
    /* By having a higher version, libraries will relink to the new entry table in RegisterLibraryEntries. */
    if (!smod_get_mod_by_name(ioman_modname, &info) || !(ioman_exports = (u32 *)QueryLibraryEntryTable(&ioman_library)))
    {
        int ret;
        u16 old_version;

        old_version = _exp_ioman.version;
        _exp_ioman.version |= 0xFF;
        ret = RegisterLibraryEntries(&_exp_ioman);
        _exp_ioman.version = old_version;
        return ret;
    }

    // patch the IOMAN export library table to call iomanX functions
#ifdef FULL_IOMAN
    /* Replace ioman library entries with our updated ones. */
    /* By having a higher version, libraries will relink to the new entry table in RegisterLibraryEntries. */
    {
        int ret;
        u16 old_version;

        old_version = _exp_ioman.version;
        _exp_ioman.version |= 0xFF;
        ret = RegisterLibraryEntries(&_exp_ioman);
        _exp_ioman.version = old_version;
        if ( ret )
            return ret;
    }
    /* Prepare the old table for restoration inside unhook_ioman. */
    ReleaseLibraryEntries((struct irx_export_table *) (((uiptr) ioman_exports) - 0x14));
#else
    /* Preserve ioman's library exports.  */
    Addr_IOMAN_AddDrv = ioman_exports[20];
    Addr_IOMAN_DelDrv = ioman_exports[21];

    ioman_exports[20] = (u32) sbv_AddDrv;
    ioman_exports[21] = (u32) sbv_DelDrv;

    /* repair all the tables that import the ioman library */
    fix_imports((iop_library_t *)(((uiptr) ioman_exports) - 0x14));
#endif

    dev_list = iomanX_GetDeviceList();
    /* steal the original IOMAN's 16 registered device entries */
    memcpy(dev_list, (void *) (info.text_start + info.text_size + info.data_size + 0x10), sizeof(iomanX_iop_device_t *) * 16);
#ifdef FULL_IOMAN
    /* ... and clear the original so they don't get double-deinitialized */
    memset((void *) (info.text_start + info.text_size + info.data_size + 0x10), 0, sizeof(iomanX_iop_device_t *) * 16);
#endif

    /* steal the original IOMAN's 16 file descriptors */
    memcpy(file_table, (void *) (info.text_start + info.text_size + info.data_size + 0x50), sizeof(iomanX_iop_file_t) * 16);
#ifdef FULL_IOMAN
    /* ... and clear the original so they don't get double-deinitialized */
    memset((void *) (info.text_start + info.text_size + info.data_size + 0x50), 0, sizeof(iomanX_iop_file_t) * 16);
#endif

	return 0;
}

int unhook_ioman(void)
{
#ifdef FULL_IOMAN
    if (ioman_exports)
    {
        ReleaseLibraryEntries(&_exp_ioman);
        RegisterLibraryEntries((struct irx_export_table *) (((uiptr) ioman_exports) - 0x14));
    }
#else
    if (!ioman_exports)
        return -2;
	/* Restore ioman's library exports.  */
    ioman_exports[20] = Addr_IOMAN_AddDrv;
    ioman_exports[21] = Addr_IOMAN_DelDrv;

    // repair all the tables that import the ioman library
    fix_imports((iop_library_t *) (((uiptr) ioman_exports) - 0x14));
#endif
    ioman_exports = NULL;

	return 0;
}
#endif
