/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#define GENERAL_ATSAVE        0x400
#define GENERAL_SRSAVE

__asm__
(
	"\t" "\t" ".global def_exc_handler" "\n"
	"\t" "\t" ".ent    def_exc_handler" "\n"
	"\t" "def_exc_handler:" "\n"
	"\t" "\t" ".set    push" "\n"
	"\t" "\t" ".set    noreorder" "\n"
	"\t" "\t" ".set    noat" "\n"
	"\t" "\t" ".word   0" "\n"
	"\t" "\t" ".word   0" "\n"
	"\t" "\t" "la      $k0, __trap_frame" "\n"
	"\t" "\t" "sw      $0, 0x10($k0)" "\n"

	"\t" "\t" "lw      $1, 0x400($0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $1, 0x14($k0)" "\n"

	"\t" "\t" "sw      $2, 0x18($k0)" "\n"
	"\t" "\t" "sw      $3, 0x1c($k0)" "\n"
	"\t" "\t" "sw      $4, 0x20($k0)" "\n"
	"\t" "\t" "sw      $5, 0x24($k0)" "\n"
	"\t" "\t" "sw      $6, 0x28($k0)" "\n"
	"\t" "\t" "sw      $7, 0x2c($k0)" "\n"
	"\t" "\t" "sw      $8, 0x30($k0)" "\n"
	"\t" "\t" "sw      $9, 0x34($k0)" "\n"
	"\t" "\t" "sw      $10, 0x38($k0)" "\n"
	"\t" "\t" "sw      $11, 0x3c($k0)" "\n"
	"\t" "\t" "sw      $12, 0x40($k0)" "\n"
	"\t" "\t" "sw      $13, 0x44($k0)" "\n"
	"\t" "\t" "sw      $14, 0x48($k0)" "\n"
	"\t" "\t" "sw      $15, 0x4c($k0)" "\n"
	"\t" "\t" "sw      $16, 0x50($k0)" "\n"
	"\t" "\t" "sw      $17, 0x54($k0)" "\n"
	"\t" "\t" "sw      $18, 0x58($k0)" "\n"
	"\t" "\t" "sw      $19, 0x5c($k0)" "\n"
	"\t" "\t" "sw      $20, 0x60($k0)" "\n"
	"\t" "\t" "sw      $21, 0x64($k0)" "\n"
	"\t" "\t" "sw      $22, 0x68($k0)" "\n"
	"\t" "\t" "sw      $23, 0x6c($k0)" "\n"
	"\t" "\t" "sw      $24, 0x70($k0)" "\n"
	"\t" "\t" "sw      $25, 0x74($k0)" "\n"
	"\t" "\t" "sw      $26, 0x78($k0)" "\n"
	"\t" "\t" "sw      $27, 0x7c($k0)" "\n"
	"\t" "\t" "sw      $28, 0x80($k0)" "\n"
	"\t" "\t" "sw      $29, 0x84($k0)" "\n"
	"\t" "\t" "sw      $30, 0x88($k0)" "\n"
	"\t" "\t" "sw      $31, 0x8c($k0)" "\n"
	"\t" "\t" "mfhi    $a0" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x90($k0)" "\n"

	"\t" "\t" "mflo    $a0" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x94($k0)" "\n"


	// "\t" "\t" "lw      $a0, 0x404($0)" "\n" // saved EPC
	"\t" "\t" "mfc0    $a0, $14" "\n" // EPC
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0($k0)" "\n"

	// "\t" "\t" "lw      $a0, 0x408($0)" "\n" // saved SR
	"\t" "\t" "mfc0    $a0, $12" "\n" // SR
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0xc($k0)" "\n"

	"\t" "\t" "mfc0    $a0, $8" "\n" // BadVaddr
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x8($k0)" "\n"

	"\t" "\t" "sw      $0, 0x98($k0)" "\n"


	// "\t" "\t" "lw      $a0, 0x40c($0)" "\n" // saved Cause
	"\t" "\t" "mfc0    $a0, $13" "\n" // Cause
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 4($k0)" "\n"

	"\t" "\t" "sll     $a0, 25" "\n"
	"\t" "\t" "srl     $a0, 27" "\n"
	"\t" "\t" "jal trap" "\n"
	"\t" "\t" "or $a1, $k0, $0" "\n"

	"\t" "\t" "la      $k0, __trap_frame" "\n"

	"\t" "\t" "lw      $a0, 0x94($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtlo    $a0" "\n"

	"\t" "\t" "lw      $a0, 0x90($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mthi    $a0" "\n"

	// "\t" "\t" "lw      $0, 0x10($k0)" "\n"
	"\t" "\t" "lw      $1, 0x14($k0)" "\n"
	"\t" "\t" "lw      $2, 0x18($k0)" "\n"
	"\t" "\t" "lw      $3, 0x1c($k0)" "\n"
	"\t" "\t" "lw      $4, 0x20($k0)" "\n"
	"\t" "\t" "lw      $5, 0x24($k0)" "\n"
	"\t" "\t" "lw      $6, 0x28($k0)" "\n"
	"\t" "\t" "lw      $7, 0x2c($k0)" "\n"
	"\t" "\t" "lw      $8, 0x30($k0)" "\n"
	"\t" "\t" "lw      $9, 0x34($k0)" "\n"
	"\t" "\t" "lw      $10, 0x38($k0)" "\n"
	"\t" "\t" "lw      $11, 0x3c($k0)" "\n"
	"\t" "\t" "lw      $12, 0x40($k0)" "\n"
	"\t" "\t" "lw      $13, 0x44($k0)" "\n"
	"\t" "\t" "lw      $14, 0x48($k0)" "\n"
	"\t" "\t" "lw      $15, 0x4c($k0)" "\n"
	"\t" "\t" "lw      $16, 0x50($k0)" "\n"
	"\t" "\t" "lw      $17, 0x54($k0)" "\n"
	"\t" "\t" "lw      $18, 0x58($k0)" "\n"
	"\t" "\t" "lw      $19, 0x5c($k0)" "\n"
	"\t" "\t" "lw      $20, 0x60($k0)" "\n"
	"\t" "\t" "lw      $21, 0x64($k0)" "\n"
	"\t" "\t" "lw      $22, 0x68($k0)" "\n"
	"\t" "\t" "lw      $23, 0x6c($k0)" "\n"
	"\t" "\t" "lw      $24, 0x70($k0)" "\n"
	"\t" "\t" "lw      $25, 0x74($k0)" "\n"
	// "\t" "\t" "lw      $26, 0x78($k0)" "\n"
	// "\t" "\t" "lw      $27, 0x7c($k0)" "\n"
	"\t" "\t" "lw      $28, 0x80($k0)" "\n"
	"\t" "\t" "lw      $29, 0x84($k0)" "\n"
	"\t" "\t" "lw      $30, 0x88($k0)" "\n"
	"\t" "\t" "lw      $31, 0x8c($k0)" "\n"

	"\t" "\t" "lw      $k1, 0xc($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "srl     $k1, 1" "\n"
	"\t" "\t" "sll     $k1, 1" "\n"
	"\t" "\t" "mtc0    $k1, $12" "\n" // restore SR

	"\t" "\t" "lw      $k1, 0($k0)" "\n"
	"\t" "\t" "lw      $k0, 0x410($0)" "\n" // k0 saved here by exception prologue (at 0x80)
	"\t" "\t" "jr      $k1" "\n"
	"\t" "\t" "cop0    0x10" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" ".set pop" "\n"
	"\t" "\t" ".end def_exc_handler" "\n"

	"\t" "\t" ".global bp_exc_handler" "\n"
	"\t" "\t" ".ent bp_exc_handler" "\n"
	"\t" "bp_exc_handler:" "\n"
	"\t" "\t" ".set push" "\n"
	"\t" "\t" ".set noreorder" "\n"
	"\t" "\t" ".set noat" "\n"
	"\t" "\t" ".word 0" "\n"
	"\t" "\t" ".word 0" "\n"
	"\t" "\t" "la      $k0, __trap_frame" "\n"
	"\t" "\t" "sw      $0, 0x10($k0)" "\n"
	"\t" "\t" "sw      $1, 0x14($k0)" "\n"
	"\t" "\t" "sw      $2, 0x18($k0)" "\n"
	"\t" "\t" "sw      $3, 0x1c($k0)" "\n"
	"\t" "\t" "sw      $4, 0x20($k0)" "\n"
	"\t" "\t" "sw      $5, 0x24($k0)" "\n"
	"\t" "\t" "sw      $6, 0x28($k0)" "\n"
	"\t" "\t" "sw      $7, 0x2c($k0)" "\n"
	"\t" "\t" "sw      $8, 0x30($k0)" "\n"
	"\t" "\t" "sw      $9, 0x34($k0)" "\n"
	"\t" "\t" "sw      $10, 0x38($k0)" "\n"
	"\t" "\t" "sw      $11, 0x3c($k0)" "\n"
	"\t" "\t" "sw      $12, 0x40($k0)" "\n"
	"\t" "\t" "sw      $13, 0x44($k0)" "\n"
	"\t" "\t" "sw      $14, 0x48($k0)" "\n"
	"\t" "\t" "sw      $15, 0x4c($k0)" "\n"
	"\t" "\t" "sw      $16, 0x50($k0)" "\n"
	"\t" "\t" "sw      $17, 0x54($k0)" "\n"
	"\t" "\t" "sw      $18, 0x58($k0)" "\n"
	"\t" "\t" "sw      $19, 0x5c($k0)" "\n"
	"\t" "\t" "sw      $20, 0x60($k0)" "\n"
	"\t" "\t" "sw      $21, 0x64($k0)" "\n"
	"\t" "\t" "sw      $22, 0x68($k0)" "\n"
	"\t" "\t" "sw      $23, 0x6c($k0)" "\n"
	"\t" "\t" "sw      $24, 0x70($k0)" "\n"
	"\t" "\t" "sw      $25, 0x74($k0)" "\n"
	"\t" "\t" "sw      $26, 0x78($k0)" "\n"
	"\t" "\t" "sw      $27, 0x7c($k0)" "\n"
	"\t" "\t" "sw      $28, 0x80($k0)" "\n"
	"\t" "\t" "sw      $29, 0x84($k0)" "\n"
	"\t" "\t" "sw      $30, 0x88($k0)" "\n"
	"\t" "\t" "sw      $31, 0x8c($k0)" "\n"
	"\t" "\t" "mfhi    $a0" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x90($k0)" "\n"

	"\t" "\t" "mflo    $a0" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x94($k0)" "\n"


	// "\t" "\t" "lw      $a0, 0x424($0)" "\n" // saved EPC
	"\t" "\t" "mfc0    $a0, $14" "\n" // EPC
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0($k0)" "\n"

	// "\t" "\t" "lw      $a0, 0x42c($0)" "\n" // saved SR
	"\t" "\t" "mfc0    $a0, $12" "\n" // SR
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0xc($k0)" "\n"

	"\t" "\t" "mfc0    $a0, $8" "\n" // BadVaddr
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x8($k0)" "\n"

	"\t" "\t" "lw      $a0, 0x430($0)" "\n" // saved DCIC
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 0x98($k0)" "\n"


	// "\t" "\t" "lw      $a0, 0x428($0)" "\n" // saved Cause
	"\t" "\t" "mfc0    $a0, $13" "\n" // Cause
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sw      $a0, 4($k0)" "\n"

	"\t" "\t" "sll     $a0, 25" "\n"
	"\t" "\t" "srl     $a0, 27" "\n"
	"\t" "\t" "jal     trap" "\n"
	"\t" "\t" "or      $a1, $k0, $0" "\n"


	"\t" "\t" "la      $k0, __trap_frame" "\n"

	"\t" "\t" "lw      $a0, 0x94($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtlo    $a0" "\n"

	"\t" "\t" "lw      $a0, 0x90($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mthi    $a0" "\n"

	"\t" "\t" "lw      $a0, 0x98($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $a0, $7" "\n"

	// "\t" "\t" "lw      $0, 0x10($k0)" "\n"
	"\t" "\t" "lw      $1, 0x14($k0)" "\n"
	"\t" "\t" "lw      $2, 0x18($k0)" "\n"
	"\t" "\t" "lw      $3, 0x1c($k0)" "\n"
	"\t" "\t" "lw      $4, 0x20($k0)" "\n"
	"\t" "\t" "lw      $5, 0x24($k0)" "\n"
	"\t" "\t" "lw      $6, 0x28($k0)" "\n"
	"\t" "\t" "lw      $7, 0x2c($k0)" "\n"
	"\t" "\t" "lw      $8, 0x30($k0)" "\n"
	"\t" "\t" "lw      $9, 0x34($k0)" "\n"
	"\t" "\t" "lw      $10, 0x38($k0)" "\n"
	"\t" "\t" "lw      $11, 0x3c($k0)" "\n"
	"\t" "\t" "lw      $12, 0x40($k0)" "\n"
	"\t" "\t" "lw      $13, 0x44($k0)" "\n"
	"\t" "\t" "lw      $14, 0x48($k0)" "\n"
	"\t" "\t" "lw      $15, 0x4c($k0)" "\n"
	"\t" "\t" "lw      $16, 0x50($k0)" "\n"
	"\t" "\t" "lw      $17, 0x54($k0)" "\n"
	"\t" "\t" "lw      $18, 0x58($k0)" "\n"
	"\t" "\t" "lw      $19, 0x5c($k0)" "\n"
	"\t" "\t" "lw      $20, 0x60($k0)" "\n"
	"\t" "\t" "lw      $21, 0x64($k0)" "\n"
	"\t" "\t" "lw      $22, 0x68($k0)" "\n"
	"\t" "\t" "lw      $23, 0x6c($k0)" "\n"
	"\t" "\t" "lw      $24, 0x70($k0)" "\n"
	"\t" "\t" "lw      $25, 0x74($k0)" "\n"
	// "\t" "\t" "lw      $26, 0x78($k0)" "\n"
	// "\t" "\t" "lw      $27, 0x7c($k0)" "\n"
	"\t" "\t" "lw      $28, 0x80($k0)" "\n"
	"\t" "\t" "lw      $29, 0x84($k0)" "\n"
	"\t" "\t" "lw      $30, 0x88($k0)" "\n"
	"\t" "\t" "lw      $31, 0x8c($k0)" "\n"

	"\t" "\t" "lw      $k1, 0xc($k0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "srl     $k1, 1" "\n"
	"\t" "\t" "sll     $k1, 1" "\n"
	"\t" "\t" "mtc0    $k1, $12" "\n" // restore SR

	"\t" "\t" "lw      $k1, 0($k0)" "\n"
	"\t" "\t" "lw      $k0, 0x420($0)" "\n" // k0 saved here by debug exception prologue (at 0x40)
	"\t" "\t" "jr      $k1" "\n"
	"\t" "\t" "cop0    0x10" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" ".set    pop" "\n"
	"\t" "\t" ".end    bp_exc_handler" "\n"
);
