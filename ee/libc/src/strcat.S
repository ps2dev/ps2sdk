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

/* Assembler version of strcat that uses quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	strcat
	.text
	.ent	strcat
	.weak	strcat
strcat:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 0, extra= 0
	.mask	0x80010000,-16
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sq	$16,0($sp)
	move	$16,$4

#ifndef __OPTIMIZE_SIZE__

/* check if dest string is doubleword aligned, quadword aligned or neither in which
   case check for null terminator a byte at a time */
	
	andi	$2,$16,0x7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L15
	sq	$31,16($sp)
	.set	macro
	.set	reorder

	andi	$2,$16,0xf
	dli	$3,0x0101010101010101
	dli	$4,0x8080808080808080
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L3
	ld	$6,0($16)
	.set	macro
	.set	reorder

/* dest string is quadword aligned, load a quadword at a time looking for a null terminator.
   Loop until null terminator is found and then search last quadword for actual position
   of null terminator. */
		
	lq	$2,0($16)
	pcpyld	$7,$3,$3
	pcpyld	$8,$4,$4
	psubb	$3,$2,$7
	pnor	$2,$0,$2
	pand	$3,$3,$2
	pand	$3,$3,$8
	pcpyud	$2,$3,$5
	or	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L9
	move	$4,$16
	.set	macro
	.set	reorder

	addu	$6,$4,16
$L14:
	lq	$2,0($6)
	#nop
	pnor	$3,$0,$2
	psubb	$2,$2,$7
	pand	$2,$2,$3
	pand	$2,$2,$8
	pcpyud	$3,$2,$5
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L14
	addu	$6,$6,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L9
	move	$4,$6
	.set	macro
	.set	reorder

/* dest is doubleword aligned.  Load a doubleword at a time, looking for a null terminator.
   Loop until null terminator is found and then check last doubleword for actual position
   of null terminator. */
	
$L3:	
	dsubu	$3,$6,$3
	nor	$2,$0,$6
	and	$3,$3,$2
	and	$3,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L9
	move	$4,$16
	.set	macro
	.set	reorder

	dli	$7,0x0101010101010101
	addu	$6,$16,8
$L16:
	ld	$2,0($6)
	#nop
	nor	$3,$0,$2
	dsubu	$2,$2,$7
	and	$2,$2,$3
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L16
	addu	$6,$6,8
	.set	macro
	.set	reorder

$L4:
	move	$4,$6
	
#else /* __OPTIMIZE_SIZE__ */
	
	sq	$31,16($sp)

#endif /* __OPTIMIZE_SIZE__ */

/* search a byte at a time for null terminator of dest string and then copy src string
   to this position using strcpy to complete concatenation. */
	
$L9:				
$L15:
	lb	$2,0($4)
	#nop
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L9
	addu	$4,$4,1
	.set	macro
	.set	reorder

	jal	strcpy
	move	$2,$16
	lq	$31,16($sp)
	lq	$16,0($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	strcat









