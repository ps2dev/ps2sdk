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
 * Interface for System 147 multi-device device.
 */

#ifndef _S147MDEV_H
#define _S147MDEV_H

#include <ioman.h>

#ifdef __cplusplus
extern "C"
{
#endif

	extern int s147mdev_4_addfs(iop_device_t *drv, int unit10);
	extern int s147mdev_5_delfs(int unit10);

#define s147mdev_IMPORTS_start DECLARE_IMPORT_TABLE(s147mdev, 1, 1)
#define s147mdev_IMPORTS_end END_IMPORT_TABLE

#define I_s147mdev_4_addfs DECLARE_IMPORT(4, s147mdev_4_addfs)
#define I_s147mdev_5_delfs DECLARE_IMPORT(5, s147mdev_5_delfs)

#ifdef __cplusplus
}
#endif

#endif
