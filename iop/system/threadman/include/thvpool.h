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
 * Variable-length memory pool management
 */

#ifndef __THVPOOL_H__
#define __THVPOOL_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Variable length memory pool
 */

// Variable-length pool attributes
#define VA_THFIFO 0x000
#define VA_THPRI  0x001
#define VA_MEMBTM 0x200

typedef struct _iop_vpl_param
{
    unsigned int attr;
    unsigned int option;
    int size;
} iop_vpl_param;

typedef struct _iop_vpl_info
{
    unsigned int attr;
    unsigned int option;
    int size;
    int freeSize;
    int numWaitThreads;
    int reserved[3];
} iop_vpl_info_t;

extern int CreateVpl(iop_vpl_param *param);
extern int DeleteVpl(int vplId);
extern void *AllocateVpl(int vplId, int size);
extern void *pAllocateVpl(int vplId, int size);
extern void *ipAllocateVpl(int vplId, int size);
extern int FreeVpl(int vplId, void *memory);
extern int ReferVplStatus(int vplId, iop_vpl_info_t *info);
extern int iReferVplStatus(int vplId, iop_vpl_info_t *info);

#define thvpool_IMPORTS_start DECLARE_IMPORT_TABLE(thvpool, 1, 1)
#define thvpool_IMPORTS_end   END_IMPORT_TABLE

#define I_CreateVpl       DECLARE_IMPORT(4, CreateVpl)
#define I_DeleteVpl       DECLARE_IMPORT(5, DeleteVpl)
#define I_AllocateVpl     DECLARE_IMPORT(6, AllocateVpl)
#define I_pAllocateVpl    DECLARE_IMPORT(7, pAllocateVpl)
#define I_ipAllocateVpl   DECLARE_IMPORT(8, ipAllocateVpl)
#define I_FreeVpl         DECLARE_IMPORT(9, FreeVpl)
#define I_ReferVplStatus  DECLARE_IMPORT(11, ReferVplStatus)
#define I_iReferVplStatus DECLARE_IMPORT(12, iReferVplStatus)

#ifdef __cplusplus
}
#endif

#endif /* __THVPOOL_H__ */
