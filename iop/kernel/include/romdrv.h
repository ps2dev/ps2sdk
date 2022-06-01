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
 * ROM file system driver routines.
 * Not available on the protokernel version of ROMDRV.
 */

#ifndef __ROMDRV_H__
#define __ROMDRV_H__

#include <irx.h>

extern int romdrv_mount(int unit, void *start);
extern int romdrv_unmount(int unit);

#define romdrv_IMPORTS_start DECLARE_IMPORT_TABLE(romdrv, 2, 1)
#define romdrv_IMPORTS_end END_IMPORT_TABLE

#define I_romdrv_mount DECLARE_IMPORT(4, romdrv_mount)
#define I_romdrv_unmount DECLARE_IMPORT(5, romdrv_unmount)

#endif /* __ROMDRV_H__ */
