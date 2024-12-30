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
 * Fixed-length memory pool management
 */

#ifndef __THFPOOL_H__
#define __THFPOOL_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fixed-length pool attributes
#define FA_THFIFO 0x000
#define FA_THPRI  0x001
#define FA_MEMBTM 0x200

/*
 * Fixed length memory pool
 */

typedef struct _iop_fpl_param
{
    unsigned int attr;
    unsigned int option;
    int block_size;
    int blocks;
} iop_fpl_param;

typedef struct _iop_fpl_info
{
    unsigned int attr;
    unsigned int option;
    int blockSize;
    int numBlocks;
    int freeBlocks;
    int numWaitThreads;
    int reserved[4];
} iop_fpl_info_t;

extern int CreateFpl(iop_fpl_param *param);
extern int DeleteFpl(int fplId);
extern void *AllocateFpl(int fplId);
extern void *pAllocateFpl(int fplId);
extern void *ipAllocateFpl(int fplId);
extern int FreeFpl(int fplId, void *memory);
extern int ReferFplStatus(int fplId, iop_fpl_info_t *info);
extern int iReferFplStatus(int fplId, iop_fpl_info_t *info);

#define thfpool_IMPORTS_start DECLARE_IMPORT_TABLE(thfpool, 1, 1)
#define thfpool_IMPORTS_end   END_IMPORT_TABLE

#define I_CreateFpl       DECLARE_IMPORT(4, CreateFpl)
#define I_DeleteFpl       DECLARE_IMPORT(5, DeleteFpl)
#define I_AllocateFpl     DECLARE_IMPORT(6, AllocateFpl)
#define I_pAllocateFpl    DECLARE_IMPORT(7, pAllocateFpl)
#define I_ipAllocateFpl   DECLARE_IMPORT(8, ipAllocateFpl)
#define I_FreeFpl         DECLARE_IMPORT(9, FreeFpl)
#define I_ReferFplStatus  DECLARE_IMPORT(11, ReferFplStatus)
#define I_iReferFplStatus DECLARE_IMPORT(12, iReferFplStatus)

#ifdef __cplusplus
}
#endif

#endif /* __THFPOOL_H__ */
