/* Assembler version of memcmp using quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	memcmp
	.text
	.ent	memcmp
	.weak	memcmp
memcmp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

#ifndef __OPTIMIZE_SIZE__

/* check if length to compare >= 16 and both src and dest quad aligned */

	sltu	$2,$6,16
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L2
	or	$2,$4,$5
	.set	macro
	.set	reorder

	andi	$2,$2,0xf
	bne	$2,$0,$L2
	
/* while bytes to compare >= 16 perform quadword compares */

$L5:
	lq	$3,0($4)
	sltu	$7,$6,32
	lq	$2,0($5)
	addu	$4,$4,16
	pxor	$8,$2,$3
	addu	$2,$5,16
	pcpyud	$10,$8,$7
	or	$9,$10,$8
	movz	$5,$2,$9
	
/* when quadwords are not equal, compare a byte at a time */

	.set	noreorder
	.set	nomacro	
	bnel	$9,$0,$L2
	addu	$4,$4,-16
	.set	macro
	.set	reorder
	.set	noreorder
	.set	nomacro
	beq	$7,$0,$L5
	addu	$6,$6,-16
	.set	macro
	.set	reorder

#endif /* !__OPTIMIZE_SIZE__ */

/* compare remaining bytes one at a time */

$L2:
	li	$2,4294901760			# 0xffff0000
	addu	$6,$6,-1
	ori	$2,$2,0xffff
	beq	$6,$2,$L9
	li	$7,4294901760			# 0xffff0000
	ori	$7,$7,0xffff

$L10:
	lbu	$3,0($4)
	lbu	$2,0($5)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L11
	addu	$4,$4,1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$31
	subu	$2,$3,$2
	.set	macro
	.set	reorder

$L11:
	addu	$6,$6,-1
	.set	noreorder
	.set	nomacro
	bne	$6,$7,$L10
	addu	$5,$5,1
	.set	macro
	.set	reorder

$L9:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	memcmp
