/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

// Will be executed in user mode, but with interrupts disabled.
// Set stack address to 0x00081fc0. This is meant to be the replacement for the built-in dispatcher,
// which also shares the same stack.

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl UModeCallbackDispatcher" "\n"
	"\t" ".ent UModeCallbackDispatcher" "\n"
	"\t" "UModeCallbackDispatcher:" "\n"
	"\t" "\t" "lui $sp, 0x0008" "\n"
	"\t" "\t" "jalr $v1" "\n"
	"\t" "\t" "addiu $sp, $sp, 0x1fc0" "\n"
	"\t" "\t" "addiu $v1, $zero, -8" "\n"
	"\t" "\t" "syscall" "\n"
	"\t" ".end UModeCallbackDispatcher" "\n"

	"\t" ".set pop" "\n"
);
