/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SBV patches.
 */

#ifndef __SBV_PATCHES_H__
#define __SBV_PATCHES_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @return 0: success, none-zero: error
 *
 * The rom0:LOADFILE RPC service is missing support for LoadModuleBuffer,
 * making it impossible (by default) to IOP load modules from EE RAM.
 * Newer LOADFILE modules do not have this limitation.
 * Unlike the official patch, this version is not dependent on 0x01e00000-0x01e80000.
 */
int sbv_patch_enable_lmb(void);
/**
 * @return 0: success, none-zero: error
 *
 * The MODLOAD module has a black/white (depends on version) list that determines what devices
 * can have unprotected EE/IOP executables loaded from. Typically, only protected executables
 * can be loaded from user-writable media like the memory card or HDD unit.
 * This patch will disable the black/white list, allowing executables to be freely loaded from any device.
 */
int sbv_patch_disable_prefix_check(void);
/**
 * @start address above which all user memory is cleared
 * @return 0: success, -1: error
 *
 * LoadExecPS2() wipes all user-space memory above 0x82000.  With this patch,
 * you can define a different start address to prevent your data from being
 * overwritten.  In order to completely disable the memory clear, simply pass
 * 0x02000000 to it.
 */
int sbv_patch_user_mem_clear(void *start);
/**
 * @return 0: success, none-zero: error
 *
 * The rom0:FILEIO RPC service has several glitches, which either result in stability issues or faulty behaviour:
 *   1. Interrupts are not disabled when sceSifSetDma is invoked for getstat() and dread(),
 *      which could allow simultaneous access into sceSifSetDma (a critical region).
 *   2. The RPC dispatcher code for remove() has a missing break, resulting in mkdir() being called after remove() returns.
 */
int sbv_patch_fileio(void);

#ifdef __cplusplus
}
#endif

#endif /* __SBV_PATCHES_H__ */
