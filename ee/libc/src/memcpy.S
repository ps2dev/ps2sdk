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
*/

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
