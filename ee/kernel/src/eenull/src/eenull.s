# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

.set push
.set noreorder

#Will be executed in user mode, but with interrupts disabled.
#Set stack address to 0x00081fc0. This is meant to be the replacement for the built-in dispatcher,
#which also shares the same stack.

.globl UModeCallbackDispatcher
.ent UModeCallbackDispatcher
UModeCallbackDispatcher:
	lui $sp, 0x0008
	jalr $v1
	addiu $sp, $sp, 0x1fc0
	addiu $v1, $zero, -8
	syscall
.end UModeCallbackDispatcher
.p2align 4

.set pop
