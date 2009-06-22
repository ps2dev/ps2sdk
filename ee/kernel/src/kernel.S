/*      
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE Kernel functions
*/

#include <syscallnr.h>

	.text
	.p2align 3

#ifdef USE_KMODE
#define SYSCALL(name) 		\
	.set	push;		\
	.set	noreorder;	\
	.text;			\
	.align 4;		\
	.globl	name;		\
	.type	name,@function;	\
	.ent	name,0;		\
name:	j	__syscall;	\
	li	$3, __NR_##name;\
	nop;			\
	.end	name;		\
	.size	name,.-name;	\
	.set	pop;		
#else
#define SYSCALL(name) 		\
	.set	push;		\
	.set	noreorder;	\
	.text;			\
	.align 4;		\
	.globl	name;		\
	.type	name,@function;	\
	.ent	name,0;		\
name:	li	$3, __NR_##name;\
	syscall;		\
	jr	$31;		\
	nop;			\
	.end	name;		\
	.size	name,.-name;	\
	.set	pop;		
#endif

#ifdef F___syscall
.set	push
.set	noreorder
.text
.align	4
.globl	__syscall
.type	__syscall,@function
.ent	__syscall,0
__syscall:
        mfc0        $2, $12
        andi        $2, $2, 0x18
        beqz        $2, _kMode
        nop
        syscall
        jr          $31
        nop
_kMode:
        sll         $3, $3, 2
        lui         $26, 0x8000
        lhu         $2, 0x02F0($26)
        sll         $2, $2, 16
        lh          $26, 0x02F8($26)
        add         $2, $26
        addu        $3, $2
        lw          $26, 0x00($3)
        jr          $26
        nop
.end	__syscall
.size	__syscall,.-__syscall
.set	pop
#endif

#ifdef F_ResetEE
SYSCALL(ResetEE)
#endif

#ifdef F_SetGsCrt
SYSCALL(SetGsCrt)
#endif

#ifdef F_Exit
SYSCALL(Exit)
#endif

#ifdef F_LoadExecPS2
SYSCALL(LoadExecPS2)
#endif

#ifdef F_ExecPS2
SYSCALL(ExecPS2)
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

#ifdef F_SetAlarm
SYSCALL(SetAlarm)
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

#ifdef F_iSetAlarm
SYSCALL(iSetAlarm)
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

#ifdef F_iRotateThreadReadyQueue
SYSCALL(iRotateThreadReadyQueue)
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

#ifdef F_iWakeupThread
SYSCALL(iWakeupThread)
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

#ifdef F_iSuspendThread
SYSCALL(iSuspendThread)
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

#ifdef F_SifStopDma
SYSCALL(SifStopDma)
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

//#ifdef F_print
//SYSCALL(_print)
//#endif

#ifdef F_SifDmaStat
SYSCALL(SifDmaStat)
#endif

#ifdef F_iSifDmaStat
SYSCALL(iSifDmaStat)
#endif

#ifdef F_SifSetDma
SYSCALL(SifSetDma)
#endif

#ifdef F_iSifSetDma
SYSCALL(iSifSetDma)
#endif

#ifdef F_SifSetDChain
SYSCALL(SifSetDChain)
#endif

#ifdef F_iSifSetDChain
SYSCALL(iSifSetDChain)
#endif

#ifdef F_SifSetReg
SYSCALL(SifSetReg)
#endif

#ifdef F_SifGetReg
SYSCALL(SifGetReg)
#endif

#ifdef F_ExecOSD
SYSCALL(ExecOSD)
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


#ifdef F_SifWriteBackDCache

	.globl	SifWriteBackDCache
	.ent	SifWriteBackDCache
	.set	noreorder

SifWriteBackDCache:

	lui	$25, 0xffff
	ori	$25, $25, 0xffc0
	blez	$5, last
	addu	$10, $4, $5
	and	$8, $4, $25
	addiu	$10, $10, -1
	and	$9, $10, $25
	subu	$10, $9, $8
	srl	$11, $10, 0x6
	addiu	$11, $11, 1
	andi	$9, $11, 0x7
	beqz	$9, eight
	srl	$10, $11, 0x3
loop1:
	sync
	cache	0x18, 0($8)
	sync
	addiu	$9, $9, -1
	nop
	bgtz	$9, loop1
	addiu	$8, $8, 64

eight:
	beqz	$10, last
loop8:
	addiu	$10, $10, -1
	sync
	cache	0x18, 0($8)
	sync
	cache	0x18, 64($8)
	sync
	cache	0x18, 128($8)
	sync
	cache	0x18, 192($8)
	sync
	cache	0x18, 256($8)
	sync
	cache	0x18, 320($8)
	sync
	cache	0x18, 384($8)
	sync
	cache	0x18, 448($8)
	sync
	bgtz	$10, loop8
	addiu	$8, $8, 512
