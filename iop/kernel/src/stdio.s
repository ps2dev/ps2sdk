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
# Stdio Import Library
*/

	.text
	.set	noreorder


/* ############################### STDIO STUB ######## */
	.local	stdio_stub
stdio_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000102
	.ascii	"stdio\0\0\0"
	.align	2

	.globl	printf				# 004

printf:
	j	$31
	li	$0, 4

	.word	0
	.word	0


