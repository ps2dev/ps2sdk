/*
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2002, David Ryan (Oobles@hotmail.com)
  ------------------------------------------------------------------------
  iop_sifcmd.s		   Serial Interface Command Functions.
			   taken from .irx files with symbol table 
*/

	.text
	.set	noreorder


/* ############################### SIFCMD STUB ######## */
/* # Added by Oobles, 5th March 2002                  # */

	.local	sifcmd_stub
sifcmd_stub:
	.word	0x41e00000
	.word	0
	.word	0x00000101
	.ascii	"sifcmd\0\0"
	.align	2

	.globl  sceSifInitCmd			# 0x04
sceSifInitCmd:
	j	$31
	li	$0, 0x04

	.globl	sceSifExitCmd			# 0x05
sceSifExitCmd:
	j	$31
	li	$0, 0x05

	.globl	sceSifGetSreg			# 0x06
sceSifGetSreg:
	j	$31
	li	$0, 0x06

	.globl	sceSifSetSreg			# 0x07
sceSifSetSreg:
	j	$31
	li	$0, 0x07

	.globl	sceSifSetCmdBuffer			# 0x08
sceSifSetCmdBuffer:
	j	$31
	li	$0, 0x08


	.globl	sceSifAddCmdHandler		# 0x0a
sceSifAddCmdHandler:
	j	$31
	li	$0, 0x0a

	.globl	sceSifRemoveCmdHandler		# 0x0b
sceSifRemoveCmdHandler:
	j	$31
	li	$0, 0x0b

	.globl  sceSifSendCmd			# 0x0c
sceSifSendCmd:
	j	$31
	li	$0, 0x0c

	.globl	I_sceSifSendCmd			# 0x0d
I_sceSifSendCmd:
	j	$31
	li	$0, 0x0d

	.globl	sceSifInitRpc			# 0x0E
sceSifInitRpc:
	j	$31
	li	$0, 0x0E

	.globl	sceSifBindRpc			# 0x0F
sceSifBindRpc:
	j	$31
	li	$0, 0x0F

	.globl	sceSifCallRpc			# 0x10
sceSifCallRpc:
	j	$31
	li	$0, 0x10

	.globl	sceSifRegisterRpc		# 0x11
sceSifRegisterRpc:
	j	$31
	li	$0, 0x11

	.globl	sceSifCheckStatRpc		# 0x12
sceSifCheckStatRpc:
	j	$31
	li	$0, 0x12

	.globl	sceSifSetRpcQueue		# 0x13
sceSifSetRpcQueue:
	j	$31
	li	$0, 0x13

	.globl	sceSifGetNextRequest		# 0x14
sceSifGetNextRequest:
	j	$31
	li	$0, 0x14

	.globl	sceSifExecRequest		# 0x15
sceSifExecRequest:
	j	$31
	li	$0, 0x15

	.globl	sceSifRpcLoop			# 0x16
sceSifRpcLoop:
	j	$31
	li	$0, 0x16

	.globl	sceSifRpcGetOtherData		# 0x17
sceSifRpcGetOtherData:
	j	$31
	li	$0, 0x17

	.globl	sceSifRemoveRpc			# 0x18
sceSifRemoveRpc:
	j	$31
	li	$0, 0x18

	.globl	sceSifRemoveRpcQueue		# 0x19
sceSifRemoveRpcQueue:
	j	$31
	li	$0, 0x19

	.globl	sceSifSendCmdIntr		# 0x20
sceSifSendCmdIntr:
	j	$31
	li	$0, 0x20

	.globl	I_sceSifSendCmdIntr		# 0x21
I_sceSifSendCmdIntr:
	j	$31
	li	$0, 0x21

	.word	0
	.word	0


