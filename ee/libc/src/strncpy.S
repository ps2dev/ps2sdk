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