last:
	jr	$31
	nop

	.end	SifWriteBackDCache
#endif

#define DXWBIN	0x14		/* Data cache: Index Writeback Invalidate.  */
#define DXIN	0x16		/* Data cache: Index Invalidate.  */

#define opDCache(name, op)	\
	.set	push;		\
	.set	noreorder;	\
	.set	nomacro;	\
	.text;			\
	.p2align 3;		\
	.globl	name;		\
	.type	name,@function;	\
	.ent	name, 0;	\
name:	lui	$7, 0xffff;	\
	daddu	$6, $0, $0;	\
	ori	$7, 0xf000;	\
	nop;			\
1:	sync;			\
	cache	0x10, 0($6);	\
	sync;			\
	mfc0	$2, $28;	\
	and	$2, $7;		\
	addu	$2, $6;		\
	sltu	$3, $5, $2;	\
	sltu	$2, $4;		\
	bnez	$2, 2f;		\
	nop;			\
	bnez	$3, 2f;		\
	nop;			\
	sync;			\
	cache	op, 0($6);	\
	sync;			\
2:	sync;			\
	cache	0x10, 1($6);	\
	sync;			\
	mfc0	$2, $28;	\
	and	$2, $7;		\
	addu	$2, $6;		\
	sltu	$3, $5, $2;	\
	sltu	$2, $4;		\
	bnez	$2, 3f;		\
	nop;			\
	bnez	$3, 3f;		\
	nop;			\
	sync;			\
	cache	op, 1($6);	\
	sync;			\
3:	sync;			\
	addiu	$6, 64;		\
	slti	$2, $6, 4096;	\
	bnez	$2, 1b;		\
	nop;			\
	jr	$31;		\
	nop;			\
	.end	name;		\
	.size	name,.-name;	\
	.set	pop;

#ifdef F__SyncDCache
opDCache(_SyncDCache, DXWBIN)
#endif

#ifdef F__InvalidDCache
opDCache(_InvalidDCache, DXIN)
#endif

#ifdef F___errno
/* This is needed in case we are linked against libm (the math library) but
   not libc.  */
	.globl	__errno
	.ent	__errno
	.weak	__errno

__errno:
	la	$2, errno
	jr	$31
	nop
	.end	__errno
#endif

#ifdef F_errno
/* New applications compiled against ps2lib that use errno will resolve to
   this, while existing newlib applications will resolve to __errno.  */
	.globl	errno
	.weak	errno

	.data
errno:
	.space	4
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
	
	.text
	.align	2
	.globl	strlen
	.text
	.ent	strlen
	.weak	strlen
strlen:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

#ifndef __OPTIMIZE_SIZE__

/* check if src is quadword aligned, doubleword aligned, or neither in which case
   perform checking for null terminator one byte at a time. */
	
	andi	$2,$4,0x7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L15
	move	$7,$4
	.set	macro
	.set	reorder

	andi	$3,$4,0xf
	dli	$2,0x0101010101010101
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L12
	move	$5,$4
	.set	macro
	.set	reorder

/* src is quadword aligned.  Load a quadword at a time and check for null terminator.
   If null terminator is found, go and find exact position by looking at each byte
   of the last quadword.  Otherwise, continue to next quadword and keep searching. */
	
	lq	$3,0($5)
	pcpyld	$8,$2,$2
	dli	$4,0x8080808080808080
	psubb	$2,$3,$8
	pnor	$3,$0,$3
	pcpyld	$9,$4,$4
	pand	$2,$2,$3
	pand	$2,$2,$9
	pcpyud	$3,$2,$8
	or	$6,$3,$2
	.set	noreorder
	.set	nomacro
	bnel	$6,$0,$L15
	move	$4,$5
	.set	macro
	.set	reorder

	addu	$5,$5,16
$L14:
	lq	$2,0($5)
	#nop
	pnor	$3,$0,$2
	psubb	$2,$2,$8
	pand	$2,$2,$3
	pand	$4,$2,$9
	pcpyud	$3,$4,$6
	or	$3,$3,$4
	.set	noreorder
	.set	nomacro
	beql	$3,$0,$L14
	addu	$5,$5,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L15
	move	$4,$5
	.set	macro
	.set	reorder

/* src is doubleword aligned.  Load a doubleword at a time and check for null terminator.
   If null terminator is found, go and find exact position by looking at each byte
   of the last doubleword.  Otherwise, continue to next doubleword and keep searching. */
	

$L12:	
	ld	$3,0($5)
	dli	$4,0x8080808080808080
	dsubu	$2,$3,$2
	nor	$3,$0,$3
	and	$2,$2,$3
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L15
	move	$4,$5
	.set	macro
	.set	reorder

	dli	$6,0x0101010101010101
	addu	$5,$5,8
