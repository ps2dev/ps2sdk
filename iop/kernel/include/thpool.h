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
 * Memory pool management
 */

#ifndef __THPOOL_H__
#define __THPOOL_H__

#include <types.h>
#include <irx.h>

//Fixed-length pool attributes
#define FA_THFIFO	0x000
#define FA_THPRI	0x001
#define FA_MEMBTM	0x200

/*
 * Fixed length memory pool
*/

typedef struct _iop_fpl_param {
	unsigned int attr;
	unsigned int option;
	int block_size;
	int blocks;
} iop_fpl_param;

typedef struct _iop_fpl_info {
	unsigned int attr;
	unsigned int option;
	int blockSize;
	int numBlocks;
	int freeBlocks;
	int numWaitThreads;
	int reserved[4];
} iop_fpl_info_t;

int CreateFpl(iop_fpl_param *param);
int DeleteFpl(int fplId);
void *AllocateFpl(int fplId);
void *pAllocateFpl(int fplId);
void *ipAllocateFpl(int fplId);
int FreeFpl(int fplId, void *memory);
int ReferFplStatus(int fplId, iop_fpl_info_t *info);
int iReferFplStatus(int fplId, iop_fpl_info_t *info);

#define thfpool_IMPORTS_start DECLARE_IMPORT_TABLE(thfpool, 1, 1)
#define thfpool_IMPORTS_end END_IMPORT_TABLE

#define I_CreateFpl DECLARE_IMPORT(4, CreateFpl)
#define I_DeleteFpl DECLARE_IMPORT(5, DeleteFpl)
#define I_AllocateFpl DECLARE_IMPORT(6, AllocateFpl)
#define I_pAllocateFpl DECLARE_IMPORT(7, pAllocateFpl)
#define I_ipAllocateFpl DECLARE_IMPORT(8, ipAllocateFpl)
#define I_FreeFpl DECLARE_IMPORT(9, FreeFpl)
#define I_ReferFplStatus DECLARE_IMPORT(11, ReferFplStatus)
#define I_iReferFplStatus DECLARE_IMPORT(12, iReferFplStatus)

/*
 * Variable length memory pool
*/

//Variable-length pool attributes
#define VA_THFIFO	0x000
#define VA_THPRI	0x001
#define VA_MEMBTM	0x200

typedef struct _iop_vpl_param {
	unsigned int attr;
	unsigned int option;
	int size;
} iop_vpl_param;

typedef struct _iop_vpl_info {
	unsigned int attr;
	unsigned int option;
	int size;
	int freeSize;
	int numWaitThreads;
	int reserved[3];
} iop_vpl_info_t;

int CreateVpl(iop_vpl_param *param);
int DeleteVpl(int vplId);
void *AllocateVpl(int vplId, int size);
void *pAllocateVpl(int vplId, int size);
void *ipAllocateVpl(int vplId, int size);
int FreeVpl(int vplId, void *memory);
int ReferVplStatus(int vplId, iop_vpl_info_t *info);
int iReferVplStatus(int vplId, iop_vpl_info_t *info);

#define thvpool_IMPORTS_start DECLARE_IMPORT_TABLE(thvpool, 1, 1)
#define thvpool_IMPORTS_end END_IMPORT_TABLE

#define I_CreateVpl DECLARE_IMPORT(4, CreateVpl)
#define I_DeleteVpl DECLARE_IMPORT(5, DeleteVpl)
#define I_AllocateVpl DECLARE_IMPORT(6, AllocateVpl)
#define I_pAllocateVpl DECLARE_IMPORT(7, pAllocateVpl)
#define I_ipAllocateVpl DECLARE_IMPORT(8, ipAllocateVpl)
#define I_FreeVpl DECLARE_IMPORT(9, FreeVpl)
#define I_ReferVplStatus DECLARE_IMPORT(11, ReferVplStatus)
#define I_iReferVplStatus DECLARE_IMPORT(12, iReferVplStatus)

#endif /* __THPOOL_H__ */
