/* Assembler version of strcmp using quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	strcmp
	.text
	.ent	strcmp
	.weak	strcmp
strcmp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	
#ifndef __OPTIMIZE_SIZE__

/* check if src and dest are quadword aligned, doubleword aligned
   or neither in which case perform comparision one byte at a time */
   
	or	$8,$4,$5
	andi	$2,$8,0x7
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L20
	lb	$2,0($4)
	.set	macro
	.set	reorder

	andi	$9,$8,0xf
	dli	$7,0x0101010101010101
	dli	$6,0x8080808080808080
	.set	noreorder
	.set	nomacro
	bne	$9,$0,$L18
	ld	$2,0($5)
	.set	macro
	.set	reorder

/* both strings are quadword aligned.  Compare a quadword at a time
   and also check for null terminator.  If equal, return 0,
   otherwise compare the latest quadwords a byte at a time. */
   
	lq	$3,0($4)
	pcpyld	$8,$7,$7 
	lq	$2,0($5)
	pcpyld	$10,$6,$6
	psubw	$7,$2,$3 
	pcpyud	$6,$7,$4
	or	$3,$6,$7
	
	.set	noreorder
	.set	nomacro
	bnel	$3,$0,$L20
	lb	$2,0($4)
	.set	macro
	.set	reorder

	lq	$2,0($4)
	pnor	$3,$0,$2
 
$L2: 
	psubb	$2,$2,$8
	pand	$2,$2,$3
	pand	$2,$2,$10
	pcpyud	$3,$2,$4
	or	$6,$3,$2
	
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L5
	addu	$4,$4,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

$L5:
	addu	$5,$5,16
	lq	$2,0($4)
	lq	$3,0($5)
	psubw	$7,$2,$3 
	pcpyud	$6,$7,$4
	or	$9,$6,$7
	
	.set	noreorder
	.set	nomacro
	beql	$9,$0,$L2
	pnor	$3,$0,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L19
	lb	$2,0($4)
	.set	macro
	.set	reorder

/* both strings are doubleword aligned.  Compare a doubleword at a time
   and also check for null terminator.  If equal, return 0,
   otherwise compare the latest doublewords a byte at a time. */
   
$L18:
	ld	$3,0($4)
	#nop
	.set	noreorder
	.set	nomacro
	bnel	$3,$2,$L20
	lb	$2,0($4)
	.set	macro
	.set	reorder

	ld	$2,0($4)
	nor	$8,$0,$2
	
$L21:
	dsubu	$2,$2,$7
	and	$2,$2,$8
	and	$2,$2,$6
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L6
	addu	$4,$4,8
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

$L6:
	addu	$5,$5,8
	ld	$2,0($4)
	ld	$3,0($5)
	#nop
	.set	noreorder
	.set	nomacro
	beql	$3,$2,$L21
	nor	$8,$0,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L19
	lb	$2,0($4)
	.set	macro
	.set	reorder
	
#else /* __OPTIMIZE_SIZE__ */

	lb	$2,0($4)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L22
	lbu	$3,0($4)
	.set	macro
	.set	reorder
	
#endif /* __OPTIMIZE_SIZE__ */

/* data is either unaligned or an inequality was found.  Compare
   a byte at a time and return difference. */
   
$L12:
	sll	$2,$3,24
	lb	$3,0($5)
	sra	$2,$2,24
	.set	noreorder
	.set	nomacro
	bnel	$2,$3,$L22
	lbu	$3,0($4)
	.set	macro
	.set	reorder

	addu	$4,$4,1
	addu	$5,$5,1
	lb	$2,0($4)
$L19:
$L20:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L12
	lbu	$3,0($4)
	.set	macro
	.set	reorder

$L22:
	lbu	$2,0($5)
	.set	noreorder
	.set	nomacro
	j	$31
	subu	$2,$3,$2
	.set	macro
	.set	reorder

	.end	strcmp
