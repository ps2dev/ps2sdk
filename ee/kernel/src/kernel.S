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

	.text
	.p2align 3

#ifdef USE_KMODE
#define SYSCALL(name, number) \
	.set	push;		\
	.set	noreorder;	\
	.text;			\
	.align 4;		\
	.globl	name;		\
	.type	name,@function;	\
	.ent	name,0;		\
name:	j	__syscall;	\
	li	$3, number;	\
	nop;			\
	.end	name;		\
	.size	name,.-name;	\
	.set	pop;		
#else
#define SYSCALL(name, number) \
	.set	push;		\
	.set	noreorder;	\
	.text;			\
	.align 4;		\
	.globl	name;		\
	.type	name,@function;	\
	.ent	name,0;		\
name:	li	$3, number;	\
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
        mfc0        $v0, $12
        andi        $v0, $v0, 0x18
        beqz        $v0, _kMode
        nop
        syscall
        jr          $ra
        nop
_kMode:
        sll         $v1, $v1, 2
        lui         $k0, 0x8000
        lhu         $v0, 0x02F0($k0)
        sll         $v0, $v0, 16
        lh          $k0, 0x02F8($k0)
        add         $v0, $k0
        addu        $v1, $v0
        lw          $k0, 0x00($v1)
        jr          $k0
        nop
.end	__syscall
.size	__syscall,.-__syscall
.set	pop
#endif

#ifdef F_ResetEE
SYSCALL(ResetEE, 1)
#endif

#ifdef F_SetGsCrt
SYSCALL(SetGsCrt, 2)
#endif

#ifdef F_Exit
SYSCALL(Exit, 4)
#endif

#ifdef F_LoadExecPS2
SYSCALL(LoadExecPS2, 6)
#endif

#ifdef F_ExecPS2
SYSCALL(ExecPS2, 7)
#endif

#ifdef F_RFU009
SYSCALL(RFU009, 9)
#endif

#ifdef F_AddSbusIntcHandler
SYSCALL(AddSbusIntcHandler, 0xa)
#endif

#ifdef F_RemoveSbusIntcHandler
SYSCALL(RemoveSbusIntcHandler, 0xb)
#endif

#ifdef F_Interrupt2Iop
SYSCALL(Interrupt2Iop, 0xc)
#endif

#ifdef F_SetVTLBRefillHandler
SYSCALL(SetVTLBRefillHandler, 0xd)
#endif

#ifdef F_SetVCommonHandler
SYSCALL(SetVCommonHandler, 0xe)
#endif

#ifdef F_SetVInterruptHandler
SYSCALL(SetVInterruptHandler, 0xf)
#endif

#ifdef F_AddIntcHandler
SYSCALL(AddIntcHandler, 0x10)
#endif

#ifdef F_AddIntcHandler2
SYSCALL(AddIntcHandler2, 0x10)
#endif

#ifdef F_RemoveIntcHandler
SYSCALL(RemoveIntcHandler, 0x11)
#endif

#ifdef F_AddDmacHandler
SYSCALL(AddDmacHandler, 0x12)
#endif

#ifdef F_RemoveDmacHandler
SYSCALL(RemoveDmacHandler, 0x13)
#endif

#ifdef F__EnableIntc
SYSCALL(_EnableIntc, 0x14)
#endif

#ifdef F__DisableIntc
SYSCALL(_DisableIntc, 0x15)
#endif

#ifdef F__EnableDmac
SYSCALL(_EnableDmac, 0x16)
#endif

#ifdef F__DisableDmac
SYSCALL(_DisableDmac, 0x17)
#endif

#ifdef F_SetAlarm
SYSCALL(SetAlarm, 0x18)
#endif

#ifdef F_ReleaseAlarm
SYSCALL(ReleaseAlarm, 0x19)
#endif

#ifdef F__iEnableIntc
SYSCALL(_iEnableIntc,- 0x1a)
#endif

#ifdef F__iDisableIntc
SYSCALL(_iDisableIntc, -0x1b)
#endif

#ifdef F__iEnableDmac
SYSCALL(_iEnableDmac, -0x1c)
#endif

#ifdef F__iDisableDmac
SYSCALL(_iDisableDmac, -0x1d)
#endif

#ifdef F_iSetAlarm
SYSCALL(iSetAlarm, -0x1e)
#endif

#ifdef F_iReleaseAlarm
SYSCALL(iReleaseAlarm, -0x1f)
#endif

#ifdef F_CreateThread
SYSCALL(CreateThread, 0x20)
#endif

#ifdef F_DeleteThread
SYSCALL(DeleteThread, 0x21)
#endif

#ifdef F_StartThread
SYSCALL(StartThread, 0x22)
#endif

#ifdef F_ExitThread
SYSCALL(ExitThread, 0x23)
#endif

