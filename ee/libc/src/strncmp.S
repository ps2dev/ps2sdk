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

/* Assembler version of strncmp that uses quadword instructions.

   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.p2align 3
	.globl	strncmp
	.text
	.ent	strncmp
	.weak	strncmp
strncmp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

/* if n is 0, simply return 0, otherwise must compare strings */
		
#ifdef __OPTIMIZE_SIZE__
	
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L30
	lb	$3,0($4)
	.set	macro
	.set	reorder
	
#else /* __OPTIMIZE_SIZE__ */
	
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L3
	or	$3,$4,$5
	.set	macro
	.set	reorder
	
#endif /* !_OPTIMIZE_SIZE__ */
	
$L20:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.p2align 2
	
#ifndef __OPTIMIZE_SIZE__

/* check if src and dest are doubleword aligned, quadword aligned, or neither in which
   case go do a byte by byte comparison */
	 
$L3:	
	andi	$2,$3,0x7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L22
	move	$2,$6
	.set	macro
	.set	reorder

	andi	$2,$3,0xf
	sltu	$7,$6,16
	dli	$9,0x0101010101010101
	or	$2,$2,$7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L31
	move	$7,$4
	.set	macro
	.set	reorder

/* data is quadword aligned, compare a quadword at a time while n >= 16 */
		
	lq	$3,0($4)
	pcpyld	$10,$9,$9
	lq	$2,0($5)
	dli	$8,0x8080808080808080
	psubw	$3,$3,$2
	pcpyld	$9,$8,$8
	pcpyud	$2,$3,$4
	move	$8,$5
	or	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L22
	move	$2,$6
	.set	macro
	.set	reorder
	addu	$6,$6,-16
	
/* quadwords are equal, check for null terminator in string.  Otherwise, continue
   looping checking a quadword at a time until either a null terminator is found,
   the strings are unequal, or n < 16 in which case compare a byte at a time. */
	
$L34:
	beq	$6,$0,$L20
	lq	$2,0($7)
	#nop
	pnor	$3,$0,$2
	psubb	$2,$2,$10
	pand	$2,$2,$3
	pand	$3,$2,$9
	pcpyud	$2,$3,$4
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L20
	addu	$7,$7,16
	.set	macro
	.set	reorder

	sltu	$2,$6,16
	lq	$3,0($7)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L6
	addu	$8,$8,16
	.set	macro
	.set	reorder

	lq	$2,0($8)
	psubw	$3,$3,$2
	pcpyud	$2,$3,$4
	or	$2,$2,$3

	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L34
	addu	$6,$6,-16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L23
	move	$4,$7
	.set	macro
	.set	reorder

/* data is doubleword aligned, compare a doubleword at a time while n >= 8 */
		
$L31:	
	sltu	$2,$6,8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L6
	move	$8,$5
	.set	macro
	.set	reorder

	ld	$3,0($4)
	ld	$2,0($5)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L22
	move	$2,$6
	.set	macro
	.set	reorder

	addu	$6,$6,-8
	dli	$10,0x8080808080808080
	.p2align 3

/* doublewords are equal, check for null terminator in string.  Otherwise, continue
   looping checking a doubleword at a time until either a null terminator is found,
   the strings are unequal, or n < 8 at which case compare a byte at a time. */
	
$L24:
	beq	$6,$0,$L20
	ld	$2,0($7)
	#nop
	nor	$3,$0,$2
	dsubu	$2,$2,$9
	and	$2,$2,$3
	and	$2,$2,$10
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L20
	addu	$7,$7,8
	.set	macro
	.set	reorder

	sltu	$2,$6,8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L6
	addu	$8,$8,8
	.set	macro
	.set	reorder

	ld	$3,0($7)
	ld	$2,0($8)
	#nop
	.set	noreorder
	.set	nomacro
	beql	$3,$2,$L24
	addu	$6,$6,-8
	.set	macro
	.set	reorder

$L6:
	move	$4,$7
$L23:
	move	$5,$8
	move	$2,$6
$L22:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L13
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L21
	lb	$3,0($4)
	.set	macro
	.set	reorder

#endif /* !__OPTIMIZE_SIZE__ */

/* compare a byte at a time until there are no more characters to compare, a null
   terminator is reached, or the bytes are unequal.  */
	
	.p2align 3
$L14:
	beq	$6,$0,$L20
	.set	noreorder
	.set	nomacro
	beq	$7,$0,$L20
	addu	$4,$4,1
	.set	macro
	.set	reorder

	addu	$5,$5,1
	lb	$3,0($4)

	
$L30:	
	addu	$6,$6,-1
$L21:
	lb	$2,0($5)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L14
	lbu	$7,0($4)
	.set	macro
	.set	reorder

$L13:
	lbu	$2,0($5)
	lbu	$3,0($4)
	.set	noreorder
	.set	nomacro
	j	$31
	subu	$2,$3,$2
	.set	macro
	.set	reorder

	.end	strncmp