$L16:
	ld	$2,0($5)
	#nop
	nor	$3,$0,$2
	dsubu	$2,$2,$6
	and	$2,$2,$3
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L16
	addu	$5,$5,8
	.set	macro
	.set	reorder

	move	$4,$5
	
#else /* __OPTIMIZE_SIZE__ */
	
	move	$7,$4
	
#endif /* __OPTIMIZE_SIZE__ */

/* search a byte at a time for null terminator and then calculate length by subtracting
   original string address from null terminator address. */
		
$L9:
$L15:
	.set	noreorder
	.set	nomacro

	lb	$2,0($4)
	#nop
	beq	$2,$0,1f
	addu	$4,$4,1

	lb	$2,0($4)
	#nop
	bne	$2,$0,$L9
	addu	$4,$4,1
1:
	subu	$4,$4,1

	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$31
	subu	$2,$4,$7
	.set	macro
	.set	reorder

	.end	strlen
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
	
	.text
	.p2align 3
	.globl	strncpy
	.text
	.ent	strncpy
	.weak	strncpy
strncpy:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	move	$8,$4
	
#ifndef __OPTIMIZE_SIZE__

/* check if src and dest are doubleword aligned, quadword aligned, or neither in which
   case copy byte by byte */
	
	or	$7,$5,$4
	li	$10,0x10
	andi	$2,$7,0x7
	li	$9,0x8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L9
	andi	$2,$7,0xf
	.set	macro
	.set	reorder
	movz	$9,$10,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L17
	sltu	$2,$6,$9
	.set	macro
	.set	reorder

/* src and dest are quadword aligned.  Check a quadword at a time looking for a
   null terminator or until the nth byte is reached.  Otherwise, copy the
   quadword to dest and continue looping checking quadwords.   Once a null
   terminator is found or n is < 16 go copy a byte at a time. */
	 
$L31:	
	bne	$2,$0,$L9
	dli	$7,0x0101010101010101
	lq	$3,0($5)
	pcpyld	$9,$7,$7
	pnor	$3,$0,$3
	dli	$7,0x8080808080808080
	psubb	$2,$3,$9
	pcpyld	$10,$7,$7
	pand	$2,$2,$3
	pand	$2,$2,$10
	pcpyud	$3,$2,$4
	or	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L5
	move	$7,$8
	.set	macro
	.set	reorder
	lq	$3,0($5)
	.p2align 3
$L39:		
	addu	$6,$6,-16
	addu	$5,$5,16
	sltu	$2,$6,16
	sq	$3,0($7)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L5
	addu	$7,$7,16
	.set	macro
	.set	reorder

	lq	$2,0($5)
	#nop
	pnor	$3,$0,$2
	psubb	$2,$2,$9
	pand	$2,$2,$3
	pand	$2,$2,$10
	pcpyud	$3,$2,$4
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L19
	lq	$3,0($5)
	b	$L9
	move	$4,$7
	.set	macro
	.set	reorder
					
/* src and dest are quadword aligned.  Check a quadword at a time looking for a
   null terminator or until the nth byte is reached.  Otherwise, copy the
   quadword to dest and continue looping checking quadwords.   Once a null
   terminator is found or n is < 16 go copy a byte at a time. */
	
$L17:		
	bne	$2,$0,$L9
	ld	$3,0($5)
	dli	$9,0x0101010101010101
	dli	$10,0x8080808080808080
	dsubu	$2,$3,$9
	nor	$3,$0,$3
	and	$2,$2,$3
	and	$2,$2,$10
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L5
	move	$7,$8
	.set	macro
	.set	reorder
	ld	$3,0($5)
	.p2align 3
$L19:
	addu	$6,$6,-8
	addu	$5,$5,8
	sltu	$2,$6,8
	sd	$3,0($7)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L5
	addu	$7,$7,8
	.set	macro
	.set	reorder

	ld	$2,0($5)
	#nop
	nor	$3,$0,$2
	dsubu	$2,$2,$9
	and	$2,$2,$3
	and	$2,$2,$10
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L19
	ld	$3,0($5)
	.set	macro
	.set	reorder

$L5:
	move	$4,$7
	
#endif /* !__OPTIMIZE_SIZE__ */

/* check a byte at a time looking for either the null terminator or until n bytes are
   copied.  If the null terminator is found and n is not reached yet, copy null
   bytes until n is reached. */
		
	.p2align 3
$L9:
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L18
	move	$2,$6
	.set	macro
	.set	reorder

	lbu	$2,0($5)
	addu	$6,$6,-1
	addu	$5,$5,1
	sb	$2,0($4)
	sll	$2,$2,24
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L9
	addu	$4,$4,1
	.set	macro
	.set	reorder

	move	$2,$6
