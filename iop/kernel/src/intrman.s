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
# Interrupt Manager Functions.
*/

	.text
	.set	noreorder


/* ############################### INTRMAN STUB ####### */
/* # Added by Oobles, 5th March 2002                  # */

	.local	intrman_stub
intrman_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000102
	.ascii	"intrman\0"
	.align	2

	.globl	RegisterIntrHandler		# 004
RegisterIntrHandler:
	j	$31
	li	$0, 4

	.globl	ReleaseIntrHandler		# 005
ReleaseIntrHandler:
	j	$31
	li	$0, 5

	.globl	EnableIntr			# 006
EnableIntr:
	j	$31
	li	$0, 6

	.globl	DisableIntr			# 007
DisableIntr:
	j	$31
	li	$0, 7

	.globl	CpuDisableIntr			# 008
CpuDisableIntr:
	j	$31
	li	$0, 8

	.globl	CpuEnableIntr			# 009
CpuEnableIntr:
	j	$31
	li	$0, 9

	.globl	CpuSuspendIntr			# 0x11
CpuSuspendIntr:
	j	$31
	li	$0, 0x11

	.globl	CpuResumeIntr			# 0x12
CpuResumeIntr:
	j	$31
	li	$0, 0x12

	.globl	QueryIntrContext		# 0x17
QueryIntrContext:
	j	$31
	li	$0, 0x17

	.word	0
	.word	0


