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


/* ############################## SIO2MAN STUB ######## */
/* # Added by linuzappz, 6th October 2004             # */

	.local	sio2man_stub
sio2man_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000102
	.ascii	"sio2man\0"
	.align	2

	DECLARE_EXPORT(sio2_ctrl_set, 4)
	DECLARE_EXPORT(sio2_ctrl_get, 5)
	DECLARE_EXPORT(sio2_stat6c_get, 6)
	DECLARE_EXPORT(sio2_portN_ctrl1_set, 7)
	DECLARE_EXPORT(sio2_portN_ctrl1_get, 8)
	DECLARE_EXPORT(sio2_portN_ctrl2_set, 9)
	DECLARE_EXPORT(sio2_portN_ctrl2_get, 10)
	DECLARE_EXPORT(sio2_stat70_get, 11)
	DECLARE_EXPORT(sio2_regN_set, 12)
	DECLARE_EXPORT(sio2_regN_get, 13)
	DECLARE_EXPORT(sio2_stat74_get, 14)
	DECLARE_EXPORT(sio2_unkn78_set, 15)
	DECLARE_EXPORT(sio2_unkn78_get, 16)
	DECLARE_EXPORT(sio2_unkn7c_set, 17)
	DECLARE_EXPORT(sio2_unkn7c_get, 18)
	DECLARE_EXPORT(sio2_data_out, 19)
	DECLARE_EXPORT(sio2_data_in, 20)
	DECLARE_EXPORT(sio2_stat_set, 21)
	DECLARE_EXPORT(sio2_stat_get, 22)
	DECLARE_EXPORT(sio2_pad_transfer_init, 23)
	DECLARE_EXPORT(sio2_mc_transfer_init, 24)
	DECLARE_EXPORT(sio2_transfer, 25)
	DECLARE_EXPORT(sio2_transfer_reset, 26)
	DECLARE_EXPORT(sio2_mtap_transfer_init, 48)

	.word	0
	.word	0

