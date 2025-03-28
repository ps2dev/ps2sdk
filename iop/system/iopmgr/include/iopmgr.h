/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * iop manager includes
 */

#ifndef __IOPMGR_H__
#define __IOPMGR_H__

#include <types.h>
#include <irx.h>
#ifndef __IOMANX_H__
#include <ioman.h>
#endif
#include <loadcore.h>

/* Module info entry.  */
typedef ModuleInfo_t smod_mod_info_t __attribute__ ((deprecated));	//For backward-compatibility with older projects.

/* from smod.c */
extern ModuleInfo_t *smod_get_next_mod(ModuleInfo_t *cur_mod);
extern ModuleInfo_t *smod_get_mod_by_name(const char *name);
extern int smod_get_modcount_by_name(const char *name);
extern int smod_get_modversion_by_name(const char *name);
extern int smod_unload_module(const char *name);

/* from slib.c */
extern iop_library_t *slib_get_lib_by_name(const char *name);
extern void *slib_get_exportlist_by_name(const char *name);
extern int slib_get_version_by_name(const char *name);
extern int slib_release_library(const char *name);

/* from devices.c */
extern iop_device_t *iopmgr_get_iomandev(char *device);
extern iop_device_t *iopmgr_get_iomanxdev(char *device);
extern iop_device_t *iopmgr_get_device(char *device);
extern int iopmgr_get_devicetype(char *device);
extern int iopmgr_get_devicelist(int man,int devtype,char *buffer);

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

#define iopmgr_IMPORTS_start DECLARE_IMPORT_TABLE(iopmgr, 1, 1)
#define iopmgr_IMPORTS_end END_IMPORT_TABLE

#define I_smod_get_next_mod DECLARE_IMPORT(4, smod_get_next_mod)
#define I_smod_get_mod_by_name DECLARE_IMPORT(5, smod_get_mod_by_name)
#define I_smod_get_modcount_by_name DECLARE_IMPORT(6, smod_get_modcount_by_name)
#define I_smod_get_modversion_by_name DECLARE_IMPORT(7, smod_get_modversion_by_name)
#define I_smod_unload_module DECLARE_IMPORT(8, smod_unload_module)
#define I_slib_get_lib_by_name DECLARE_IMPORT(10, slib_get_lib_by_name)
#define I_slib_get_exportlist_by_name DECLARE_IMPORT(11, slib_get_exportlist_by_name)
#define I_slib_get_version_by_name DECLARE_IMPORT(12, slib_get_version_by_name)
#define I_slib_release_library DECLARE_IMPORT(13, slib_release_library)
#define I_iopmgr_get_iomandev DECLARE_IMPORT(15, iopmgr_get_iomandev)
#define I_iopmgr_get_iomanxdev DECLARE_IMPORT(16, iopmgr_get_iomanxdev)
#define I_iopmgr_get_device DECLARE_IMPORT(17, iopmgr_get_device)
#define I_iopmgr_get_devicetype DECLARE_IMPORT(18, iopmgr_get_devicetype)
#define I_iopmgr_get_devicelist DECLARE_IMPORT(19, iopmgr_get_devicelist)

#endif /* __IOPMGR_H__ */
