/*
#
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Hardware breakpoint functions
*/

#define ABI_EABI64 // force all register names to EABI64 (legacy toolchain)
#include "as_reg_compat.h"

#define STRINNER(x) #x
#define STR(x) STRINNER(x)

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	"\t" "\t" ".global InitBPC" "\n"
	"\t" "\t" ".ent	InitBPC" "\n"

	"\t" "InitBPC:" "\n"
	"\t" "\t" "sync.p" "\n"

	// Disable the exception handling
	"\t" "\t" "li	$" STR(t0) ", (1 << 15)" "\n"
	"\t" "\t" "mtbpc	$" STR(t0) "" "\n"

	// Clear out the registers
	"\t" "\t" "mtiab	$0" "\n"
	"\t" "\t" "li	$" STR(t0) ", 0xffffffff" "\n"
	"\t" "\t" "mtiabm	$" STR(t0) "" "\n"
	"\t" "\t" "mtdab	$0" "\n"
	"\t" "\t" "mtdabm	$" STR(t0) "" "\n"
	"\t" "\t" "mtdvb	$0" "\n"
	"\t" "\t" "mtdvbm	$" STR(t0) "" "\n"

	"\t" "\t" "jr	$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end 	InitBPC" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	"\t" "    .global SetInstructionBP" "\n"
	"\t" "    .ent	SetInstructionBP" "\n"

	// Set the instruction breakpoint registers
	// a0 = PC location
	// a1 = PC Mask
	// a2 = options
	"\t" "SetInstructionBP:" "\n"

	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "mfbpc	$" STR(t0) "" "\n"
	// Jump if top bit not set
	"\t" "\t" "bgez	$" STR(t0) ", 1f" "\n"
	"\t" "\t" "nop" "\n"

	// Clear top bit
	"\t" "\t" "li	$" STR(t1) ", (1 << 31)" "\n"
	"\t" "\t" "xor	$" STR(t0) ", $" STR(t0) ", $" STR(t1) "" "\n"
	"\t" "\t" "mtbpc	$" STR(t0) "" "\n"

	"\t" "\t" "sync.p" "\n"
	"\t" "1:" "\n"
	"\t" "\t" "mtiab	$a0" "\n"
	"\t" "\t" "mtiabm	$a1" "\n"
	"\t" "\t" "mfbpc	$" STR(t0) "" "\n"
	// Clear flags relating to IAB
	"\t" "\t" "li	$" STR(t1) ", ~((1 << 26) | (1 << 25) | (1 << 24) | (1 << 23) | (1 << 17) | (1 << 15) | 1)" "\n"
	"\t" "\t" "and	$" STR(t0) ", $" STR(t0) ", $" STR(t1) "" "\n"

	// Set flags relating to IAB, only enable for user and supervisor modes
	// "\t" "\t" "li  " STR(t1) ", " (BPC_IAE | BPC_IUE | BPC_ISE)

	// Clean up options to ensure we are only setting instruction stuff
	"\t" "\t" "li  $" STR(t1) ", ((1 << 26) | (1 << 25) | (1 << 24) | (1 << 23))" "\n"
	"\t" "\t" "and $a2, $a2, $" STR(t1) "" "\n"

	// Or in options
	"\t" "\t" "li	$" STR(t1) ", (1 << 31)" "\n"
	"\t" "\t" "or	$" STR(t1) ", $a2, $" STR(t1) "" "\n"

	// Or in original bpc
	"\t" "\t" "or	$" STR(t0) ", $" STR(t0) ", $" STR(t1) "" "\n"
	"\t" "\t" "mtbpc	$" STR(t0) "" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "jr	$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end	SetInstructionBP" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set a data address breakpoint
	// a0 - Data address
	// a1 - Data address mask
	// a2 - options (e.g. BPC_DUE)
	"\t" "\t" ".global SetDataAddrBP" "\n"
	"\t" "\t" ".ent	SetDataAddrBP" "\n"
	"\t" "SetDataAddrBP:" "\n"
	"\t" "\t" "sync.l" "\n"

	// Mask out any data address related bits
	"\t" "\t" "mfbpc	$" STR(t0) "" "\n"
	"\t" "\t" "li	$" STR(t1) ", ~((1 << 30) | (1 << 29) | (1 << 28) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18) | (1 << 16) | (1 << 15) | (1 << 2) | (1 << 1))" "\n"

	"\t" "\t" "and	$" STR(t0) ", $" STR(t0) ", $" STR(t1) "" "\n"
	"\t" "\t" "mtbpc	$" STR(t0) "" "\n"
	"\t" "\t" "sync.p" "\n"

	// Move in address and mask
	"\t" "\t" "mtdab	$a0" "\n"
	"\t" "\t" "mtdabm	$a1" "\n"

	// Mask out all non data address bits
	"\t" "\t" "li		$" STR(t1) ", ((1 << 30) | (1 << 29) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18))" "\n"
	"\t" "\t" "and 	$" STR(t1) ", $a2, $" STR(t1) "" "\n"

	"\t" "\t" "mfbpc	$" STR(t0) "" "\n"
	"\t" "\t" "or		$" STR(t0) ", $" STR(t0) ", $" STR(t1) "" "\n"

	"\t" "\t" "mtbpc	$" STR(t0) "" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "jr	$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end	SetDataAddrBP" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set a data value breakpoint
	// a0 - Data address
	// a1 - Data address mask
	// a2 - Data value
	// a3 - Data value mask
	// " STR(t0) " - options (e.g. BPC_DUE)
	"\t" "\t" ".global SetDataValueBP" "\n"
	"\t" "\t" ".ent	SetDataValueBP" "\n"
	"\t" "SetDataValueBP:" "\n"
	"\t" "\t" "sync.l" "\n"

	// Mask out any data address related bits
	"\t" "\t" "mfbpc	$" STR(t1) "" "\n"
	"\t" "\t" "li	$" STR(t2) ", ~((1 << 30) | (1 << 29) | (1 << 28) | (1 << 21) |   (1 << 20) | (1 << 19) | (1 << 18) | (1 << 16) | (1 << 15) | (1 << 2) | (1 << 1))" "\n"

	"\t" "\t" "and	$" STR(t1) ", $" STR(t1) ", $" STR(t2) "" "\n"
	"\t" "\t" "mtbpc	$" STR(t1) "" "\n"
	"\t" "\t" "sync.p" "\n"

	// Move in address and mask
	"\t" "\t" "mtdab	$a0" "\n"
	"\t" "\t" "mtdabm	$a1" "\n"
	"\t" "\t" "mtdvb   $a2" "\n"
	"\t" "\t" "mtdvbm  $a3" "\n"

	// Mask out all non data address bits
	"\t" "\t" "li		$" STR(t2) ", ((1 << 30) | (1 << 29) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18))" "\n"
	"\t" "\t" "and 	$" STR(t2) ", $" STR(t0) ", $" STR(t2) "" "\n"
	"\t" "\t" "li		$" STR(t0) ", (1 << 28)" "\n" // Data value enable
	"\t" "\t" "or		$" STR(t2) ", $" STR(t2) ", $" STR(t0) "" "\n"

	"\t" "\t" "mfbpc	$" STR(t1) "" "\n"
	"\t" "\t" "or		$" STR(t1) ", $" STR(t1) ", $" STR(t2) "" "\n"

	"\t" "\t" "mtbpc	$" STR(t1) "" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "jr	$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end	SetDataValueBP" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the BPC
	"\t" "\t" ".global GetBPC" "\n"
	"\t" "\t" ".ent	GetBPC" "\n"
	"\t" "GetBPC:" "\n"
	"\t" "\t" "sync.l" "\n"
	"\t" "\t" "mfbpc	$v0" "\n"
	"\t" "\t" "jr	$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end 	GetBPC" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the BPC
	"\t" "\t" ".global SetBPC" "\n"
	"\t" "\t" ".ent	SetBPC" "\n"
	"\t" "SetBPC:" "\n"
	"\t" "\t" "mtbpc	$a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetBPC" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the Instruction address register
	"\t" "\t" ".global GetIAB" "\n"
	"\t" "\t" ".ent GetIAB" "\n"
	"\t" "GetIAB:" "\n"
	"\t" "\t" "mfiab	$v0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end GetIAB" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the Instruction address mask register
	"\t" "\t" ".global GetIABM" "\n"
	"\t" "\t" ".ent GetIABM" "\n"
	"\t" "GetIABM:" "\n"
	"\t" "\t" "mfiabm	$v0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end GetIABM" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the data address register
	"\t" "\t" ".global GetDAB" "\n"
	"\t" "\t" ".ent GetDAB" "\n"
	"\t" "GetDAB:" "\n"
	"\t" "\t" "mfdab	$v0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end GetDAB" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the data address mask register
	"\t" "\t" ".global GetDABM" "\n"
	"\t" "\t" ".ent GetDABM" "\n"
	"\t" "GetDABM:" "\n"
	"\t" "\t" "mfdabm	$v0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end GetDABM" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the data value register
	"\t" "\t" ".global GetDVB" "\n"
	"\t" "\t" ".ent GetDVB" "\n"
	"\t" "GetDVB:" "\n"
	"\t" "\t" "mfdvb	$v0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end GetDVB" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Get the current value of the data value mask register
	"\t" "\t" ".global GetDVBM" "\n"
	"\t" "\t" ".ent GetDVBM" "\n"
	"\t" "GetDVBM:" "\n"
	"\t" "\t" "mfdvbm	$v0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end GetDVBM" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the Instruction address register
	"\t" "\t" ".global SetIAB" "\n"
	"\t" "\t" ".ent SetIAB" "\n"
	"\t" "SetIAB:" "\n"
	"\t" "\t" "mtiab	$a0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetIAB" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the Instruction address mask register
	"\t" "\t" ".global SetIABM" "\n"
	"\t" "\t" ".ent SetIABM" "\n"
	"\t" "SetIABM:" "\n"
	"\t" "\t" "mtiabm	$a0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetIABM" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the data address register
	"\t" "\t" ".global SetDAB" "\n"
	"\t" "\t" ".ent SetDAB" "\n"
	"\t" "SetDAB:" "\n"
	"\t" "\t" "mtdab	$a0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetDAB" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the data address mask register
	"\t" "\t" ".global SetDABM" "\n"
	"\t" "\t" ".ent SetDABM" "\n"
	"\t" "SetDABM:" "\n"
	"\t" "\t" "mtdabm	$a0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetDABM" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the data value register
	"\t" "\t" ".global SetDVB" "\n"
	"\t" "\t" ".ent SetDVB" "\n"
	"\t" "SetDVB:" "\n"
	"\t" "\t" "mtdvb	$a0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetDVB" "\n"

	"\t" "\t" ".set pop" "\n"
);

__asm__
(
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"

	// Set the current value of the data value mask register
	"\t" "\t" ".global SetDVBM" "\n"
	"\t" "\t" ".ent SetDVBM" "\n"
	"\t" "SetDVBM:" "\n"
	"\t" "\t" "mtdvbm	$a0" "\n"
	"\t" "\t" "jr		$ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" ".end SetDVBM" "\n"

	"\t" "\t" ".set pop" "\n"
);
