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
 * Definitions and imports for cdvdfsv
 */

#ifndef __CDVDFSV_H__
#define __CDVDFSV_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

int sceCdChangeThreadPriority(int priority);

#define cdvdfsv_IMPORTS_start DECLARE_IMPORT_TABLE(cdvdfsv, 1, 1)
#define cdvdfsv_IMPORTS_end END_IMPORT_TABLE

#define I_sceCdChangeThreadPriority DECLARE_IMPORT(5, sceCdChangeThreadPriority)

#ifdef __cplusplus
}
#endif

#endif /* __CDVDFSV_H__ */
