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
# Vblank Manager Functions.
*/

	.text
	.set	noreorder


/* ################################ VBLANK STUB ####### */
/* # Added by Sjeep, 28th March 2002                  # */

	.local	vblank_stub
vblank_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000101
	.ascii	"vblank\0\0"
	.align	2

	.globl WaitVblankStart
WaitVblankStart:
 	jr      $31
	li      $0,4

	.globl WaitVblankEnd
WaitVblankEnd:
	jr      $31
	li      $0,5

	.globl WaitVblank
WaitVblank:
	jr      $31
	li      $0,6

	.globl WaitNonVblank
WaitNonVblank:
	jr      $31
	li      $0,7

	.globl RegisterVblankHandler
RegisterVblankHandler:
	jr      $31
	li      $0,8

	.globl ReleaseVblankHandler
ReleaseVblankHandler:
	jr      $31
	li      $0,9
