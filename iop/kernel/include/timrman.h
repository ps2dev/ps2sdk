/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2005 linuzappz <linuzappz@hotmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Timer manager.
*/

#ifndef IOP_TIMRMAN_H
#define IOP_TIMRMAN_H

#include "types.h"
#include "irx.h"

#define timrman_IMPORTS_start DECLARE_IMPORT_TABLE(timrman, 1, 1)
#define timrman_IMPORTS_end END_IMPORT_TABLE

int  AllocHardTimer(int source, int size, int prescale);
#define I_AllocHardTimer DECLARE_IMPORT(4, AllocHardTimer)
int  ReferHardTimer(int source, int size, int mode, int modemask);
#define I_ReferHardTimer DECLARE_IMPORT(5, ReferHardTimer)
int  FreeHardTimer(int timid);
#define I_FreeHardTimer DECLARE_IMPORT(6, FreeHardTimer)

void SetTimerMode(int timid, int mode);
#define I_SetTimerMode DECLARE_IMPORT(7, SetTimerMode)

u32  GetTimerStatus(int timid);
#define I_GetTimerStatus DECLARE_IMPORT(8, GetTimerStatus)

void SetTimerCounter(int timid, u32 count);
#define I_SetTimerCounter DECLARE_IMPORT(9, SetTimerCounter)
u32  GetTimerCounter(int timid);
#define I_GetTimerCounter DECLARE_IMPORT(10, GetTimerCounter)

void SetTimerCompare(int timid, u32 compare);
#define I_SetTimerCompare DECLARE_IMPORT(11, SetTimerCompare)
u32  GetTimerCompare(int timid);
#define I_GetTimerCompare DECLARE_IMPORT(12, GetTimerCompare)

void SetHoldMode(int holdnum, int mode);
#define I_SetHoldMode DECLARE_IMPORT(13, SetHoldMode)
u32  GetHoldMode(int holdnum);
#define I_GetHoldMode DECLARE_IMPORT(14, GetHoldMode)

u32  GetHoldReg(int holdnum);
#define I_GetHoldReg DECLARE_IMPORT(15, GetHoldReg)

int  GetHardTimerIntrCode(int timid);
#define I_GetHardTimerIntrCode DECLARE_IMPORT(16, GetHardTimerIntrCode)


#define timrman_IMPORTS \
	timrman_IMPORTS_start \
 \
 	I_AllocHardTimer \
	I_ReferHardTimer \
	I_FreeHardTimer \
 \
	I_SetTimerMode \
 \
	I_GetTimerStatus \
 \
	I_SetTimerCounter \
	I_GetTimerCounter \
 \
	I_SetTimerCompare \
	I_GetTimerCompare \
 \
	I_SetHoldMode \
	I_GetHoldMode \
 \
	I_GetHardTimerIntrCode \
 \
	timrman_IMPORTS_end END_IMPORT_TABLE

#endif /* IOP_TIMRMAN_H */
