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
 * Kernel-based threads.
 */

#ifndef __THBASE_H__
#define __THBASE_H__

#include <types.h>
#include <irx.h>

// Thread attribute definitions
#define TH_ASM          0x01000000
#define TH_C            0x02000000
#define TH_UMODE        0x00000008
#define TH_NO_FILLSTACK 0x00100000
#define TH_CLEAR_STACK  0x00200000

// Thread priority definitions
#define HIGHEST_PRIORITY      1
#define USER_HIGHEST_PRIORITY 9
#define USER_LOWEST_PRIORITY  123
#define LOWEST_PRIORITY       126

typedef struct _iop_thread
{
    u32 attr;
    u32 option;
    void (*thread)(void *);
    u32 stacksize;
    u32 priority;
} iop_thread_t;

/** Special thread ID for referring to the running thread. Not supported by all functions. */
#define TH_SELF 0

// Thread status definitions
#define THS_RUN         0x01
#define THS_READY       0x02
#define THS_WAIT        0x04
#define THS_SUSPEND     0x08
#define THS_WAITSUSPEND 0x0C
#define THS_DORMANT     0x10

// Thread wait status definitions
#define TSW_SLEEP     1
#define TSW_DELAY     2
#define TSW_SEMA      3
#define TSW_EVENTFLAG 4
#define TSW_MBX       5
#define TSW_VPL       6
#define TSW_FPL       7

typedef struct _iop_thread_status
{
    unsigned int attr;
    unsigned int option;
    int status;
    void *entry;
    void *stack;
    int stackSize;
    void *gpReg;
    int initPriority;
    int currentPriority;
    int waitType;
    int waitId;
    int wakeupCount;
    /** Only valid for use with iReferThreadStatus. */
    long int *regContext; //
    unsigned int reserved[4];
} iop_thread_info_t;

typedef struct _iop_sys_clock
{
    u32 lo, hi;
} iop_sys_clock_t;

typedef struct _iop_thread_run_status
{
    int status;
    int currentPriority;
    int waitType;
    int waitId;
    int wakeupCount;
    long int *regContext;
    iop_sys_clock_t runClocks;
    unsigned int intrPreemptCount;
    unsigned int threadPreemptCount;
    unsigned int releaseCount;
} iop_thread_run_status_t;

typedef struct _iop_sys_status
{
    unsigned int status;
    int systemLowTimerWidth;
    iop_sys_clock_t idleClocks;
    iop_sys_clock_t kernelClocks;
    unsigned int comesOutOfIdleCount;
    unsigned int threadSwitchCount;
    unsigned int reserved[8];
} iop_sys_status_t;

int CreateThread(iop_thread_t *thread);
int DeleteThread(int thid);

int StartThread(int thid, void *arg);
int StartThreadArgs(int thid, int args, void *argp);

int ExitThread();
int ExitDeleteThread();
int TerminateThread(int thid);
int iTerminateThread(int thid);

int DisableDispatchThread(void);
int EnableDispatchThread(void);

int ChangeThreadPriority(int thid, int priority);
int iChangeThreadPriority(int thid, int priority);

int RotateThreadReadyQueue(int priority);
int iRotateThreadReadyQueue(int priority);

int ReleaseWaitThread(int thid);
int iReleaseWaitThread(int thid);

int GetThreadId(void);
int CheckThreadStack(void);
int ReferThreadStatus(int thid, iop_thread_info_t *info);
int iReferThreadStatus(int thid, iop_thread_info_t *info);

int SleepThread(void);
int WakeupThread(int thid);
int iWakeupThread(int thid);
int CancelWakeupThread(int thid);
int iCancelWakeupThread(int thid);

int SuspendThread(int thid);
int iSuspendThread(int thid);
int ResumeThread(int thid);
int iResumeThread(int thid);

int DelayThread(int usec);

int GetSystemTime(iop_sys_clock_t *sys_clock);
int SetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *), void *arg);
int iSetAlarm(iop_sys_clock_t *sys_clock, unsigned int (*alarm_cb)(void *), void *arg);
int CancelAlarm(unsigned int (*alarm_cb)(void *), void *arg);
int iCancelAlarm(unsigned int (*alarm_cb)(void *), void *arg);
void USec2SysClock(u32 usec, iop_sys_clock_t *sys_clock);
void SysClock2USec(iop_sys_clock_t *sys_clock, u32 *sec, u32 *usec);

int GetSystemStatusFlag();

