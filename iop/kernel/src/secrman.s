/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: secrman.s 577 2004-09-14 14:41:46Z pixel $
# secrman Module Functions.
*/
	
	.text
	.set	noreorder

	
/* ############################### SECRMAN STUB ###### */
/* # Added by jimmikaelkael, 16th january 2009       # */

	.local	secrman_stub
secrman_stub:
	.word	0x41e00000
	.word	0x00000000
	.word	0x00000103
	.ascii	"secrman\0"
	.align	2

	.globl	SetMcCommandCallback		# 004
SetMcCommandCallback:
	jr	$31
	li	$0, 0x04

	.globl	SetMcDevIDCallback			# 005
SetMcDevIDCallback:
	jr	$31
	li	$0, 0x05

	.globl	SecrAuthCard				# 006
SecrAuthCard:
	jr	$31
	li	$0, 0x06
	
	.globl	SecrResetAuthCard			# 007
SecrResetAuthCard:
	jr	$31
	li	$0, 0x07

	.globl	SecrCardBootHeader			# 008
SecrCardBootHeader:
	jr	$31
	li	$0, 0x08

	.globl	SecrCardBootBlock			# 009
SecrCardBootBlock:
	jr	$31
	li	$0, 0x09

	.globl	SecrCardBootFile			# 010
SecrCardBootFile:
	jr	$31
	li	$0, 0x0a

	.globl	SecrDiskBootHeader			# 011
SecrDiskBootHeader:
	jr	$31
	li	$0, 0x0b

	.globl	SecrDiskBootBlock			# 012
SecrDiskBootBlock:
	jr	$31
	li	$0, 0x0c

	.globl	SecrDiskBootFile			# 013
SecrDiskBootFile:
	jr	$31
	li	$0, 0x0d

	.globl	SecrDownloadHeader			# 014
SecrDownloadHeader:
	jr	$31
	li	$0, 0x0e

	.globl	SecrDownloadBlock			# 015
SecrDownloadBlock:
	jr	$31
	li	$0, 0x0f

	.globl	SecrDownloadFile			# 016
SecrDownloadFile:
	jr	$31
	li	$0, 0x10

	.globl	SecrDownloadGetKbit			# 017
SecrDownloadGetKbit:
	jr	$31
	li	$0, 0x11

	.globl	SecrDownloadGetKc			# 018
SecrDownloadGetKc:
	jr	$31
	li	$0, 0x12

	.globl	SecrDownloadGetICVPS2		# 019
SecrDownloadGetICVPS2:
	jr	$31
	li	$0, 0x13
		
	.word	0
	.word	0