$L20:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L18
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	.p2align 3
$L16:
	sb	$0,0($4)
	move	$2,$6
	addu	$4,$4,1
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L16
	addu	$6,$6,-1
	.set	macro
	.set	reorder

$L18:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$8
	.set	macro
	.set	reorder

	.end	strncpy
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
	
	.text
	.align	2
	.globl	memcpy
	.text
	.ent	memcpy
	.weak	memcpy
memcpy:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	move	$8,$4

#ifndef __OPTIMIZE_SIZE__

/* if bytes to move are < 32 or src or dest are not quadword aligned, jump to
   code that moves one byte at a time */

	sltu	$2,$6,32
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	move	$3,$8
	.set	macro
	.set	reorder

	or	$2,$5,$8
	andi	$2,$2,0xF
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L20
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	move	$7,$8

/* while remainder to move is >= 32 bytes, use LQ/SQ quadword instructions
   to move data */

$L5:
	lq	$3,0($5)
	addu	$6,$6,-32
	addu	$5,$5,16
	sltu	$4,$6,32
	sq	$3,0($7)
	addu	$7,$7,16
	lq	$2,0($5)
	addu	$5,$5,16
	sq	$2,0($7)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L5
	addu	$7,$7,16
	.set	macro
	.set	reorder

	sltu	$2,$6,8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	move	$3,$7
	.set	macro
	.set	reorder

/* while remainder to move is >= 8 bytes, use LD/SD doubleword instructions
   to move data */

$L9:
	ld	$3,0($5)
	addu	$6,$6,-8
	addu	$5,$5,8
	sltu	$2,$6,8
	sd	$3,0($7)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L9
	addu	$7,$7,8
	.set	macro
	.set	reorder

	move	$3,$7

#else  /* __OPTIMIZE_SIZE__ */

	move	$3,$8

#endif /* !__OPTIMIZE_SIZE__ */

/* Move any remaining bytes one at a time */

$L2:
	addu	$6,$6,-1
$L20:
	li	$2,-1			# 0xffffffffffffffff
	.set	noreorder
	.set	nomacro
	beq	$6,$2,$L12
	move	$4,$2
	.set	macro
	.set	reorder

$L13:
	lbu	$2,0($5)
	addu	$6,$6,-1
	addu	$5,$5,1
	sb	$2,0($3)
	.set	noreorder
	.set	nomacro
	bne	$6,$4,$L13
	addu	$3,$3,1
	.set	macro
	.set	reorder

$L12:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$8
	.set	macro
	.set	reorder

	.end	memcpy
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
	
	.text
	.align	2
	.globl	memset
	.text
	.ent	memset
	.weak	memset
memset:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

#ifndef __OPTIMIZE_SIZE__

/* if not setting a double word or more, go and set one byte at a time */

	sltu	$2,$6,8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	move	$3,$4
	.set	macro
	.set	reorder

/* if not aligned on a quadword boundary, set one byte at a time */

	andi	$2,$4,0xf
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	move	$7,$4
	.set	macro
	.set	reorder

/* otherwise, build a double word containing the bytes to set */

	andi	$9,$5,0xff
	sltu	$10,$6,32
	move	$8,$9
	dsll	$3,$8,8
	or	$8,$3,$9
	pcpyh	$3,$8
	.set	noreorder
	.set	nomacro

/* check if setting 32 bytes or more, otherwise set 8 bytes at a time */

	bne	$10,$0,$L31
	sltu	$2,$6,8
	.set	macro
	.set	reorder

/* take double word built from the byte to set and make a quadword */

	pcpyld	$8,$3,$3

/* loop while bytes to set >=32 and use quadword stores */

$L12:
	sq	$8,0($7)
	addu	$6,$6,-32
	addu	$7,$7,16
	sltu	$2,$6,32
	sq	$8,0($7)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L12
	addu	$7,$7,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L31
	sltu	$2,$6,8
	.set	macro
	.set	reorder

$L16:
	addu	$6,$6,-8
	addu	$7,$7,8
	sltu	$2,$6,8
$L31:
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L16
	sd	$3,0($7)
	.set	macro
	.set	reorder

	move	$3,$7

#else  /* __OPTIMIZE_SIZE__ */
 
	move	$3,$4

#endif /* __OPTIMIZE_SIZE__ */

/* loop while bytes left to set and set one byte at a time */

$L2:
	li	$2,4294901760			# 0xffff0000
	addu	$6,$6,-1
	ori	$2,$2,0xffff
	beq	$6,$2,$L19
	li	$2,4294901760			# 0xffff0000
	ori	$2,$2,0xffff

$L20:
	sb	$5,0($3)
	addu	$6,$6,-1
	.set	noreorder
	.set	nomacro
	bne	$6,$2,$L20
	addu	$3,$3,1
	.set	macro
	.set	reorder

$L19:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$4
	.set	macro
	.set	reorder

	.end	memset
#endif