// clang-format off
#define thbase_IMPORTS \
	thbase_IMPORTS_start \
 \
 	I_CreateThread \
	I_DeleteThread \
 \
 	I_StartThread \
	I_StartThreadArgs \
 \
 	I_ExitThread \
	I_ExitDeleteThread \
	I_TerminateThread \
	I_iTerminateThread \
 \
	I_DisableDispatchThread \
	I_EnableDispatchThread \
 \
 	I_ChangeThreadPriority \
	I_iChangeThreadPriority \
 \
 	I_RotateThreadReadyQueue \
	I_iRotateThreadReadyQueue \
 \
 	I_ReleaseWaitThread \
	I_iReleaseWaitThread \
 \
 	I_GetThreadId \
	I_ReferThreadStatus \
	I_iReferThreadStatus \
 \
 	I_SleepThread \
	I_WakeupThread \
	I_iWakeupThread \
	I_CancelWakeupThread \
	I_iCancelWakeupThread \
 \
	I_SuspendThread \
	I_iSuspendThread \
	I_ResumeThread \
	I_iResumeThread \
 \
 	I_DelayThread \
 \
 	I_GetSystemTime \
	I_SetAlarm \
	I_iSetAlarm \
	I_CancelAlarm \
	I_iCancelAlarm \
	I_USec2SysClock \
	I_SysClock2USec \
 \
 	I_GetSystemStatusFlag \
 \
	thbase_IMPORTS_end
// clang-format on

#define thbase_IMPORTS_start DECLARE_IMPORT_TABLE(thbase, 1, 1)
#define thbase_IMPORTS_end   END_IMPORT_TABLE

#define I_CreateThread            DECLARE_IMPORT(4, CreateThread)
#define I_DeleteThread            DECLARE_IMPORT(5, DeleteThread)
#define I_StartThread             DECLARE_IMPORT(6, StartThread)
#define I_StartThreadArgs         DECLARE_IMPORT(7, StartThreadArgs)
#define I_ExitThread              DECLARE_IMPORT(8, ExitThread)
#define I_ExitDeleteThread        DECLARE_IMPORT(9, ExitDeleteThread)
#define I_TerminateThread         DECLARE_IMPORT(10, TerminateThread)
#define I_iTerminateThread        DECLARE_IMPORT(11, iTerminateThread)
#define I_DisableDispatchThread   DECLARE_IMPORT(12, DisableDispatchThread)
#define I_EnableDispatchThread    DECLARE_IMPORT(13, EnableDispatchThread)
#define I_ChangeThreadPriority    DECLARE_IMPORT(14, ChangeThreadPriority)
#define I_iChangeThreadPriority   DECLARE_IMPORT(15, iChangeThreadPriority)
#define I_RotateThreadReadyQueue  DECLARE_IMPORT(16, RotateThreadReadyQueue)
#define I_iRotateThreadReadyQueue DECLARE_IMPORT(17, iRotateThreadReadyQueue)
#define I_ReleaseWaitThread       DECLARE_IMPORT(18, ReleaseWaitThread)
#define I_iReleaseWaitThread      DECLARE_IMPORT(19, iReleaseWaitThread)
#define I_GetThreadId             DECLARE_IMPORT(20, GetThreadId)
#define I_CheckThreadStack        DECLARE_IMPORT(21, CheckThreadStack)
#define I_ReferThreadStatus       DECLARE_IMPORT(22, ReferThreadStatus)
#define I_iReferThreadStatus      DECLARE_IMPORT(23, iReferThreadStatus)
#define I_SleepThread             DECLARE_IMPORT(24, SleepThread)
#define I_WakeupThread            DECLARE_IMPORT(25, WakeupThread)
#define I_iWakeupThread           DECLARE_IMPORT(26, iWakeupThread)
#define I_CancelWakeupThread      DECLARE_IMPORT(27, CancelWakeupThread)
#define I_iCancelWakeupThread     DECLARE_IMPORT(28, iCancelWakeupThread)
#define I_SuspendThread           DECLARE_IMPORT(29, SuspendThread)
#define I_iSuspendThread          DECLARE_IMPORT(30, iSuspendThread)
#define I_ResumeThread            DECLARE_IMPORT(31, ResumeThread)
#define I_iResumeThread           DECLARE_IMPORT(32, iResumeThread)
#define I_DelayThread             DECLARE_IMPORT(33, DelayThread)
#define I_GetSystemTime           DECLARE_IMPORT(34, GetSystemTime)
#define I_SetAlarm                DECLARE_IMPORT(35, SetAlarm)
#define I_iSetAlarm               DECLARE_IMPORT(36, iSetAlarm)
#define I_CancelAlarm             DECLARE_IMPORT(37, CancelAlarm)
#define I_iCancelAlarm            DECLARE_IMPORT(38, iCancelAlarm)
#define I_USec2SysClock           DECLARE_IMPORT(39, USec2SysClock)
#define I_SysClock2USec           DECLARE_IMPORT(40, SysClock2USec)
#define I_GetSystemStatusFlag     DECLARE_IMPORT(41, GetSystemStatusFlag)

#endif /* __THBASE_H__ */
