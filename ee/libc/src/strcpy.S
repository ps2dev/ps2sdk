/* Assembler version of strcpy using quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	strcpy
	.text
	.ent	strcpy
	.weak	strcpy
strcpy:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	move	$7,$4

#ifndef __OPTIMIZE_SIZE__

/* check if dest and src are quadword aligned, doubleword aligned,
   or neither in which case copy a byte at a time. */
   
	or	$8,$5,$7
	andi	$2,$8,0x7
	.set	noreorder
	.set	nomacro
	
	bne	$2,$0,$L7
	move	$3,$7
	.set	macro
	.set	reorder
	andi	$2,$8,0xf
	dli	$9,0x0101010101010101
	dli	$4,0x8080808080808080
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L3
	ld	$10,0($5)
	.set	macro
	.set	reorder
	
/* data is quadword aligned.  Load a quadword at a time, 
   looking for a null terminator.  If no null terminator
   is found, copy the quadword to dest and check next
   quadword for a null terminator. */
   
	pcpyld	$10,$9,$9
	lq	$9,0($5)
	pcpyld	$8,$4,$4
	psubb	$2,$9,$10
	pnor	$3,$0,$9
	pand	$2,$2,$3
	pand	$2,$2,$8
	pcpyud	$4,$2,$9
	or	$3,$2,$4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L4
	move	$6,$7
	.set	macro
	.set	reorder

$L2:
	sq	$9,0($6)
	addu	$5,$5,16
	lq	$9,0($5)
	psubb	$2,$9,$10
	pnor	$3,$0,$9
	pand	$2,$2,$3
	pand	$2,$2,$8
	pcpyud	$4,$2,$9
	or	$3,$2,$4
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L2
	addu	$6,$6,16 
	.set	macro
	.set	reorder
	.set	noreorder
	.set	nomacro
	b	$L7
	move	$3,$6
	.set	macro
	.set	reorder
	
/* data is not quadword aligned, but doubleword aligned.
   Load a doubleword at a time, checking for a null terminator.
   If no terminator found, copy the doubleword to dest and
   loop and check the next doubleword. */
   
$L3:	 
	dsubu	$2,$10,$9
	nor	$3,$0,$10
	and	$2,$2,$3
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L4
	move	$6,$7
	.set	macro
	.set	reorder

$L5:
	sd	$10,0($6)
	addu	$5,$5,8
	ld	$10,0($5)
	nor	$2,$0,$10
	dsubu	$3,$10,$9
	and	$3,$3,$2
	and	$3,$3,$4
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L5
	addu	$6,$6,8
	.set	macro
	.set	reorder

$L4:
	move	$3,$6

#else /* __OPTIMIZE_SIZE__ */

	move	$3,$4
	
#endif /* __OPTIMIZE_SIZE__ */
	
/* null terminator found, must copy src a byte at a time */

$L7:
	lbu	$2,0($5)
	addu	$5,$5,1
	sb	$2,0($3)
	sll	$2,$2,24
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L7
	addu	$3,$3,1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$7
	.set	macro
	.set	reorder

	.end	strcpy
