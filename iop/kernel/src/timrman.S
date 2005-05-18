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
# Serial Input Output 2 Manager Functions.
*/

#define DECLARE_EXPORT(name, n) \
	.globl  name; \
name: \
	j	$31; \
	li	$0, n

	.text
	.set	noreorder


/* ############################## TIMRMAN STUB ######## */
/* # Added by linuzappz, 18th May 2005                # */

	.local	timrman_stub
timrman_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000102
	.ascii	"timrman\0"
	.align	2

	DECLARE_EXPORT(AllocHardTimer, 4)
	DECLARE_EXPORT(ReferHardTimer, 5)
	DECLARE_EXPORT(FreeHardTimer, 6)
	DECLARE_EXPORT(SetTimerMode, 7)
	DECLARE_EXPORT(GetTimerStatus, 8)
	DECLARE_EXPORT(SetTimerCounter, 9)
	DECLARE_EXPORT(GetTimerCounter, 10)
	DECLARE_EXPORT(SetTimerCompare, 11)
	DECLARE_EXPORT(GetTimerCompare, 12)
	DECLARE_EXPORT(SetHoldMode, 13)
	DECLARE_EXPORT(GetHoldMode, 14)
	DECLARE_EXPORT(GetHoldReg, 15)
	DECLARE_EXPORT(GetHardTimerIntrCode, 16)

	.word	0
	.word	0

