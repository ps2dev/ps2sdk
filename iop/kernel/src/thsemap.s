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
#
# Semaphore Function Imports.
*/

	.text
	.set	noreorder


/* ############################### THSEMAP STUB ####### */
/* # Added by Oobles, 5th March 2002                  # */

	.local	thsemap_stub
thsemap_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000101
	.ascii	"thsemap\0"
	.align	2

	.globl	CreateSema			# 004
CreateSema:
	j	$31
	li	$0, 4

	.globl	DeleteSema			# 005
DeleteSema:
	j	$31
	li	$0, 5

	.globl	SignalSema			# 006
SignalSema:
	j	$31
	li	$0, 6

	.globl	iSignalSema			# 007
iSignalSema:
	j	$31
	li	$0, 7

	.globl	WaitSema			# 008
WaitSema:
	j	$31
	li	$0, 8

	.globl	PollSema			# 009
PollSema:
	j	$31
	li	$0, 0x09

	.globl	ReferSemaStatus			# 00B
ReferSemaStatus:
	j	$31
	li	$0, 0x0B

	.globl	iReferSemaStatus		# 00C
iReferSemaStatus:
	j	$31
	li	$0, 0x0C

	.word	0
	.word	0


