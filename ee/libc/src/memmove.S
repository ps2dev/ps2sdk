/* Assembler version of memmove using quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	memmove
	.text
	.ent	memmove
	.weak	memmove
memmove:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	move	$8,$4

/* check if an overlapped move is being performed in which case
   perform a move backwards byte by byte */

	sltu	$2,$5,$8
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2
	move	$3,$8
	.set	macro
	.set	reorder

	addu	$7,$5,$6
	sltu	$2,$8,$7
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2
	li	$2,-1			# 0xffffffffffffffff
	.set	macro
	.set	reorder

/* overlapped move, perform backwards copy byte by byte */

	addu	$3,$8,$6
	addu	$6,$6,-1
	.set	noreorder
	.set	nomacro
	beq	$6,$2,$L7
	move	$5,$7
	.set	macro
	.set	reorder

	move	$4,$2
$L5:
	addu	$5,$5,-1
	addu	$3,$3,-1
	lbu	$2,0($5)
	addu	$6,$6,-1
	.set	noreorder
	.set	nomacro
	bne	$6,$4,$L5
	sb	$2,0($3)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$8
	.set	macro
	.set	reorder

/* otherwise, non-overlapped copy */

$L2:

#ifndef __OPTIMIZE_SIZE__

/* if less than 32 bytes or not quad-aligned, just move byte by byte */

	sltu	$2,$6,32
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L29
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	or	$2,$5,$3
	andi	$2,$2,0xf
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L29
	addu	$6,$6,-1
	.set	macro
	.set	reorder

	move	$7,$3

/* while bytes to copy >=32, move bytes using quadword instructions */

$L11:
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
	beq	$4,$0,$L11
	addu	$7,$7,16
	.set	macro
	.set	reorder


/* while bytes left to copy >= 8, move doublewords at a time */

	sltu	$2,$6,8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L30
	move	$3,$7
	.set	macro
	.set	reorder

$L15:
	ld	$3,0($5)
	addu	$6,$6,-8
	addu	$5,$5,8
	sltu	$2,$6,8
	sd	$3,0($7)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L15
	addu	$7,$7,8
	.set	macro
	.set	reorder

	move	$3,$7

#else /* __OPTIMIZE_SIZE__ */

	move	$3,$4

#endif /* __OPTIMIZE_SIZE__ */

/* move remaining bytes one byte at a time */

$L30:
	addu	$6,$6,-1
$L29:
	li	$2,-1			# 0xffffffffffffffff
	.set	noreorder
	.set	nomacro
	beq	$6,$2,$L7
	move	$4,$2
	.set	macro
	.set	reorder

$L19:
	lbu	$2,0($5)
	addu	$6,$6,-1
	addu	$5,$5,1
	sb	$2,0($3)
	.set	noreorder
	.set	nomacro
	bne	$6,$4,$L19
	addu	$3,$3,1
	.set	macro
	.set	reorder

$L7:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$8
	.set	macro
	.set	reorder
	
	.end	memmove




