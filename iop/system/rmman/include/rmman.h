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
 * RMMAN definitions
 */

#ifndef __RMMAN_H__
#define __RMMAN_H__

#include <types.h>
#include <irx.h>
#include <librm-common.h>

int rmmanInit(void);
int rmmanOpen(int port, int slot, void *buffer);
int rmmanClose(int port, int slot);
int rmmanEnd(void);

#define rmman_IMPORTS_start DECLARE_IMPORT_TABLE(rmman, 1, 1)
#define rmman_IMPORTS_end END_IMPORT_TABLE

#define I_rmmanInit DECLARE_IMPORT(4, rmmanInit)
#define I_rmmanOpen DECLARE_IMPORT(5, rmmanOpen)
#define I_rmmanClose DECLARE_IMPORT(6, rmmanClose)
#define I_rmmanEnd DECLARE_IMPORT(7, rmmanEnd)

#endif /* __RMMAN_H__ */
