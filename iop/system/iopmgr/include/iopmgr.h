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
# iop manager includes
*/

#ifndef _IOP_IOPMGR_H
#define _IOP_IOPMGR_H

#include "types.h"
#include "irx.h"
#include "loadcore.h"

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

#define iopmgr_IMPORTS_start DECLARE_IMPORT_TABLE(iopmgr, 1, 1)
#define iopmgr_IMPORTS_end END_IMPORT_TABLE

/* from smod.c */
smod_mod_info_t *smod_get_next_mod(smod_mod_info_t *cur_mod);
#define I_smod_get_next_mod DECLARE_IMPORT(4, smod_get_next_mod)
smod_mod_info_t *smod_get_mod_by_name(const char *name);
#define I_smod_get_mod_by_name DECLARE_IMPORT(5, smod_get_mod_by_name)
int smod_get_modcount_by_name(const char *name);
#define I_smod_get_modcount_by_name DECLARE_IMPORT(6, smod_get_modcount_by_name)
int smod_get_modversion_by_name(const char *name);
#define I_smod_get_modversion_by_name DECLARE_IMPORT(7, smod_get_modversion_by_name)
int smod_unload_module(const char *name);
#define I_smod_unload_module DECLARE_IMPORT(8, smod_unload_module)

/* from slib.c */
iop_library_t *slib_get_lib_by_name(const char *name);
#define I_slib_get_lib_by_name DECLARE_IMPORT(10, slib_get_lib_by_name)
void *slib_get_exportlist_by_name(const char *name);
#define I_slib_get_exportlist_by_name DECLARE_IMPORT(11, slib_get_exportlist_by_name)
int slib_get_version_by_name(const char *name);
#define I_slib_get_version_by_name DECLARE_IMPORT(12, slib_get_version_by_name)
int slib_release_library(const char *name);
#define I_slib_release_library DECLARE_IMPORT(13, slib_release_library)


/* from devices.c */
iop_device_t *iopmgr_get_iomandev(char *device);
#define I_iopmgr_get_iomandev DECLARE_IMPORT(15, iopmgr_get_iomandev)
iop_device_t *iopmgr_get_iomanxdev(char *device);
#define I_iopmgr_get_iomanxdev DECLARE_IMPORT(16, iopmgr_get_iomanxdev)
iop_device_t *iopmgr_get_device(char *device);
#define I_iopmgr_get_device DECLARE_IMPORT(17, iopmgr_get_device)
int iopmgr_get_devicetype(char *device);
#define I_iopmgr_get_devicetype DECLARE_IMPORT(18, iopmgr_get_devicetype)
int iopmgr_get_devicelist(int man,int devtype,char *buffer);
#define I_iopmgr_get_devicelist DECLARE_IMPORT(19, iopmgr_get_devicelist)

/* from modules.c */

/* These are the types returned by the devicetype call as return values. */
#define IOPMGR_DEVTYPE_INVALID  0
#define IOPMGR_DEVTYPE_IOMAN    1
#define IOPMGR_DEVTYPE_IOMANX   2
#define IOPMGR_DEVTYPE_ALL      0xffff

/* iomgr module identifier */
#define IOPMGR_MODNAME        "IOP_Manager"
#define IOPMGR_LIBNAME        "iopmgr"

/* iomgr module version number, so programs can check required version is installed */
#define IOPMGR_VERSION_HIGH 1
#define IOPMGR_VERSION_LOW  1

/* defines for ioman and iomanx module identifiers , so we can locate them */
#define IOPMGR_IOMAN_IDENT  "IO/File_Manager"
#define IOPMGR_IOMANX_IDENT "IOX/File_Manager"

#endif
