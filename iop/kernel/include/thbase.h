/*
 * thbase.h - Kernel-based threads.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This file is based off of the work [RO]man, Herben, and any others involved
 * in the "modules" project at http://ps2dev.pgamers.com/.  It is also based
 * off of the work of the many contributors to the ps2lib project at
 * http://ps2dev.livemedia.com.au/.
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef IOP_THBASE_H
#define IOP_THBASE_H

#include "types.h"
#include "irx.h"

#define TH_C		0x02000000

typedef struct _iop_thread {
	u32	attr;
	u32	option;
	void	(*thread)(void *);
	u32	stacksize;
	u32	priority;
} iop_thread_t;

/* Items returned from i/ReferThreadStatus()  */
enum _iop_thinfo {
	IOP_THINFO_ATTR, IOP_THINFO_OPTION, IOP_THINFO_THREAD = 3,
	IOP_THINFO_STACK, IOP_THINFO_STACKSIZE, IOP_THINFO_INITPRIORITY = 7,
	IOP_THINFO_PRIORITY,
	IOP_THINFO_MAX = 17
};

typedef struct {
	u32	info[IOP_THINFO_MAX];
} iop_thread_info_t;

typedef struct _iop_sys_clock {
	u32	lo, hi;
} iop_sys_clock_t;

#define thbase_IMPORTS_start DECLARE_IMPORT_TABLE(thbase, 1, 1)
#define thbase_IMPORTS_end END_IMPORT_TABLE

int CreateThread(iop_thread_t *thread);
#define I_CreateThread DECLARE_IMPORT(4, CreateThread)
int DeleteThread(int thid);
#define I_DeleteThread DECLARE_IMPORT(5, DeleteThread)

int StartThread(int thid, void *arg);
#define I_StartThread DECLARE_IMPORT(6, StartThread)
/*int StartThreadArgs(int thid, ...) */
#define I_StartThreadArgs DECLARE_IMPORT(7, StartThreadArgs)

int ExitThread();
#define I_ExitThread DECLARE_IMPORT(8, ExitThread)
int ExitDeleteThread();
#define I_ExitDeleteThread DECLARE_IMPORT(9, ExitDeleteThread)
int TerminateThread(int thid);
#define I_TerminateThread DECLARE_IMPORT(10, TerminateThread)
int iTerminateThread(int thid);
#define I_iTerminateThread DECLARE_IMPORT(11, iTerminateThread)

int ChangeThreadPriority(int thid, int priority);
#define I_ChangeThreadPriority DECLARE_IMPORT(14, ChangeThreadPriority)
int iChangeThreadPriority(int thid, int priority);
#define I_iChangeThreadPriority DECLARE_IMPORT(15, iChangeThreadPriority)

int RotateThreadReadyQueue(int priority);
#define I_RotateThreadReadyQueue DECLARE_IMPORT(16, RotateThreadReadyQueue)
int iRotateThreadReadyQueue(int priority);
#define I_iRotateThreadReadyQueue DECLARE_IMPORT(17, iRotateThreadReadyQueue)

int ReleaseWaitThread(int thid);
#define I_ReleaseWaitThread DECLARE_IMPORT(18, ReleaseWaitThread)
int iReleaseWaitThread(int thid);
#define I_iReleaseWaitThread DECLARE_IMPORT(19, iReleaseWaitThread)

int GetThreadId(void);
#define I_GetThreadId DECLARE_IMPORT(20, GetThreadId)
int ReferThreadStatus(int thid, iop_thread_info_t *info);
#define I_ReferThreadStatus DECLARE_IMPORT(22, ReferThreadStatus)
int iReferThreadStatus(int thid, iop_thread_info_t *info);
#define I_iReferThreadStatus DECLARE_IMPORT(23, iReferThreadStatus)

int SleepThread(void);
#define I_SleepThread DECLARE_IMPORT(24, SleepThread)
int WakeupThread(int thid);
#define I_WakeupThread DECLARE_IMPORT(25, WakeupThread)
int iWakeupThread(int thid);
#define I_iWakeupThread DECLARE_IMPORT(26, iWakeupThread)

int DelayThread(int usec);
#define I_DelayThread DECLARE_IMPORT(33, DelayThread)

int GetSystemTime(iop_sys_clock_t *sys_clock);
#define I_GetSystemTime DECLARE_IMPORT(34, GetSystemTime)
int SetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *), void *arg);
#define I_SetAlarm DECLARE_IMPORT(35, SetAlarm)
int iSetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *), void *arg);
#define I_iSetAlarm DECLARE_IMPORT(36, iSetAlarm)
int CancelAlarm(unsigned int (*alarm_cb)(void *), void *arg);
#define I_CancelAlarm DECLARE_IMPORT(37, CancelAlarm)
int iCancelAlarm(unsigned int (*alarm_cb)(void *), void *arg);
#define I_iCancelAlarm DECLARE_IMPORT(38, iCancelAlarm)
void USec2SysClock(u32 usec, iop_sys_clock_t *sys_clock);
#define I_USec2SysClock DECLARE_IMPORT(39, USec2SysClock)
void SysClock2USec(iop_sys_clock_t *sys_clock, u32 *sec, u32 *usec);
#define I_SysClock2USec DECLARE_IMPORT(40, SysClock2USec)

int GetSystemStatusFlag();
#define I_GetSystemStatusFlag DECLARE_IMPORT(41, GetSystemStatusFlag)

#endif /* IOP_THBASE_H */
