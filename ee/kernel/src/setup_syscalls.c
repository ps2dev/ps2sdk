/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl setup" "\n"
	"\t" ".ent setup" "\n"
	"\t" "setup:" "\n"
	"\t" "\t" "li $v1, 0x74" "\n"
	"\t" "\t" "syscall" "\n"
	"\t" "\t" "jr $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end setup" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl Copy" "\n"
	"\t" ".ent Copy" "\n"
	"\t" "Copy:" "\n"
	"\t" "\t" "li $v1, 0x5A" "\n"
	"\t" "\t" "syscall" "\n"
	"\t" "\t" "jr $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end Copy" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".globl GetEntryAddress" "\n"
	"\t" ".ent GetEntryAddress" "\n"
	"\t" "GetEntryAddress:" "\n"
	"\t" "\t" "li $v1, 0x5B" "\n"
	"\t" "\t" "syscall" "\n"
	"\t" "\t" "jr $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end GetEntryAddress" "\n"
);
