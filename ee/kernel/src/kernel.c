/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE Kernel functions
 */

#include <syscallnr.h>

#define STRINNER(x) #x
#define STR(x) STRINNER(x)

#define SYSCALL(name)   \
		SYSCALL_SPECIAL(name, name)

#ifdef USE_KMODE
#define SYSCALL_SPECIAL(symbol, name)       \
	__asm__ ( \
		"\t" "\t" ".set    push;" "\n" \
		"\t" "\t" ".set    noreorder;" "\n" \
		"\t" "\t" ".globl  " STR(symbol) ";" "\n" \
		"\t" "\t" ".type   " STR(symbol) ",@function;" "\n" \
		"\t" "\t" ".ent    " STR(symbol) ",0;" "\n" \
		"\t" "" STR(symbol) ": j   __syscall;" "\n" \
		"\t" "\t" "li  $3, " STR(__NR_##name) ";" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" ".end    " STR(symbol) ";" "\n" \
		"\t" "\t" ".size   " STR(symbol) ",.-" STR(symbol) ";" "\n" \
		"\t" "\t" ".set    pop;" "\n" \
	);
#else
#define SYSCALL_SPECIAL(symbol, name)       \
	__asm__ ( \
		"\t" "\t" ".set    push;" "\n" \
		"\t" "\t" ".set    noreorder;" "\n" \
		"\t" "\t" ".globl  " STR(symbol) ";" "\n" \
		"\t" "\t" ".type   " STR(symbol) ",@function;" "\n" \
		"\t" "\t" ".ent    " STR(symbol) ",0;" "\n" \
		"\t" "" STR(symbol) ": li  $3, " STR(__NR_##name) ";" "\n" \
		"\t" "\t" "syscall;" "\n" \
		"\t" "\t" "jr  $31;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" ".end    " STR(symbol) ";" "\n" \
		"\t" "\t" ".size   " STR(symbol) ",.-" STR(symbol) ";" "\n" \
		"\t" "\t" ".set    pop;" "\n" \
	);
#endif

#ifdef F___syscall
__asm__
(
	"\t" ".set    push" "\n"
	"\t" ".set    noreorder" "\n"
	"\t" ".globl  __syscall" "\n"
	"\t" ".type   __syscall,@function" "\n"
	"\t" ".ent    __syscall,0" "\n"
	"\t" "__syscall:" "\n"
	"\t" "\t" "mfc0        $2, $12" "\n"
	"\t" "\t" "andi        $2, $2, 0x18" "\n"
	"\t" "\t" "beqz        $2, _kMode" "\n"
	"\t" "\t" "slt         $2, $3, $0" "\n"
	"\t" "\t" "syscall" "\n"
	"\t" "\t" "jr          $31" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "_kMode:" "\n"
	"\t" "\t" "subu        $26, $0, $3" "\n"
	"\t" "\t" "movn        $3, $26, $2" "\n"
	"\t" "\t" "sll         $3, $3, 2" "\n"
	"\t" "\t" "lui         $26, 0x8000" "\n"
	"\t" "\t" "lhu         $2, 0x02F0($26)" "\n"
	"\t" "\t" "sll         $2, $2, 16" "\n"
	"\t" "\t" "lh          $26, 0x02F8($26)" "\n"
	"\t" "\t" "add         $2, $26" "\n"
	"\t" "\t" "addu        $3, $2" "\n"
	"\t" "\t" "lw          $26, 0x00($3)" "\n"
	"\t" "\t" "jr          $26" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end    __syscall" "\n"
	"\t" ".size   __syscall,.-__syscall" "\n"
	"\t" ".set    pop" "\n"
);
#endif

#ifdef F_ResetEE
SYSCALL(ResetEE)
#endif

#ifdef F_SetGsCrt
SYSCALL(SetGsCrt)
#endif

#ifdef F_KExit
SYSCALL(KExit)
#endif

#ifdef F__LoadExecPS2
SYSCALL(_LoadExecPS2)
#endif

#ifdef F__ExecPS2
SYSCALL(_ExecPS2)
#endif

#ifdef F_RFU009
SYSCALL(RFU009)
#endif

#ifdef F_AddSbusIntcHandler
SYSCALL(AddSbusIntcHandler)
#endif

#ifdef F_RemoveSbusIntcHandler
SYSCALL(RemoveSbusIntcHandler)
#endif

#ifdef F_Interrupt2Iop
SYSCALL(Interrupt2Iop)
#endif

#ifdef F_SetVTLBRefillHandler
SYSCALL(SetVTLBRefillHandler)
#endif

#ifdef F_SetVCommonHandler
SYSCALL(SetVCommonHandler)
#endif

#ifdef F_SetVInterruptHandler
SYSCALL(SetVInterruptHandler)
#endif

#ifdef F_AddIntcHandler
SYSCALL(AddIntcHandler)
#endif

#ifdef F_AddIntcHandler2
SYSCALL(AddIntcHandler2)
#endif

#ifdef F_RemoveIntcHandler
SYSCALL(RemoveIntcHandler)
#endif

#ifdef F_AddDmacHandler
SYSCALL(AddDmacHandler)
#endif

#ifdef F_AddDmacHandler2
SYSCALL(AddDmacHandler2)
#endif

#ifdef F_RemoveDmacHandler
SYSCALL(RemoveDmacHandler)
#endif

#ifdef F__EnableIntc
SYSCALL(_EnableIntc)
#endif

#ifdef F__DisableIntc
SYSCALL(_DisableIntc)
#endif

#ifdef F__EnableDmac
SYSCALL(_EnableDmac)
#endif

#ifdef F__DisableDmac
SYSCALL(_DisableDmac)
#endif

#ifdef F__SetAlarm
SYSCALL(_SetAlarm)
#endif

#ifdef F_SetAlarm
SYSCALL(SetAlarm)
#endif

#ifdef F__ReleaseAlarm
SYSCALL(_ReleaseAlarm)
#endif

#ifdef F_ReleaseAlarm
SYSCALL(ReleaseAlarm)
#endif

#ifdef F__iEnableIntc
SYSCALL(_iEnableIntc)
#endif

#ifdef F__iDisableIntc
SYSCALL(_iDisableIntc)
#endif

#ifdef F__iEnableDmac
SYSCALL(_iEnableDmac)
#endif

#ifdef F__iDisableDmac
SYSCALL(_iDisableDmac)
#endif

#ifdef F__iSetAlarm
SYSCALL(_iSetAlarm)
#endif

#ifdef F_iSetAlarm
SYSCALL(iSetAlarm)
#endif

#ifdef F__iReleaseAlarm
SYSCALL(_iReleaseAlarm)
#endif

#ifdef F_iReleaseAlarm
SYSCALL(iReleaseAlarm)
#endif

#ifdef F_CreateThread
SYSCALL(CreateThread)
#endif

#ifdef F_DeleteThread
SYSCALL(DeleteThread)
#endif

#ifdef F_StartThread
SYSCALL(StartThread)
#endif

#ifdef F_ExitThread
SYSCALL(ExitThread)
#endif

#ifdef F_ExitDeleteThread
SYSCALL(ExitDeleteThread)
#endif

#ifdef F_TerminateThread
SYSCALL(TerminateThread)
#endif

#ifdef F_iTerminateThread
SYSCALL(iTerminateThread)
#endif

#ifdef F_DisableDispatchThread
SYSCALL(DisableDispatchThread)
#endif

#ifdef F_EnableDispatchThread
SYSCALL(EnableDispatchThread)
#endif

#ifdef F_ChangeThreadPriority
SYSCALL(ChangeThreadPriority)
#endif

#ifdef F_iChangeThreadPriority
SYSCALL(iChangeThreadPriority)
#endif

#ifdef F_RotateThreadReadyQueue
SYSCALL(RotateThreadReadyQueue)
#endif

#ifdef F__iRotateThreadReadyQueue
SYSCALL(_iRotateThreadReadyQueue)
#endif

#ifdef F_ReleaseWaitThread
SYSCALL(ReleaseWaitThread)
#endif

#ifdef F_iReleaseWaitThread
SYSCALL(iReleaseWaitThread)
#endif

#ifdef F_GetThreadId
SYSCALL(GetThreadId)
#endif

#ifdef F__iGetThreadId
SYSCALL(_iGetThreadId)
#endif

#ifdef F_ReferThreadStatus
SYSCALL(ReferThreadStatus)
#endif

#ifdef F_iReferThreadStatus
SYSCALL(iReferThreadStatus)
#endif

#ifdef F_SleepThread
SYSCALL(SleepThread)
#endif

#ifdef F_WakeupThread
SYSCALL(WakeupThread)
#endif

#ifdef F__iWakeupThread
SYSCALL(_iWakeupThread)
#endif

#ifdef F_CancelWakeupThread
SYSCALL(CancelWakeupThread)
#endif

#ifdef F_iCancelWakeupThread
SYSCALL(iCancelWakeupThread)
#endif

#ifdef F_SuspendThread
SYSCALL(SuspendThread)
#endif

#ifdef F__iSuspendThread
SYSCALL(_iSuspendThread)
#endif

#ifdef F_ResumeThread
SYSCALL(ResumeThread)
#endif

#ifdef F_iResumeThread
SYSCALL(iResumeThread)
#endif

#ifdef F_RFU059
SYSCALL(RFU059)
#endif

#ifdef F_RFU060
SYSCALL(RFU060)
#endif

#ifdef F_SetupThread
SYSCALL(SetupThread)
#endif

#ifdef F_RFU061
SYSCALL(RFU061)
#endif

#ifdef F_SetupHeap
SYSCALL(SetupHeap)
#endif

#ifdef F_EndOfHeap
SYSCALL(EndOfHeap)
#endif

#ifdef F_CreateSema
SYSCALL(CreateSema)
#endif

#ifdef F_DeleteSema
SYSCALL(DeleteSema)
#endif

#ifdef F_SignalSema
SYSCALL(SignalSema)
#endif

#ifdef F_iSignalSema
SYSCALL(iSignalSema)
#endif

#ifdef F_WaitSema
SYSCALL(WaitSema)
#endif

#ifdef F_PollSema
SYSCALL(PollSema)
#endif

#ifdef F_iPollSema
SYSCALL(iPollSema)
#endif

#ifdef F_ReferSemaStatus
SYSCALL(ReferSemaStatus)
#endif

#ifdef F_iReferSemaStatus
SYSCALL(iReferSemaStatus)
#endif

#ifdef F_iDeleteSema
SYSCALL(iDeleteSema)
#endif

#ifdef F_SetOsdConfigParam
SYSCALL(SetOsdConfigParam)
#endif

#ifdef F_GetOsdConfigParam
SYSCALL(GetOsdConfigParam)
#endif

#ifdef F_GetGsHParam
SYSCALL(GetGsHParam)
#endif

#ifdef F_GetGsVParam
SYSCALL(GetGsVParam)
#endif

#ifdef F_SetGsHParam
SYSCALL(SetGsHParam)
#endif

#ifdef F_SetGsVParam
SYSCALL(SetGsVParam)
#endif

#ifdef F_CreateEventFlag
SYSCALL(CreateEventFlag)
#endif

#ifdef F_DeleteEventFlag
SYSCALL(DeleteEventFlag)
#endif

#ifdef F_SetEventFlag
SYSCALL(SetEventFlag)
#endif

#ifdef F_iSetEventFlag
SYSCALL(iSetEventFlag)
#endif

#ifdef F_PutTLBEntry
SYSCALL(PutTLBEntry)
#endif

#ifdef F_iPutTLBEntry
SYSCALL(iPutTLBEntry)
#endif

#ifdef F__SetTLBEntry
SYSCALL(_SetTLBEntry)
#endif

#ifdef F_iSetTLBEntry
SYSCALL(iSetTLBEntry)
#endif

#ifdef F_GetTLBEntry
SYSCALL(GetTLBEntry)
#endif

#ifdef F_iGetTLBEntry
SYSCALL(iGetTLBEntry)
#endif

#ifdef F_ProbeTLBEntry
SYSCALL(ProbeTLBEntry)
#endif

#ifdef F_iProbeTLBEntry
SYSCALL(iProbeTLBEntry)
#endif

#ifdef F_ExpandScratchPad
SYSCALL(ExpandScratchPad)
#endif

#ifdef F_EnableIntcHandler
SYSCALL(EnableIntcHandler)
#endif

#ifdef F_iEnableIntcHandler
SYSCALL(iEnableIntcHandler)
#endif

#ifdef F_DisableIntcHandler
SYSCALL(DisableIntcHandler)
#endif

#ifdef F_iDisableIntcHandler
SYSCALL(iDisableIntcHandler)
#endif

#ifdef F_EnableDmacHandler
SYSCALL(EnableDmacHandler)
#endif

#ifdef F_iEnableDmacHandler
SYSCALL(iEnableDmacHandler)
#endif

#ifdef F_DisableDmacHandler
SYSCALL(DisableDmacHandler)
#endif

#ifdef F_iDisableDmacHandler
SYSCALL(iDisableDmacHandler)
#endif

#ifdef F_KSeg0
SYSCALL(KSeg0)
#endif

#ifdef F_EnableCache
SYSCALL(EnableCache)
#endif

#ifdef F_DisableCache
SYSCALL(DisableCache)
#endif

#ifdef F_GetCop0
SYSCALL(GetCop0)
#endif

#ifdef F_FlushCache
SYSCALL(FlushCache)
#endif

#ifdef F_CpuConfig
SYSCALL(CpuConfig)
#endif

#ifdef F_iGetCop0
SYSCALL(iGetCop0)
#endif

#ifdef F_iFlushCache
SYSCALL(iFlushCache)
#endif

#ifdef F_RFU105
SYSCALL(RFU105)
#endif

#ifdef F_iCpuConfig
SYSCALL(iCpuConfig)
#endif

#ifdef F_sceSifStopDma
SYSCALL(sceSifStopDma)
#endif

#ifdef F_SetCPUTimerHandler
SYSCALL(SetCPUTimerHandler)
#endif

#ifdef F_SetCPUTimer
SYSCALL(SetCPUTimer)
#endif

#ifdef F_SetOsdConfigParam2
SYSCALL(SetOsdConfigParam2)
#endif

#ifdef F_GetOsdConfigParam2
SYSCALL(GetOsdConfigParam2)
#endif

#ifdef F_GsGetIMR
SYSCALL(GsGetIMR)
#endif

#ifdef F_iGsGetIMR
SYSCALL(iGsGetIMR)
#endif

#ifdef F_GsPutIMR
SYSCALL(GsPutIMR)
#endif

#ifdef F_iGsPutIMR
SYSCALL(iGsPutIMR)
#endif

#ifdef F_SetPgifHandler
SYSCALL(SetPgifHandler)
#endif

#ifdef F_SetVSyncFlag
SYSCALL(SetVSyncFlag)
#endif

#ifdef F_SetSyscall
SYSCALL(SetSyscall)
#endif

#ifdef F__print
SYSCALL(_print)
#endif

#ifdef F_sceSifDmaStat
SYSCALL(sceSifDmaStat)
#endif

#ifdef F_isceSifDmaStat
SYSCALL(isceSifDmaStat)
#endif

#ifdef F_sceSifSetDma
SYSCALL(sceSifSetDma)
#endif

#ifdef F_isceSifSetDma
SYSCALL(isceSifSetDma)
#endif

#ifdef F_sceSifSetDChain
SYSCALL(sceSifSetDChain)
#endif

#ifdef F_isceSifSetDChain
SYSCALL(isceSifSetDChain)
#endif

#ifdef F_sceSifSetReg
SYSCALL(sceSifSetReg)
#endif

#ifdef F_sceSifGetReg
SYSCALL(sceSifGetReg)
#endif

#ifdef F__ExecOSD
SYSCALL(_ExecOSD)
#endif

#ifdef F_Deci2Call
SYSCALL(Deci2Call)
#endif

#ifdef F_PSMode
SYSCALL(PSMode)
#endif

#ifdef F_MachineType
SYSCALL(MachineType)
#endif

#ifdef F_GetMemorySize
SYSCALL(GetMemorySize)
#endif

#ifdef F__GetGsDxDyOffset
SYSCALL(_GetGsDxDyOffset)
#endif

#ifdef F__InitTLB
SYSCALL(_InitTLB)
#endif

#ifdef F_SetMemoryMode
SYSCALL(SetMemoryMode)
#endif

#ifdef F_GetMemoryMode
SYSCALL(GetMemoryMode)
#endif

#ifdef F_GPfuncs
__asm__
(
	"\t" ".globl ChangeGP" "\n"
	"\t" ".ent ChangeGP" "\n"
	"\t" "ChangeGP:" "\n"
	"\t" "\t" "move    $v0, $gp" "\n"
	"\t" "\t" "jr  $ra" "\n"
	"\t" "\t" "move    $gp, $a0" "\n"
	"\t" ".end ChangeGP" "\n"

	"\t" ".globl SetGP" "\n"
	"\t" ".ent SetGP" "\n"
	"\t" "SetGP:" "\n"
	"\t" "\t" "jr  $ra" "\n"
	"\t" "\t" "move    $gp, $a0" "\n"
	"\t" ".end SetGP" "\n"

	"\t" ".globl GetGP" "\n"
	"\t" ".ent GetGP" "\n"
	"\t" "GetGP:" "\n"
	"\t" "\t" "jr  $ra" "\n"
	"\t" "\t" "move    $v0, $gp" "\n"
	"\t" ".end GetGP" "\n"
);
#endif

#ifdef F_sceSifWriteBackDCache
__asm__
(
	"\t" "\t" ".globl  sceSifWriteBackDCache" "\n"
	"\t" "\t" ".ent    sceSifWriteBackDCache" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"

	"\t" "sceSifWriteBackDCache:" "\n" /* DHWBIN: Data cache Hit WriteBack INvalidate.  */

	"\t" "\t" "lui $25, 0xffff" "\n"
	"\t" "\t" "ori $25, $25, 0xffc0" "\n"
	"\t" "\t" "blez    $5, last" "\n"
	"\t" "\t" "addu    $10, $4, $5" "\n"
	"\t" "\t" "and $8, $4, $25" "\n"
	"\t" "\t" "addiu   $10, $10, -1" "\n"
	"\t" "\t" "and $9, $10, $25" "\n"
	"\t" "\t" "subu    $10, $9, $8" "\n"
	"\t" "\t" "srl $11, $10, 0x6" "\n"
	"\t" "\t" "addiu   $11, $11, 1" "\n"
	"\t" "\t" "andi    $9, $11, 0x7" "\n"
	"\t" "\t" "beqz    $9, eight" "\n"
	"\t" "\t" "srl $10, $11, 0x3" "\n"
	"\t" "loop1:" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 0($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "addiu   $9, $9, -1" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "bgtz    $9, loop1" "\n"
	"\t" "\t" "addiu   $8, $8, 64" "\n"

	"\t" "eight:" "\n"
	"\t" "\t" "beqz    $10, last" "\n"
	"\t" "loop8:" "\n"
	"\t" "\t" "addiu   $10, $10, -1" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 0($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 64($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 128($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 192($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 256($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 320($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 384($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "cache   0x18, 448($8)" "\n"
	"\t" "\t" "sync" "\n"
	"\t" "\t" "bgtz    $10, loop8" "\n"
	"\t" "\t" "addiu   $8, $8, 512" "\n"
	"\t" "last:" "\n"
	"\t" "\t" "jr  $31" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".set    pop" "\n"
	"\t" "\t" ".end    sceSifWriteBackDCache" "\n"
);
#endif

#define DXWBIN  0x14        /* Data cache: indeX WriteBack INvalidate.  */
#define DXIN    0x16        /* Data cache: indeX INvalidate.  */

#define opDCache(name, op)  \
	__asm__ ( \
		"\t" "\t" ".set    push;" "\n" \
		"\t" "\t" ".set    noreorder;" "\n" \
		"\t" "\t" ".set    nomacro;" "\n" \
		"\t" "\t" ".globl  " STR(name) ";" "\n" \
		"\t" "\t" ".type   " STR(name) ",@function;" "\n" \
		"\t" "\t" ".ent    " STR(name) ", 0;" "\n" \
		"\t" "" STR(name) ":   lui $7, 0xffff;" "\n" \
		"\t" "\t" "daddu   $6, $0, $0;" "\n" \
		"\t" "\t" "ori $7, 0xf000;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "1:  sync;" "\n" \
		"\t" "\t" "cache   0x10, 0($6);" "\n" \
		"\t" "\t" "sync;" "\n" \
		"\t" "\t" "mfc0    $2, $28;" "\n" \
		"\t" "\t" "and $2, $7;" "\n" \
		"\t" "\t" "addu    $2, $6;" "\n" \
		"\t" "\t" "sltu    $3, $5, $2;" "\n" \
		"\t" "\t" "sltu    $2, $4;" "\n" \
		"\t" "\t" "bnez    $2, 2f;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" "bnez    $3, 2f;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" "sync;" "\n" \
		"\t" "\t" "cache   " STR(op) ", 0($6);" "\n" \
		"\t" "\t" "sync;" "\n" \
		"\t" "2:  sync;" "\n" \
		"\t" "\t" "cache   0x10, 1($6);" "\n" \
		"\t" "\t" "sync;" "\n" \
		"\t" "\t" "mfc0    $2, $28;" "\n" \
		"\t" "\t" "and $2, $7;" "\n" \
		"\t" "\t" "addu    $2, $6;" "\n" \
		"\t" "\t" "sltu    $3, $5, $2;" "\n" \
		"\t" "\t" "sltu    $2, $4;" "\n" \
		"\t" "\t" "bnez    $2, 3f;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" "bnez    $3, 3f;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" "sync;" "\n" \
		"\t" "\t" "cache   " STR(op) ", 1($6);" "\n" \
		"\t" "\t" "sync;" "\n" \
		"\t" "3:  sync;" "\n" \
		"\t" "\t" "addiu   $6, 64;" "\n" \
		"\t" "\t" "slti    $2, $6, 4096;" "\n" \
		"\t" "\t" "bnez    $2, 1b;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" "jr  $31;" "\n" \
		"\t" "\t" "nop;" "\n" \
		"\t" "\t" ".end    " STR(name) ";" "\n" \
		"\t" "\t" ".size   " STR(name) ",.-" STR(name) ";" "\n" \
		"\t" "\t" ".set    pop;" "\n" \
	);

#ifdef F__SyncDCache
opDCache(_SyncDCache, DXWBIN)
#endif

#ifdef F__InvalidDCache
opDCache(_InvalidDCache, DXIN)
#endif

#ifdef F___errno
/* This is needed in case we are linked against libm (the math library) but
   not libc.  */
__asm__
(
	"\t" "\t" ".globl  __errno" "\n"
	"\t" "\t" ".ent    __errno" "\n"
	"\t" "\t" ".weak   __errno" "\n"

	"\t" "__errno:" "\n"
	"\t" "\t" "la  $2, errno" "\n"
	"\t" "\t" "jr  $31" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" ".end    __errno" "\n"
);
#endif

#ifdef F_errno
/* New applications compiled against ps2lib that use errno will resolve to
   this, while existing newlib applications will resolve to __errno.  */
__asm__
(
	"\t" "\t" ".globl  errno" "\n"
	"\t" "\t" ".weak   errno" "\n"

	"\t" "\t" ".data" "\n"
	"\t" "errno:" "\n"
	"\t" "\t" ".space  4" "\n"
);
#endif

#ifdef F_strlen
/* Assembler version of strlen that uses quadword instructions.

   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.

   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */

__asm__
(
	"\t" "\t" ".globl  strlen" "\n"
	"\t" "\t" ".ent    strlen" "\n"
	"\t" "\t" ".weak   strlen" "\n"
	"\t" "strlen:" "\n"
	"\t" "\t" ".frame  $sp,0,$31" "\n" // vars= 0, regs= 0/0, args= 0, extra= 0
	"\t" "\t" ".mask   0x00000000,0" "\n"
	"\t" "\t" ".fmask  0x00000000,0" "\n"

#ifndef __OPTIMIZE_SIZE__

	/* check if src is quadword aligned, doubleword aligned, or neither in which case
	   perform checking for null terminator one byte at a time. */

	"\t" "\t" "andi    $2,$4,0x7" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L15" "\n"
	"\t" "\t" "move    $7,$4" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "andi    $3,$4,0xf" "\n"
	"\t" "\t" "dli $2,0x0101010101010101" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $3,$0,$L12" "\n"
	"\t" "\t" "move    $5,$4" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* src is quadword aligned.  Load a quadword at a time and check for null terminator.
	   If null terminator is found, go and find exact position by looking at each byte
	   of the last quadword.  Otherwise, continue to next quadword and keep searching. */

	"\t" "\t" "lq  $3,0($5)" "\n"
	"\t" "\t" "pcpyld  $8,$2,$2" "\n"
	"\t" "\t" "dli $4,0x8080808080808080" "\n"
	"\t" "\t" "psubb   $2,$3,$8" "\n"
	"\t" "\t" "pnor    $3,$0,$3" "\n"
	"\t" "\t" "pcpyld  $9,$4,$4" "\n"
	"\t" "\t" "pand    $2,$2,$3" "\n"
	"\t" "\t" "pand    $2,$2,$9" "\n"
	"\t" "\t" "pcpyud  $3,$2,$8" "\n"
	"\t" "\t" "or  $6,$3,$2" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bnel    $6,$0,$L15" "\n"
	"\t" "\t" "move    $4,$5" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "addu    $5,$5,16" "\n"
	"\t" "$L14:" "\n"
	"\t" "\t" "lq  $2,0($5)" "\n"
	"\t" "\t" "#nop" "\n"
	"\t" "\t" "pnor    $3,$0,$2" "\n"
	"\t" "\t" "psubb   $2,$2,$8" "\n"
	"\t" "\t" "pand    $2,$2,$3" "\n"
	"\t" "\t" "pand    $4,$2,$9" "\n"
	"\t" "\t" "pcpyud  $3,$4,$6" "\n"
	"\t" "\t" "or  $3,$3,$4" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beql    $3,$0,$L14" "\n"
	"\t" "\t" "addu    $5,$5,16" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "b   $L15" "\n"
	"\t" "\t" "move    $4,$5" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* src is doubleword aligned.  Load a doubleword at a time and check for null terminator.
	   If null terminator is found, go and find exact position by looking at each byte
	   of the last doubleword.  Otherwise, continue to next doubleword and keep searching. */


	"\t" "$L12:" "\n"
	"\t" "\t" "ld  $3,0($5)" "\n"
	"\t" "\t" "dli $4,0x8080808080808080" "\n"
	"\t" "\t" "dsubu   $2,$3,$2" "\n"
	"\t" "\t" "nor $3,$0,$3" "\n"
	"\t" "\t" "and $2,$2,$3" "\n"
	"\t" "\t" "and $2,$2,$4" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bnel    $2,$0,$L15" "\n"
	"\t" "\t" "move    $4,$5" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "dli $6,0x0101010101010101" "\n"
	"\t" "\t" "addu    $5,$5,8" "\n"
	"\t" "$L16:" "\n"
	"\t" "\t" "ld  $2,0($5)" "\n"
	"\t" "\t" "#nop" "\n"
	"\t" "\t" "nor $3,$0,$2" "\n"
	"\t" "\t" "dsubu   $2,$2,$6" "\n"
	"\t" "\t" "and $2,$2,$3" "\n"
	"\t" "\t" "and $2,$2,$4" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beql    $2,$0,$L16" "\n"
	"\t" "\t" "addu    $5,$5,8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "move    $4,$5" "\n"

#else /* __OPTIMIZE_SIZE__ */

	"\t" "\t" "move    $7,$4" "\n"

#endif /* __OPTIMIZE_SIZE__ */

	/* search a byte at a time for null terminator and then calculate length by subtracting
	   original string address from null terminator address. */

	"\t" "$L9:" "\n"
	"\t" "$L15:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"

	"\t" "\t" "lb  $2,0($4)" "\n"
	"\t" "\t" "#nop" "\n"
	"\t" "\t" "beq $2,$0,1f" "\n"
	"\t" "\t" "addu    $4,$4,1" "\n"

	"\t" "\t" "lb  $2,0($4)" "\n"
	"\t" "\t" "#nop" "\n"
	"\t" "\t" "bne $2,$0,$L9" "\n"
	"\t" "\t" "addu    $4,$4,1" "\n"
	"\t" "1:" "\n"
	"\t" "\t" "subu    $4,$4,1" "\n"

	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "j   $31" "\n"
	"\t" "\t" "subu    $2,$4,$7" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".end    strlen" "\n"
);
#endif

#ifdef F_strncpy
/* Assembler version of strncpy using quadword instructions

   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.

   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */

__asm__
(
	"\t" "\t" ".globl  strncpy" "\n"
	"\t" "\t" ".ent    strncpy" "\n"
	"\t" "\t" ".weak   strncpy" "\n"
	"\t" "strncpy:" "\n"
	"\t" "\t" ".frame  $sp,0,$31" "\n" // vars= 0, regs= 0/0, args= 0, extra= 0
	"\t" "\t" ".mask   0x00000000,0" "\n"
	"\t" "\t" ".fmask  0x00000000,0" "\n"

	"\t" "\t" "move    $8,$4" "\n"

#ifndef __OPTIMIZE_SIZE__

	/* check if src and dest are doubleword aligned, quadword aligned, or neither in which
	   case copy byte by byte */

	"\t" "\t" "or  $7,$5,$4" "\n"
	"\t" "\t" "li  $10,0x10" "\n"
	"\t" "\t" "andi    $2,$7,0x7" "\n"
	"\t" "\t" "li  $9,0x8" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L9" "\n"
	"\t" "\t" "andi    $2,$7,0xf" "\n"
	"\t" "\t" ".set    pop" "\n"
	"\t" "\t" "movz    $9,$10,$2" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L17" "\n"
	"\t" "\t" "sltu    $2,$6,$9" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* src and dest are quadword aligned.  Check a quadword at a time looking for a
	   null terminator or until the nth byte is reached.  Otherwise, copy the
	   quadword to dest and continue looping checking quadwords.   Once a null
	   terminator is found or n is < 16 go copy a byte at a time. */

	"\t" "$L31:" "\n"
	"\t" "\t" "bne $2,$0,$L9" "\n"
	"\t" "\t" "dli $7,0x0101010101010101" "\n"
	"\t" "\t" "lq  $3,0($5)" "\n"
	"\t" "\t" "pcpyld  $9,$7,$7" "\n"
	"\t" "\t" "pnor    $3,$0,$3" "\n"
	"\t" "\t" "dli $7,0x8080808080808080" "\n"
	"\t" "\t" "psubb   $2,$3,$9" "\n"
	"\t" "\t" "pcpyld  $10,$7,$7" "\n"
	"\t" "\t" "pand    $2,$2,$3" "\n"
	"\t" "\t" "pand    $2,$2,$10" "\n"
	"\t" "\t" "pcpyud  $3,$2,$4" "\n"
	"\t" "\t" "or  $3,$2,$3" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $3,$0,$L5" "\n"
	"\t" "\t" "move    $7,$8" "\n"
	"\t" "\t" ".set    pop" "\n"
	"\t" "\t" "lq  $3,0($5)" "\n"
	"\t" "\t" ".p2align 3" "\n"
	"\t" "$L39:" "\n"
	"\t" "\t" "addu    $6,$6,-16" "\n"
	"\t" "\t" "addu    $5,$5,16" "\n"
	"\t" "\t" "sltu    $2,$6,16" "\n"
	"\t" "\t" "sq  $3,0($7)" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L5" "\n"
	"\t" "\t" "addu    $7,$7,16" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "lq  $2,0($5)" "\n"
	"\t" "\t" "#nop" "\n"
	"\t" "\t" "pnor    $3,$0,$2" "\n"
	"\t" "\t" "psubb   $2,$2,$9" "\n"
	"\t" "\t" "pand    $2,$2,$3" "\n"
	"\t" "\t" "pand    $2,$2,$10" "\n"
	"\t" "\t" "pcpyud  $3,$2,$4" "\n"
	"\t" "\t" "or  $2,$2,$3" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beql    $2,$0,$L19" "\n"
	"\t" "\t" "lq  $3,0($5)" "\n"
	"\t" "\t" "b   $L9" "\n"
	"\t" "\t" "move    $4,$7" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* src and dest are quadword aligned.  Check a quadword at a time looking for a
	   null terminator or until the nth byte is reached.  Otherwise, copy the
	   quadword to dest and continue looping checking quadwords.   Once a null
	   terminator is found or n is < 16 go copy a byte at a time. */

	"\t" "$L17:" "\n"
	"\t" "\t" "bne $2,$0,$L9" "\n"
	"\t" "\t" "ld  $3,0($5)" "\n"
	"\t" "\t" "dli $9,0x0101010101010101" "\n"
	"\t" "\t" "dli $10,0x8080808080808080" "\n"
	"\t" "\t" "dsubu   $2,$3,$9" "\n"
	"\t" "\t" "nor $3,$0,$3" "\n"
	"\t" "\t" "and $2,$2,$3" "\n"
	"\t" "\t" "and $2,$2,$10" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L5" "\n"
	"\t" "\t" "move    $7,$8" "\n"
	"\t" "\t" ".set    pop" "\n"
	"\t" "\t" "ld  $3,0($5)" "\n"
	"\t" "\t" ".p2align 3" "\n"
	"\t" "$L19:" "\n"
	"\t" "\t" "addu    $6,$6,-8" "\n"
	"\t" "\t" "addu    $5,$5,8" "\n"
	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "\t" "sd  $3,0($7)" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L5" "\n"
	"\t" "\t" "addu    $7,$7,8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "ld  $2,0($5)" "\n"
	"\t" "\t" "#nop" "\n"
	"\t" "\t" "nor $3,$0,$2" "\n"
	"\t" "\t" "dsubu   $2,$2,$9" "\n"
	"\t" "\t" "and $2,$2,$3" "\n"
	"\t" "\t" "and $2,$2,$10" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beql    $2,$0,$L19" "\n"
	"\t" "\t" "ld  $3,0($5)" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "$L5:" "\n"
	"\t" "\t" "move    $4,$7" "\n"

#endif /* !__OPTIMIZE_SIZE__ */

	/* check a byte at a time looking for either the null terminator or until n bytes are
	   copied.  If the null terminator is found and n is not reached yet, copy null
	   bytes until n is reached. */

	"\t" "\t" ".p2align 3" "\n"
	"\t" "$L9:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beq $6,$0,$L18" "\n"
	"\t" "\t" "move    $2,$6" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "lbu $2,0($5)" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" "addu    $5,$5,1" "\n"
	"\t" "\t" "sb  $2,0($4)" "\n"
	"\t" "\t" "sll $2,$2,24" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L9" "\n"
	"\t" "\t" "addu    $4,$4,1" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "move    $2,$6" "\n"
	"\t" "$L20:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beq $2,$0,$L18" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".p2align 3" "\n"
	"\t" "$L16:" "\n"
	"\t" "\t" "sb  $0,0($4)" "\n"
	"\t" "\t" "move    $2,$6" "\n"
	"\t" "\t" "addu    $4,$4,1" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "bne $2,$0,$L16" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "$L18:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "j   $31" "\n"
	"\t" "\t" "move    $2,$8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".end    strncpy" "\n"
);
#endif

#ifdef F_memcpy
/* Assembler version of memcpy using quadword instructions.

   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.

   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */

__asm__
(
	"\t" "\t" ".globl  memcpy" "\n"
	"\t" "\t" ".ent    memcpy" "\n"
	"\t" "\t" ".weak   memcpy" "\n"
	"\t" "memcpy:" "\n"
	"\t" "\t" ".frame  $sp,0,$31" "\n" // vars= 0, regs= 0/0, args= 0, extra= 0
	"\t" "\t" ".mask   0x00000000,0" "\n"
	"\t" "\t" ".fmask  0x00000000,0" "\n"
	"\t" "\t" "move    $8,$4" "\n"

#ifndef __OPTIMIZE_SIZE__

	/* if bytes to move are < 32 or src or dest are not quadword aligned, jump to
	   code that moves one byte at a time */

	"\t" "\t" "sltu    $2,$6,32" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L2" "\n"
	"\t" "\t" "move    $3,$8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "or  $2,$5,$8" "\n"
	"\t" "\t" "andi    $2,$2,0xF" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bnel    $2,$0,$L20" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "move    $7,$8" "\n"

	/* while remainder to move is >= 32 bytes, use LQ/SQ quadword instructions
	   to move data */

	"\t" "$L5:" "\n"
	"\t" "\t" "lq  $3,0($5)" "\n"
	"\t" "\t" "addu    $6,$6,-32" "\n"
	"\t" "\t" "addu    $5,$5,16" "\n"
	"\t" "\t" "sltu    $4,$6,32" "\n"
	"\t" "\t" "sq  $3,0($7)" "\n"
	"\t" "\t" "addu    $7,$7,16" "\n"
	"\t" "\t" "lq  $2,0($5)" "\n"
	"\t" "\t" "addu    $5,$5,16" "\n"
	"\t" "\t" "sq  $2,0($7)" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beq $4,$0,$L5" "\n"
	"\t" "\t" "addu    $7,$7,16" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L2" "\n"
	"\t" "\t" "move    $3,$7" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* while remainder to move is >= 8 bytes, use LD/SD doubleword instructions
	   to move data */

	"\t" "$L9:" "\n"
	"\t" "\t" "ld  $3,0($5)" "\n"
	"\t" "\t" "addu    $6,$6,-8" "\n"
	"\t" "\t" "addu    $5,$5,8" "\n"
	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "\t" "sd  $3,0($7)" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beq $2,$0,$L9" "\n"
	"\t" "\t" "addu    $7,$7,8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "move    $3,$7" "\n"

#else  /* __OPTIMIZE_SIZE__ */

	"\t" "\t" "move    $3,$8" "\n"

#endif /* !__OPTIMIZE_SIZE__ */

	/* Move any remaining bytes one at a time */

	"\t" "$L2:" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "$L20:" "\n"
	"\t" "\t" "li  $2,-1" "\n" // 0xffffffffffffffff
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beq $6,$2,$L12" "\n"
	"\t" "\t" "move    $4,$2" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "$L13:" "\n"
	"\t" "\t" "lbu $2,0($5)" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" "addu    $5,$5,1" "\n"
	"\t" "\t" "sb  $2,0($3)" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "bne $6,$4,$L13" "\n"
	"\t" "\t" "addu    $3,$3,1" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "$L12:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "j   $31" "\n"
	"\t" "\t" "move    $2,$8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".end    memcpy" "\n"
);
#endif

#ifdef F_memset
/* Assembler version of memset using quadword instructions.

   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.

   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */

__asm__
(
	"\t" "\t" ".globl  memset" "\n"
	"\t" "\t" ".ent    memset" "\n"
	"\t" "\t" ".weak   memset" "\n"
	"\t" "memset:" "\n"
	"\t" "\t" ".frame  $sp,0,$31" "\n" // vars= 0, regs= 0/0, args= 0, extra= 0
	"\t" "\t" ".mask   0x00000000,0" "\n"
	"\t" "\t" ".fmask  0x00000000,0" "\n"

#ifndef __OPTIMIZE_SIZE__

	/* if not setting a double word or more, go and set one byte at a time */

	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L2" "\n"
	"\t" "\t" "move    $3,$4" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* if not aligned on a quadword boundary, set one byte at a time */

	"\t" "\t" "andi    $2,$4,0xf" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $2,$0,$L2" "\n"
	"\t" "\t" "move    $7,$4" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* otherwise, build a double word containing the bytes to set */

	"\t" "\t" "andi    $9,$5,0xff" "\n"
	"\t" "\t" "sltu    $10,$6,32" "\n"
	"\t" "\t" "move    $8,$9" "\n"
	"\t" "\t" "dsll    $3,$8,8" "\n"
	"\t" "\t" "or  $8,$3,$9" "\n"
	"\t" "\t" "pcpyh   $3,$8" "\n"

	/* check if setting 32 bytes or more, otherwise set 8 bytes at a time */

	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "bne $10,$0,$L31" "\n"
	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "\t" ".set    pop" "\n"

	/* take double word built from the byte to set and make a quadword */

	"\t" "\t" "pcpyld  $8,$3,$3" "\n"

	/* loop while bytes to set >=32 and use quadword stores */

	"\t" "$L12:" "\n"
	"\t" "\t" "sq  $8,0($7)" "\n"
	"\t" "\t" "addu    $6,$6,-32" "\n"
	"\t" "\t" "addu    $7,$7,16" "\n"
	"\t" "\t" "sltu    $2,$6,32" "\n"
	"\t" "\t" "sq  $8,0($7)" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "beq $2,$0,$L12" "\n"
	"\t" "\t" "addu    $7,$7,16" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "b   $L31" "\n"
	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "$L16:" "\n"
	"\t" "\t" "addu    $6,$6,-8" "\n"
	"\t" "\t" "addu    $7,$7,8" "\n"
	"\t" "\t" "sltu    $2,$6,8" "\n"
	"\t" "$L31:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "beql    $2,$0,$L16" "\n"
	"\t" "\t" "sd  $3,0($7)" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" "move    $3,$7" "\n"

#else  /* __OPTIMIZE_SIZE__ */

	"\t" "\t" "move    $3,$4" "\n"

#endif /* __OPTIMIZE_SIZE__ */

	/* loop while bytes left to set and set one byte at a time */

	"\t" "$L2:" "\n"
	"\t" "\t" "li  $2,4294901760" "\n" // 0xffff0000
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" "ori $2,$2,0xffff" "\n"
	"\t" "\t" "beq $6,$2,$L19" "\n"
	"\t" "\t" "li  $2,4294901760" "\n" // 0xffff0000
	"\t" "\t" "ori $2,$2,0xffff" "\n"

	"\t" "$L20:" "\n"
	"\t" "\t" "sb  $5,0($3)" "\n"
	"\t" "\t" "addu    $6,$6,-1" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "bne $6,$2,$L20" "\n"
	"\t" "\t" "addu    $3,$3,1" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "$L19:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    nomacro" "\n"
	"\t" "\t" "j   $31" "\n"
	"\t" "\t" "move    $2,$4" "\n"
	"\t" "\t" ".set    pop" "\n"

	"\t" "\t" ".end    memset" "\n"
);
#endif