#ifdef F_ExitDeleteThread
SYSCALL(ExitDeleteThread, 0x24)
#endif

#ifdef F_TerminateThread
SYSCALL(TerminateThread, 0x25)
#endif

#ifdef F_iTerminateThread
SYSCALL(iTerminateThread, -0x26)
#endif

#ifdef F_DisableDispatchThread
SYSCALL(DisableDispatchThread, 0x27)
#endif

#ifdef F_EnableDispatchThread
SYSCALL(EnableDispatchThread, 0x28)
#endif

#ifdef F_ChangeThreadPriority
SYSCALL(ChangeThreadPriority, 0x29)
#endif

#ifdef F_iChangeThreadPriority
SYSCALL(iChangeThreadPriority, -0x2a)
#endif

#ifdef F_RotateThreadReadyQueue
SYSCALL(RotateThreadReadyQueue, 0x2b)
#endif

#ifdef F_iRotateThreadReadyQueue
SYSCALL(iRotateThreadReadyQueue, -0x2c)
#endif

#ifdef F_ReleaseWaitThread
SYSCALL(ReleaseWaitThread, 0x2d)
#endif

#ifdef F_iReleaseWaitThread
SYSCALL(iReleaseWaitThread, -0x2e)
#endif

#ifdef F_GetThreadId
SYSCALL(GetThreadId, 0x2f)
#endif

#ifdef F_ReferThreadStatus
SYSCALL(ReferThreadStatus, 0x30)
#endif

#ifdef F_iReferThreadStatus
SYSCALL(iReferThreadStatus, -0x31)
#endif

#ifdef F_SleepThread
SYSCALL(SleepThread, 0x32)
#endif

#ifdef F_WakeupThread
SYSCALL(WakeupThread, 0x33)
#endif

#ifdef F_iWakeupThread
SYSCALL(iWakeupThread, -0x34)
#endif

#ifdef F_CancelWakeupThread
SYSCALL(CancelWakeupThread, 0x35)
#endif

#ifdef F_iCancelWakeupThread
SYSCALL(iCancelWakeupThread, -0x36)
#endif

#ifdef F_SuspendThread
SYSCALL(SuspendThread, 0x37)
#endif

#ifdef F_iSuspendThread
SYSCALL(iSuspendThread, -0x38)
#endif

#ifdef F_ResumeThread
SYSCALL(ResumeThread, 0x39)
#endif

#ifdef F_iResumeThread
SYSCALL(iResumeThread, -0x3a)
#endif

#ifdef F_RFU059
SYSCALL(RFU059, 0x3b)
#endif

#ifdef F_RFU060
SYSCALL(RFU060, 0x3c)
#endif

#ifdef F_SetupThread
SYSCALL(SetupThread, 0x3c)
#endif

#ifdef F_RFU061
SYSCALL(RFU061, 0x3d)
#endif

#ifdef F_SetupHeap
SYSCALL(SetupHeap, 0x3d)
#endif

#ifdef F_EndOfHeap
SYSCALL(EndOfHeap, 0x3e)
#endif

#ifdef F_CreateSema
SYSCALL(CreateSema, 0x40)
#endif

#ifdef F_DeleteSema
SYSCALL(DeleteSema, 0x41)
#endif

#ifdef F_SignalSema
SYSCALL(SignalSema, 0x42)
#endif

#ifdef F_iSignalSema
SYSCALL(iSignalSema, -0x43)
#endif

#ifdef F_WaitSema
SYSCALL(WaitSema, 0x44)
#endif

#ifdef F_PollSema
SYSCALL(PollSema, 0x45)
#endif

#ifdef F_iPollSema
SYSCALL(iPollSema, -0x46)
#endif

#ifdef F_ReferSemaStatus
SYSCALL(ReferSemaStatus, 0x47)
#endif

#ifdef F_iReferSemaStatus
SYSCALL(iReferSemaStatus, -0x48)
#endif

#ifdef F_iDeleteSema
SYSCALL(iDeleteSema, -0x49)
#endif

#ifdef F_SetOsdConfigParam
SYSCALL(SetOsdConfigParam, 0x4a)
#endif

#ifdef F_GetOsdConfigParam
SYSCALL(GetOsdConfigParam, 0x4b)
#endif

#ifdef F_GetGsHParam
SYSCALL(GetGsHParam, 0x4c)
#endif

#ifdef F_GetGsVParam
SYSCALL(GetGsVParam, 0x4d)
#endif

#ifdef F_SetGsHParam
SYSCALL(SetGsHParam, 0x4e)
#endif

#ifdef F_SetGsVParam
SYSCALL(SetGsVParam, 0x4f)
#endif

#ifdef F_CreateEventFlag
SYSCALL(CreateEventFlag, 0x50)
#endif

