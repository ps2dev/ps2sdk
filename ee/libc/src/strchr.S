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

/* Assembler version of strchr that uses quadword instructions.
	
   Jeff Johnston, Cygnus Solutions, Feb 10/1999.

   ============================================================
   Copyright (C) 1999 by Cygnus Solutions. All rights reserved.
 
   Permission to use, copy, modify, and distribute this
   software is freely granted, provided that this notice
   is preserved.
   ============================================================  */
	
	.text
	.align	2
	.globl	strchr
	.text
	.ent	strchr
	.weak	strchr
strchr:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

#ifndef __OPTIMIZE_SIZE__
	
	andi	$2,$4,0x7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L24
	andi	$5,$5,0x00ff
	.set	macro
	.set	reorder

	dsll	$3,$5,8
	dli	$6,0x0101010101010101
	daddu	$10,$3,$5
	andi	$3,$4,0xf
	dsll	$2,$10,16
	dli	$8,0x8080808080808080
	daddu	$2,$2,$10
	dsll	$10,$2,32
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L6
	daddu	$7,$2,$10
	.set	macro
	.set	reorder

/* src is quadword aligned.  Load a quadword at a time and check for either a null terminator
   or the desired character.  If either is found, check the last quadword loaded, a byte at a time,
   otherwise load the next quadword and check again. */
	
$L16:	
	lq	$9,0($4)
	pcpyld	$10,$6,$6
	pnor	$3,$0,$9
	psubb	$2,$9,$10
	pcpyld	$6,$8,$8
	pand	$2,$2,$3
	pcpyld	$8,$7,$7
	pand	$2,$2,$6
	pcpyud	$3,$2,$7
	or	$3,$2,$3
	.set	noreorder
	.set	nomacro
	bnel	$3,$0,$L25
	lbu	$2,0($4)
	.set	macro
	.set	reorder

	pxor	$2,$9,$8
	psubb	$3,$2,$10
	pnor	$2,$0,$2
	dli	$8,0x8080808080808080
	pand	$3,$3,$2
	pand	$3,$3,$6
	dli	$6,0x0101010101010101
	pcpyud	$2,$3,$5
	or	$3,$2,$3
	.set	noreorder
	.set	nomacro
	beql	$3,$0,$L16
	addu	$4,$4,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	b	$L25
	lbu	$2,0($4)
	.set	macro
	.set	reorder

/* src is doubleword aligned.  Load a doubleword at a time and check for either a null terminator
   or the desired character.  If either is found, check the last doubleword loaded, a byte at a 
   time, otherwise continue by checking the next doubleword. */
	
$L6:
	ld	$9,0($4)
	nor	$3,$0,$9
	dsubu	$2,$9,$6
	and	$2,$2,$3
	and	$2,$2,$8
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L25
	lbu	$2,0($4)
	.set	macro
	.set	reorder

	#nop
	xor	$2,$9,$7
	dsubu	$3,$2,$6
	nor	$2,$0,$2
	and	$3,$3,$2
	and	$3,$3,$8
	.set	noreorder
	.set	nomacro
	bnel	$3,$0,$L25
	lbu	$2,0($4)
	.set	macro
	.set	reorder

	move	$9,$6
	addu	$4,$4,8
$L26:
	ld	$6,0($4)
	#nop
	dsubu	$2,$6,$9
	nor	$3,$0,$6
	and	$2,$2,$3
	and	$2,$2,$8
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L24
	xor	$2,$6,$7
	.set	macro
	.set	reorder

	nor	$3,$0,$2
	dsubu	$2,$2,$9
	and	$2,$2,$3
	and	$2,$2,$8
	
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L26
	addu	$4,$4,8
	.set	macro
	.set	reorder

#else /* __OPTIMIZE_SIZE__ */
	
	andi	$5,$5,0x00ff
	
#endif /* __OPTIMIZE_SIZE__ */

	.set	noreorder
	.set	nomacro
	b	$L25
	lbu	$2,0($4)
	.set	macro
	.set	reorder

/* check a byte at a time until the desired character is found in which case return its
   address or else the null terminator is found in which case return null. */
		
$L17:
	#nop
	.set	noreorder
	.set	nomacro
	beql	$2,$5,$L27
	lbu	$3,0($4)
	.set	macro
	.set	reorder

	addu	$4,$4,1
$L24:
	lbu	$2,0($4)
$L25:
	bne	$2,$0,$L17
	lbu	$3,0($4)
$L27:
	move	$2,$0
	xor	$3,$3,$5
	.set	noreorder
	.set	nomacro
	j	$31
	movz	$2,$4,$3
	.set	macro
	.set	reorder

	.end	strchr
