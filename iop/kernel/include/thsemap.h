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
# Kernel-based semaphores.
*/

/*
 * This file is based off of the work [RO]man, Herben, and any others involved
 * in the "modules" project at http://ps2dev.pgamers.com/.  It is also based
 * off of the work of the many contributors to the ps2lib project at
 * http://ps2dev.livemedia.com.au/.
 */

#ifndef IOP_THSEMAP_H
#define IOP_THSEMAP_H

#include "types.h"
#include "irx.h"

typedef struct {
	u32	attr;
	u32	option;
	int	initial;
	int	max;
} iop_sema_t;

typedef struct {
	u32 attr;
	u32 option;
	int initial;
	int max;
	int current;
	int unknown[3];
} iop_sema_info_t;

#define thsemap_IMPORTS_start DECLARE_IMPORT_TABLE(thsemap, 1, 2)
#define thsemap_IMPORTS_end END_IMPORT_TABLE

int CreateSema(iop_sema_t *sema);
#define I_CreateSema DECLARE_IMPORT(4, CreateSema)
int DeleteSema(int semid);
#define I_DeleteSema DECLARE_IMPORT(5, DeleteSema)

int SignalSema(int semid);
#define I_SignalSema DECLARE_IMPORT(6, SignalSema)
int iSignalSema(int semid);
#define I_iSignalSema DECLARE_IMPORT(7, iSignalSema)
int WaitSema(int semid);
#define I_WaitSema DECLARE_IMPORT(8, WaitSema);
int PollSema(int semid);
#define I_PollSema DECLARE_IMPORT(9, PollSema);

int ReferSemaStatus(int semid, iop_sema_info_t *info);
#define I_ReferSemaStatus DECLARE_IMPORT(11, ReferSemaStatus)
int iReferSemaStatus(int semid, iop_sema_info_t *info);
#define I_iReferSemaStatus DECLARE_IMPORT(12, iReferSemaStatus)

#define IOP_MUTEX_LOCKED	0
#define IOP_MUTEX_UNLOCKED	1

static inline int CreateMutex(int state)
{
	iop_sema_t sema;
	sema.attr = 0;
	sema.option = 0;
	sema.initial = state;
	sema.max = 1;
	return CreateSema(&sema);
}

#endif /* IOP_THSEMAP_H */
