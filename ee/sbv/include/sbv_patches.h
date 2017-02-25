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

#ifndef SBV_PATCHES_H
#define SBV_PATCHES_H

#ifdef __cplusplus
extern "C" {
#endif

int sbv_patch_enable_lmb(void);
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
int sbv_patch_fileio(void);

#ifdef __cplusplus
}
#endif

#endif /* SBV_PATCHES_H */
