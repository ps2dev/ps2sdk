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
