/* sbv_patches.h - SBV patches.
 *
 * Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This code is licensed under the Academic Free License v2.0.
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef SBV_PATCHES_H
#define SBV_PATCHES_H

#ifdef __cplusplus
extern "C" {
#endif

int sbv_patch_enable_lmb(void);

int sbv_patch_disable_prefix_check(void);

#ifdef __cplusplus
}
#endif

#endif /* SBV_PATCHES_H */
