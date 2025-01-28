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
 * low-level IOP "debug" helper functions.
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

	"\t" ".global iop_dbg_get_dcic" "\n"
	"\t" ".ent iop_dbg_get_dcic" "\n"
	"\t" "iop_dbg_get_dcic:" "\n"
	"\t" "\t" "mfc0    $v0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_get_dcic" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_get_bpc" "\n"
	"\t" ".ent iop_dbg_get_bpc" "\n"
	"\t" "iop_dbg_get_bpc:" "\n"
	"\t" "\t" "mtc0    $v0, " STR(IOP_COP0_BPC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_get_bpc" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_get_bpcm" "\n"
	"\t" ".ent iop_dbg_get_bpcm" "\n"
	"\t" "iop_dbg_get_bpcm:" "\n"
	"\t" "\t" "mfc0    $v0, " STR(IOP_COP0_BPCM) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end iop_dbg_get_bpcm" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_get_bda" "\n"
	"\t" ".ent iop_dbg_get_bda" "\n"
	"\t" "iop_dbg_get_bda:" "\n"
	"\t" "\t" "mfc0    $v0, " STR(IOP_COP0_BDA) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_get_bda" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_get_bdam" "\n"
	"\t" ".ent iop_dbg_get_bdam" "\n"
	"\t" "iop_dbg_get_bdam:" "\n"
	"\t" "\t" "mfc0    $v0, " STR(IOP_COP0_BDAM) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_get_bdam" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_set_dcic" "\n"
	"\t" ".ent iop_dbg_set_dcic" "\n"
	"\t" "iop_dbg_set_dcic:" "\n"
	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_set_dcic" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_set_bpc" "\n"
	"\t" ".ent iop_dbg_set_bpc" "\n"
	"\t" "iop_dbg_set_bpc:" "\n"
	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_BPC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_set_bpc" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_set_bpcm" "\n"
	"\t" ".ent iop_dbg_set_bpcm" "\n"
	"\t" "iop_dbg_set_bpcm:" "\n"
	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_BPCM) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_set_bpcm" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_set_bda" "\n"
	"\t" ".ent iop_dbg_set_bda" "\n"
	"\t" "iop_dbg_set_bda:" "\n"
	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_BDA) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_set_bda" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global iop_dbg_set_bdam" "\n"
	"\t" ".ent iop_dbg_set_bdam" "\n"
	"\t" "iop_dbg_set_bdam:" "\n"
	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_BDAM) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end iop_dbg_set_bdam" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global _iop_dbg_set_bpda" "\n"
	"\t" ".ent _iop_dbg_set_bpda" "\n"
	"\t" "_iop_dbg_set_bpda:" "\n"

	"\t" "\t" "mfc0    $t0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $zero, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_BDA) "" "\n"
	"\t" "\t" "mtc0    $a1, " STR(IOP_COP0_BDAM) "" "\n"

	"\t" "\t" "li      $t1, ~(" STR(IOP_DCIC_DR) " | " STR(IOP_DCIC_DW) ")" "\n"
	"\t" "\t" "and     $t0, $t0, $t1" "\n"

	"\t" "\t" "li      $t1, (" STR(IOP_DCIC_TR) " | " STR(IOP_DCIC_DAE) " | " STR(IOP_DCIC_DE) ")" "\n"
	"\t" "\t" "or      $t0, $t0, $t1" "\n"
	"\t" "\t" "or      $t0, $t0, $a2" "\n"
	"\t" "\t" "mtc0    $t0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end _iop_dbg_set_bpda" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global _iop_dbg_set_bpx" "\n"
	"\t" ".ent _iop_dbg_set_bpx" "\n"
	"\t" "_iop_dbg_set_bpx:" "\n"

	"\t" "\t" "mfc0    $t0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "mtc0    $zero, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "mtc0    $a0, " STR(IOP_COP0_BPC) "" "\n"
	"\t" "\t" "mtc0    $a1, " STR(IOP_COP0_BPCM) "" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "li      $t1, ~(" STR(IOP_DCIC_PC) ")" "\n"
	"\t" "\t" "and     $t0, $t0, $t1" "\n"

	"\t" "\t" "li      $t1, (" STR(IOP_DCIC_TR) " | " STR(IOP_DCIC_PCE) " | " STR(IOP_DCIC_DE) ")" "\n"
	"\t" "\t" "or      $t0, $t0, $t1" "\n"
	"\t" "\t" "or      $t0, $t0, $a2" "\n"
	"\t" "\t" "mtc0    $t0, " STR(IOP_COP0_DCIC) "" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end _iop_dbg_set_bpx" "\n"

	"\t" ".set pop" "\n"
);
