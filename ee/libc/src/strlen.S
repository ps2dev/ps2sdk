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
