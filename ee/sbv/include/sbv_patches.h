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
# SBV patches.
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
