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
# Serial Interface Manager Functions.
*/

	.text
	.set	noreorder


/* ############################### SIFMAN STUB ######## */
/* # Added by Oobles, 7th March 2002                  # */

	.local	sifman_stub
sifman_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000101
	.ascii	"sifman\0\0"
	.align	2

	.globl  sceSifInit				# 0x05
sceSifInit:
	j	$31
	li	$0, 0x05

	.globl	sceSifSetDChain			# 0x06
sceSifSetDChain:
	j	$31
	li	$0, 0x06

	.globl	sceSifSetDma			# 0x07
sceSifSetDma:
	j	$31
	li	$0, 0x07

	.globl	sceSifDmaStat			# 0x08
sceSifDmaStat:
	j	$31
	li	$0, 0x08
	
	.globl	sceSifGetSMFlag			# 0x17
sceSifGetSMFlag:
	j	$31
	li	$0, 0x17
	
	.globl	sceSifSetSMFlag			# 0x18
sceSifSetSMFlag:
	j	$31
	li	$0, 0x18

	.globl	sceSifIntrMain			# 0x1c
sceSifIntrMain:
	j	$31
	li	$0, 0x1c

	.globl	sceSifCheckInit			# 0x1d
sceSifCheckInit:
	j	$31
	li	$0, 0x1d

	.globl	sceSifSetDmaIntr		# 0x20
sceSifSetDmaIntr:
	j	$31
	li	$0, 0x20

	.word	0
	.word	0
