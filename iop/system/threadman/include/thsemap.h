/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Kernel-based semaphores.
 */

/*
 * This file is based off of the work [RO]man, Herben, and any others involved
 * in the "modules" project at http://ps2dev.pgamers.com/.  It is also based
 * off of the work of the many contributors to the ps2lib project at
 * http://ps2dev.livemedia.com.au/.
 */

#ifndef __THSEMAP_H__
#define __THSEMAP_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

// Semaphore attributes
#define SA_THFIFO  0x000
#define SA_THPRI   0x001
#define SA_IHTHPRI 0x100

typedef struct
{
    u32 attr;
    u32 option;
    int initial;
    int max;
} iop_sema_t;

typedef struct
{
    u32 attr;
    u32 option;
    int initial;
    int max;
    int current;
    int numWaitThreads;
    int reserved[2];
} iop_sema_info_t;

extern int CreateSema(iop_sema_t *sema);
extern int DeleteSema(int semid);

extern int SignalSema(int semid);
extern int iSignalSema(int semid);
extern int WaitSema(int semid);
extern int PollSema(int semid);

extern int ReferSemaStatus(int semid, iop_sema_info_t *info);
extern int iReferSemaStatus(int semid, iop_sema_info_t *info);

#define IOP_MUTEX_LOCKED   0
#define IOP_MUTEX_UNLOCKED 1

static inline int CreateMutex(int state)
{
    iop_sema_t sema;
    sema.attr    = 0;
    sema.option  = 0;
    sema.initial = state;
    sema.max     = 1;
    return CreateSema(&sema);
}

#define thsemap_IMPORTS_start DECLARE_IMPORT_TABLE(thsemap, 1, 2)
#define thsemap_IMPORTS_end   END_IMPORT_TABLE

#define I_CreateSema       DECLARE_IMPORT(4, CreateSema)
#define I_DeleteSema       DECLARE_IMPORT(5, DeleteSema)
#define I_SignalSema       DECLARE_IMPORT(6, SignalSema)
#define I_iSignalSema      DECLARE_IMPORT(7, iSignalSema)
#define I_WaitSema         DECLARE_IMPORT(8, WaitSema);
#define I_PollSema         DECLARE_IMPORT(9, PollSema);
#define I_ReferSemaStatus  DECLARE_IMPORT(11, ReferSemaStatus)
#define I_iReferSemaStatus DECLARE_IMPORT(12, iReferSemaStatus)

#ifdef __cplusplus
}
#endif

#endif /* __THSEMAP_H__ */
