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

/* Assembler version of strncat that uses quadword instructions. 
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	 .text
	.p2align 3
	.globl	strncat
	.text
	.ent	strncat
	.weak	strncat
strncat:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	move	$8,$4
	
#ifndef __OPTIMIZE_SIZE__

/* check if string to concatenate to is doubleword aligned, quadword aligned,
   or neither in which case must search for null terminator one byte at a time. */
	
	andi	$2,$8,0x7
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L18
	lb	$2,0($4)
	.set	macro
	.set	reorder

	andi	$3,$8,0xf
	.set	noreorder
	.set	nomacro
	bnel	$3,$0,$L17
	ld	$2,0($8)
	.set	macro
	.set	reorder

/* string to concatenate to is quadword aligned.  Loop, checking a quadword at a time
   for the null terminator.  If the null terminator is found, go check the last
   quadword fetched a byte at a time to find the actual position, otherwise,
   continue looping and checking quadwords. */
	
$L37:
	dli	$7,0x0101010101010101
	lq	$2,0($8)
	pcpyld	$9,$7,$7
	dli	$4,0x8080808080808080
	psubb	$3,$2,$9
	pnor	$2,$0,$2
	pcpyld	$10,$4,$4
	pand	$3,$3,$2
	pand	$3,$3,$10
	pcpyud	$2,$3,$4
	or	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L5
	move	$7,$8
	.set	macro
	.set	reorder

	addu	$7,$7,16
	.p2align 3
$L39:
	lq	$2,0($7)
	pnor	$3,$0,$2
	psubb	$2,$2,$9
	pand	$2,$2,$3
	pand	$2,$2,$10
	pcpyud	$3,$2,$4
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L39
	addu	$7,$7,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L5
	addu	$7,$7,-16
	.set	macro
	.set	reorder

 /* string to concatenate to is doubleword aligned.  Loop, checking a doubleword at a time
   for the null terminator.  If the null terminator is found, go check the last
   doubleword fetched a byte at a time to find the actual position, otherwise,
   continue looping and checking doublewords. */
	
$L17:	
	dli	$9,0x0101010101010101
	dli	$4,0x8080808080808080
	dsubu	$3,$2,$9
	nor	$2,$0,$2
	and	$3,$3,$2
	and	$3,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L5
	move	$7,$8
	.set	macro
	.set	reorder

	addu	$7,$7,8
	.p2align 3
$L19:
	ld	$2,0($7)
	nor	$3,$0,$2
	dsubu	$2,$2,$9
	and	$2,$2,$3
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L19
	addu	$7,$7,8
	.set	macro
	.set	reorder

$L5:
	move	$4,$7

#endif /* !__OPTIMIZE_SIZE__ */
	
	lb	$2,0($4)

/* find actual position of null terminator in string to be concatenated to */
		
$L18:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L9
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	addu	$4,$4,1
	.p2align 3
$L20:
	lb	$2,0($4)
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L20
	addu	$4,$4,1
	.set	macro
	.set	reorder

/* now start concatenating second string starting at null position.  Concatenation
   is a byte at a time until either the null terminator is reached or n is exhausted.
   A null terminator is added to the end of the result.	 */
	
$L9:
	li	$2,4294901760			# 0xffff0000
	ori	$2,$2,0xffff
	beq	$6,$2,$L13
	lbu	$2,0($5)
	addu	$5,$5,1
	sb	$2,0($4)
	sll	$2,$2,24
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L13
	addu	$4,$4,1
	.set	macro
	.set	reorder

	li	$3,4294901760			# 0xffff0000
	ori	$3,$3,0xffff
	.p2align 3
$L14:
	.set	noreorder
	.set	nomacro
	beql	$6,$0,$L12
	sb	$0,0($4)
	.set	macro
	.set	reorder

$L12:
	addu	$6,$6,-1
	beq	$6,$3,$L13
	lbu	$2,0($5)
	addu	$5,$5,1
	sb	$2,0($4)
	sll	$2,$2,24
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L14
	addu	$4,$4,1
	.set	macro
	.set	reorder

$L13:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$8
	.set	macro
	.set	reorder

	.end	strncat
