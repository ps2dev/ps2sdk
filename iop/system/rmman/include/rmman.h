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

#include <types.h>
#include <librm-common.h>

#ifndef _IOP_RMMAN_H
#define _IOP_RMMAN_H

#define rmman_IMPORTS_start DECLARE_IMPORT_TABLE(rmman, 1, 1)
#define rmman_IMPORTS_end END_IMPORT_TABLE

int rmmanInit(void);
#define I_rmmanInit DECLARE_IMPORT(4, rmmanInit)

int rmmanOpen(int port, int slot, void *buffer);
#define I_rmmanOpen DECLARE_IMPORT(5, rmmanOpen)

int rmmanClose(int port, int slot);
#define I_rmmanClose DECLARE_IMPORT(6, rmmanClose)

int rmmanEnd(void);
#define I_rmmanEnd DECLARE_IMPORT(7, rmmanEnd)

#endif	// _IOP_RMMAN_H