#ifdef F_DeleteEventFlag
SYSCALL(DeleteEventFlag, 0x51)
#endif

#ifdef F_SetEventFlag
SYSCALL(SetEventFlag, 0x52)
#endif

#ifdef F_iSetEventFlag
SYSCALL(iSetEventFlag, 0x53)
#endif

#ifdef F_EnableIntcHandler
SYSCALL(EnableIntcHandler, 0x5C)
#endif

#ifdef F_iEnableIntcHandler
SYSCALL(iEnableIntcHandler, -0x5C)
#endif

#ifdef F_DisableIntcHandler
SYSCALL(DisableIntcHandler, 0x5D)
#endif

#ifdef F_iDisableIntcHandler
SYSCALL(iDisableIntcHandler, -0x5D)
#endif

#ifdef F_EnableDmacHandler
SYSCALL(EnableDmacHandler, 0x5E)
#endif

#ifdef F_iEnableDmacHandler
SYSCALL(iEnableDmacHandler, -0x5E)
#endif

#ifdef F_DisableDmacHandler
SYSCALL(DisableDmacHandler, 0x5F)
#endif

#ifdef F_iDisableDmacHandler
SYSCALL(iDisableDmacHandler, -0x5F)
#endif

#ifdef F_KSeg0
SYSCALL(KSeg0, 0x60)
#endif

#ifdef F_EnableCache
SYSCALL(EnableCache, 0x61)
#endif

#ifdef F_DisableCache
SYSCALL(DisableCache, 0x62)
#endif

#ifdef F_GetCop0
SYSCALL(GetCop0, 0x63)
#endif

#ifdef F_FlushCache
SYSCALL(FlushCache, 0x64)
#endif

#ifdef F_CpuConfig
SYSCALL(CpuConfig, 0x66)
#endif

#ifdef F_iGetCop0
SYSCALL(iGetCop0, -0x67)
#endif

#ifdef F_iFlushCache
SYSCALL(iFlushCache, -0x68)
#endif

#ifdef F_RFU105
SYSCALL(RFU105, 0x69)
#endif

#ifdef F_iCpuConfig
SYSCALL(iCpuConfig, -0x6a)
#endif

#ifdef F_SifStopDma
SYSCALL(SifStopDma, 0x6b)
#endif

#ifdef F_SetCPUTimerHandler
SYSCALL(SetCPUTimerHandler, 0x6c)
#endif

#ifdef F_SetCPUTimer
SYSCALL(SetCPUTimer, 0x6d)
#endif

#ifdef F_SetOsdConfigParam2
SYSCALL(SetOsdConfigParam2, 0x6e)
#endif

#ifdef F_GetOsdConfigParam2
SYSCALL(GetOsdConfigParam2, 0x6f)
#endif

#ifdef F_GsGetIMR
SYSCALL(GsGetIMR, 0x70)
#endif

#ifdef F_iGsGetIMR
SYSCALL(iGsGetIMR, -0x70)
#endif

#ifdef F_GsPutIMR
SYSCALL(GsPutIMR, 0x71)
#endif

#ifdef F_iGsPutIMR
SYSCALL(iGsPutIMR, -0x71)
#endif

#ifdef F_SetPgifHandler
SYSCALL(SetPgifHandler, 0x72)
#endif

#ifdef F_SetVSyncFlag
SYSCALL(SetVSyncFlag, 0x73)
#endif

#ifdef F_SetSyscall
SYSCALL(SetSyscall, 0x74)
#endif

//#ifdef F_print
//SYSCALL(_print, 0x75)
//#endif

#ifdef F_SifDmaStat
SYSCALL(SifDmaStat, 0x76)
#endif

#ifdef F_iSifDmaStat
SYSCALL(iSifDmaStat, -0x76)
#endif

#ifdef F_SifSetDma
SYSCALL(SifSetDma, 0x77)
#endif

#ifdef F_iSifSetDma
SYSCALL(iSifSetDma, -0x77)
#endif

#ifdef F_SifSetDChain
SYSCALL(SifSetDChain, 0x78)
#endif

#ifdef F_iSifSetDChain
SYSCALL(iSifSetDChain, -0x78)
#endif

#ifdef F_SifSetReg
SYSCALL(SifSetReg, 0x79)
#endif

#ifdef F_SifGetReg
SYSCALL(SifGetReg, 0x7a)
#endif

#ifdef F_ExecOSD
SYSCALL(ExecOSD, 0x7b)
#endif

#ifdef F_Deci2Call
SYSCALL(Deci2Call, 0x7c)
#endif

#ifdef F_PSMode
SYSCALL(PSMode, 0x7d)
#endif

#ifdef F_MachineType
SYSCALL(MachineType, 0x7e)
#endif

#ifdef F_GetMemorySize
SYSCALL(GetMemorySize, 0x7f)
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
