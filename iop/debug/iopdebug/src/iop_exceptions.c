/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOPDEBUG - IOP debugging library.
 * low-level IOP exception-handler code.
 */

#include <iop_cop0_defs.h>
#include "iopdebug_defs.h"

#define STRINNER(x) #x
#define STR(x) STRINNER(x)

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global _iop_save_frame" "\n"
	"\t" ".ent _iop_save_frame" "\n"
	"\t" "_iop_save_frame:" "\n"
	"\t" "\t" "sw      $zero, 0x000($s0)" "\n"

	"\t" "\t" "sw      $at, 0x004($s0)" "\n"

	"\t" "\t" "sw      $v0, 0x008($s0)" "\n"
	"\t" "\t" "sw      $v1, 0x00C($s0)" "\n"

	"\t" "\t" "sw      $a0, 0x010($s0)" "\n"
	"\t" "\t" "sw      $a1, 0x014($s0)" "\n"
	"\t" "\t" "sw      $a2, 0x018($s0)" "\n"
	"\t" "\t" "sw      $a3, 0x01C($s0)" "\n"

	"\t" "\t" "sw      $t0, 0x020($s0)" "\n"
	"\t" "\t" "sw      $t1, 0x024($s0)" "\n"
	"\t" "\t" "sw      $t2, 0x028($s0)" "\n"
	"\t" "\t" "sw      $t3, 0x02C($s0)" "\n"
	"\t" "\t" "sw      $t4, 0x030($s0)" "\n"
	"\t" "\t" "sw      $t5, 0x034($s0)" "\n"
	"\t" "\t" "sw      $t6, 0x038($s0)" "\n"
	"\t" "\t" "sw      $t7, 0x03C($s0)" "\n"

	// "\t" "\t" "sw      $s0, 0x040($s0)" "\n"
	"\t" "\t" "sw      $s1, 0x044($s0)" "\n"
	"\t" "\t" "sw      $s2, 0x048($s0)" "\n"
	"\t" "\t" "sw      $s3, 0x04C($s0)" "\n"
	"\t" "\t" "sw      $s4, 0x050($s0)" "\n"
	"\t" "\t" "sw      $s5, 0x054($s0)" "\n"
	"\t" "\t" "sw      $s6, 0x058($s0)" "\n"
	"\t" "\t" "sw      $s7, 0x05C($s0)" "\n"

	"\t" "\t" "sw      $t8, 0x060($s0)" "\n"
	"\t" "\t" "sw      $t9, 0x064($s0)" "\n"

	// "\t" "\t" "sw      $k0, 0x068($s0)" "\n"
	"\t" "\t" "sw      $k1, 0x06C($s0)" "\n"

	"\t" "\t" "sw      $gp, 0x070($s0)" "\n"
	// "\t" "\t" "sw      $sp, 0x074($s0)" "\n"
	"\t" "\t" "sw      $fp, 0x078($s0)" "\n"
	// "\t" "\t" "sw      $ra, 0x07C($s0)" "\n"

	"\t" "\t" "mfhi    $t0" "\n"  // hi
	"\t" "\t" "mflo    $t1" "\n"  // lo
	"\t" "\t" "nop" "\n"  // necessary??
	"\t" "\t" "sw      $t0, 0x080($s0)" "\n"
	"\t" "\t" "sw      $t1, 0x084($s0)" "\n"
	"\t" "\t" "nop" "\n"  // necessary??

	"\t" "\t" "mfc0    $t0, $3" "\n"  // IOP_COP0_BPC
	"\t" "\t" "mfc0    $t1, $5" "\n"  // IOP_COP0_BDA
	"\t" "\t" "mfc0    $t2, $8" "\n"  // IOP_COP0_BADVADDR
	"\t" "\t" "mfc0    $t3, $9" "\n"  // IOP_COP0_BDAM
	"\t" "\t" "mfc0    $t4, $11" "\n"  // IOP_COP0_BPCM
	"\t" "\t" "mfc0    $t5, $12" "\n"  // SR
	"\t" "\t" "mfc0    $t6, $13" "\n"  // Cause
	"\t" "\t" "mfc0    $t7, $14" "\n"  // EPC

	"\t" "\t" "sw      $t0, 0x088($s0)" "\n"
	"\t" "\t" "sw      $t1, 0x08C($s0)" "\n"
	// gap for IOP_COP0_DCIC
	"\t" "\t" "sw      $t2, 0x094($s0)" "\n"
	"\t" "\t" "sw      $t3, 0x098($s0)" "\n"
	"\t" "\t" "sw      $t4, 0x09C($s0)" "\n"
	"\t" "\t" "sw      $t5, 0x0A0($s0)" "\n"
	"\t" "\t" "sw      $t6, 0x0A4($s0)" "\n"
	"\t" "\t" "sw      $t7, 0x0A8($s0)" "\n"

	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end _iop_save_frame" "\n"

	"\t" ".global _iop_load_frame" "\n"
	"\t" ".ent _iop_load_frame" "\n"
	"\t" "_iop_load_frame:" "\n"
	// "\t" "\t" "lw      $zero, 0x000($s0)" "\n"

	"\t" "\t" "lw      $at, 0x004($s0)" "\n"

	"\t" "\t" "lw      $v0, 0x008($s0)" "\n"
	"\t" "\t" "lw      $v1, 0x00C($s0)" "\n"

	"\t" "\t" "lw      $a0, 0x010($s0)" "\n"
	"\t" "\t" "lw      $a1, 0x014($s0)" "\n"
	"\t" "\t" "lw      $a2, 0x018($s0)" "\n"
	"\t" "\t" "lw      $a3, 0x01C($s0)" "\n"

	// "\t" "\t" "lw      $s0, 0x040($s0)" "\n"
	"\t" "\t" "lw      $s1, 0x044($s0)" "\n"
	"\t" "\t" "lw      $s2, 0x048($s0)" "\n"
	"\t" "\t" "lw      $s3, 0x04C($s0)" "\n"
	"\t" "\t" "lw      $s4, 0x050($s0)" "\n"
	"\t" "\t" "lw      $s5, 0x054($s0)" "\n"
	"\t" "\t" "lw      $s6, 0x058($s0)" "\n"
	"\t" "\t" "lw      $s7, 0x05C($s0)" "\n"

	"\t" "\t" "lw      $t8, 0x060($s0)" "\n"
	"\t" "\t" "lw      $t9, 0x064($s0)" "\n"

	// "\t" "\t" "lw      $k0, 0x068($s0)" "\n"
	"\t" "\t" "lw      $k1, 0x06C($s0)" "\n"

	"\t" "\t" "lw      $gp, 0x070($s0)" "\n"
	// "\t" "\t" "lw      $sp, 0x074($s0)" "\n"
	"\t" "\t" "lw      $fp, 0x078($s0)" "\n"
	// "\t" "\t" "lw      $ra, 0x07C($s0)" "\n"

	"\t" "\t" "lw      $t0, 0x080($s0)" "\n"
	"\t" "\t" "lw      $t1, 0x084($s0)" "\n"
	"\t" "\t" "mthi    $t0" "\n"  // hi
	"\t" "\t" "mtlo    $t1" "\n"  // lo
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "lw      $t0, 0x088($s0)" "\n"
	"\t" "\t" "lw      $t1, 0x08C($s0)" "\n"
	"\t" "\t" "mtc0    $t0, $3" "\n"  // IOP_COP0_BPC
	"\t" "\t" "mtc0    $t1, $5" "\n"  // IOP_COP0_BDA

	// gap for IOP_COP0_DCIC
	"\t" "\t" "lw      $t2, 0x094($s0)" "\n"
	"\t" "\t" "lw      $t3, 0x098($s0)" "\n"
	"\t" "\t" "mtc0    $t2, $8" "\n"  // IOP_COP0_BADVADDR
	"\t" "\t" "mtc0    $t3, $9" "\n"  // IOP_COP0_BDAM

	"\t" "\t" "lw      $t4, 0x09C($s0)" "\n"
	"\t" "\t" "lw      $t5, 0x0A0($s0)" "\n"
	"\t" "\t" "mtc0    $t4, $11" "\n"  // IOP_COP0_BPCM
	"\t" "\t" "mtc0    $t5, $12" "\n"  // SR

	"\t" "\t" "lw      $t6, 0x0A4($s0)" "\n"
	"\t" "\t" "lw      $t7, 0x0A8($s0)" "\n"
	"\t" "\t" "mtc0    $t6, $13" "\n"  // Cause
	"\t" "\t" "mtc0    $t7, $14" "\n"  // EPC

	"\t" "\t" "lw      $t0, 0x020($s0)" "\n"
	"\t" "\t" "lw      $t1, 0x024($s0)" "\n"
	"\t" "\t" "lw      $t2, 0x028($s0)" "\n"
	"\t" "\t" "lw      $t3, 0x02c($s0)" "\n"
	"\t" "\t" "lw      $t4, 0x030($s0)" "\n"
	"\t" "\t" "lw      $t5, 0x034($s0)" "\n"
	"\t" "\t" "lw      $t6, 0x038($s0)" "\n"
	"\t" "\t" "lw      $t7, 0x03C($s0)" "\n"

	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end _iop_load_frame" "\n"

	"\t" ".global _iop_ex_cmn" "\n"
	"\t" ".ent _iop_ex_cmn" "\n"
	"\t" "_iop_ex_cmn:" "\n"
	"\t" "\t" "addiu   $sp, $sp, -0x10" "\n"
	"\t" "\t" "sw      $ra, 0x00($sp)" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "jal     _iop_save_frame" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "jal     iop_exception_dispatcher" "\n"
	"\t" "\t" "move    $a0, $s0" "\n"

	"\t" "\t" "jal     _iop_load_frame" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "lw      $ra, 0x00($sp)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "addiu   $sp, $sp, 0x10" "\n"
	"\t" ".end _iop_ex_cmn" "\n"

	"\t" ".global _iop_ex_def_handler" "\n"
	"\t" ".ent _iop_ex_def_handler" "\n"
	"\t" "_iop_ex_def_handler:" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "___iop_ex_def_handler_start:" "\n"
	"\t" "\t" "la      $k0, _iop_ex_def_frame" "\n"

	"\t" "\t" "sw      $s0, 0x040($k0)" "\n"  // Save $s0
	"\t" "\t" "sw      $sp, 0x074($k0)" "\n"  // Save $sp
	"\t" "\t" "mfc0    $s0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "sw      $ra, 0x07C($k0)" "\n"  // Save $ra
	"\t" "\t" "lw      $sp, 0x410($zero)" "\n"  // k0 saved here by default exception prologue (at 0x80)
	"\t" "\t" "sw      $s0, 0x090($k0)" "\n"  // Save IOP_COP0_DCIC
	"\t" "\t" "sw      $sp, 0x068($k0)" "\n"  // Save $k0
	"\t" "\t" "lw      $at, 0x400($0)" "\n"  // at is saved here by default exception prologue (at 0x80)
	"\t" "\t" "la      $sp, _iop_ex_def_stack + " STR(_EX_DEF_STACK_SIZE) "" "\n"

	"\t" "\t" "jal     _iop_ex_cmn" "\n"
	"\t" "\t" "move    $s0, $k0" "\n"

	"\t" "\t" "lw      $k0, 0x068($s0)" "\n"  // restore k0
	"\t" "\t" "lw      $sp, 0x074($s0)" "\n"  // restore sp
	"\t" "\t" "lw      $ra, 0x07C($s0)" "\n"  // restore ra

	"\t" "\t" "lw      $k1, 0x090($s0)" "\n"  // restore IOP_COP0_DCIC
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $k1, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "lw      $k1, 0x0A8($s0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $k1, $14" "\n"  // EPC
	"\t" "\t" "lw      $s0, 0x040($s0)" "\n"  // restore s0
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $k1" "\n"
	"\t" "\t" "cop0    0x10" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end _iop_ex_def_handler" "\n"

	"\t" ".global _iop_ex_break_handler" "\n"
	"\t" ".ent _iop_ex_break_handler" "\n"
	"\t" "_iop_ex_break_handler:" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "j       ___iop_ex_def_handler_start" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end _iop_ex_break_handler" "\n"

	"\t" ".global _iop_ex_dbg_handler" "\n"
	"\t" ".ent _iop_ex_dbg_handler" "\n"
	"\t" "_iop_ex_dbg_handler:" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "la      $k0, _iop_ex_dbg_frame" "\n"

	"\t" "\t" "sw      $s0, 0x040($k0)" "\n"  // Save $s0
	"\t" "\t" "sw      $sp, 0x074($k0)" "\n"  // Save $sp
	"\t" "\t" "sw      $ra, 0x07C($k0)" "\n"  // Save $ra
	"\t" "\t" "lw      $sp, 0x420($zero)" "\n"  // k0 saved here by debug exception prologue (at 0x40)
	"\t" "\t" "lw      $s0, 0x430($0)" "\n"  // IOP_COP0_DCIC is saved here by debug exception prologue (at 0x40)
	"\t" "\t" "sw      $sp, 0x068($k0)" "\n"  // Save $k0
	"\t" "\t" "sw      $s0, 0x090($k0)" "\n"  // Save IOP_COP0_DCIC
	"\t" "\t" "la      $sp, _iop_ex_dbg_stack + " STR(_EX_DBG_STACK_SIZE) "" "\n"

	"\t" "\t" "jal     _iop_ex_cmn" "\n"
	"\t" "\t" "move    $s0, $k0" "\n"

	"\t" "\t" "lw      $k0, 0x068($s0)" "\n"  // restore k0
	"\t" "\t" "lw      $sp, 0x074($s0)" "\n"  // restore sp
	"\t" "\t" "lw      $ra, 0x07C($s0)" "\n"  // restore ra

	"\t" "\t" "lw      $k1, 0x090($s0)" "\n"  // restore IOP_COP0_DCIC
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $k1, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "lw      $k1, 0x0A8($s0)" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $k1, $14" "\n"  // EPC
	"\t" "\t" "lw      $s0, 0x040($s0)" "\n"  // restore s0
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $k1" "\n"
	"\t" "\t" "cop0    0x10" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end _iop_ex_dbg_handler" "\n"

	"\t" ".set pop" "\n"
);
