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
# Memory pool management
*/

#ifndef IOP_THPOOL_H
#define IOP_THPOOL_H

#include "types.h"
#include "irx.h"

/*
 * Fixed length memory pool
*/

typedef struct _iop_fpl_param {
	u32 attr;
	u32 unknown;
	u32 block_size;
	u32 blocks;
} iop_fpl_param;

#define thfpool_IMPORTS_start DECLARE_IMPORT_TABLE(thfpool, 1, 1)
#define thfpool_IMPORTS_end END_IMPORT_TABLE

int CreateFpl(iop_fpl_param *param);
#define I_CreateFpl DECLARE_IMPORT(4, CreateFpl)
int DeleteFpl(int fplId);
#define I_DeleteFpl DECLARE_IMPORT(5, DeleteFpl)
void *AllocateFpl(int fplId);
#define I_AllocateFpl DECLARE_IMPORT(6, AllocateFpl)
void *pAllocateFpl(int fplId);
#define I_pAllocateFpl DECLARE_IMPORT(7, pAllocateFpl)
void *ipAllocateFpl(int fplId);
#define I_ipAllocateFpl DECLARE_IMPORT(8, ipAllocateFpl)
int FreeFpl(int fplId, void *memory);
#define I_FreeFpl DECLARE_IMPORT(9, FreeFpl)

/*
 * Variable length memory pool
*/

typedef struct _iop_vpl_param {
	u32 attr;
	u32 unknown;
	u32 size;
} iop_vpl_param;

#define thvpool_IMPORTS_start DECLARE_IMPORT_TABLE(thvpool, 1, 1)
#define thvpool_IMPORTS_end END_IMPORT_TABLE

int CreateVpl(iop_vpl_param *param);
#define I_CreateVpl DECLARE_IMPORT(4, CreateVpl)
int DeleteVpl(int vplId);
#define I_DeleteVpl DECLARE_IMPORT(5, DeleteVpl)
void *AllocateVpl(int vplId, int size);
#define I_AllocateVpl DECLARE_IMPORT(6, AllocateVpl)
void *pAllocateVpl(int vplId, int size);
#define I_pAllocateVpl DECLARE_IMPORT(7, pAllocateVpl)
void *ipAllocateVpl(int vplId, int size);
#define I_ipAllocateVpl DECLARE_IMPORT(8, ipAllocateVpl)
int FreeVpl(int vplId, void *memory);
#define I_FreeVpl DECLARE_IMPORT(9, FreeVpl)

#endif
