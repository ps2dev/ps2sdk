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
 * EEDEBUG - EE debugging library.
 * low-level EE "debug" helper functions.
 */

#include <ee_cop0_defs.h>
#include "eedebug_defs.h"

#define ABI_EABI64 // force all register names to EABI64 (legacy toolchain)
#include "as_reg_compat.h"

#define STRINNER(x) #x
#define STR(x) STRINNER(x)

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".ent ee_dbg_get_bpc" "\n"
	"\t" ".global ee_dbg_get_bpc" "\n"
	"\t" "ee_dbg_get_bpc:" "\n"
	"\t" "\t" "mfbpc   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end ee_dbg_get_bpc" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_get_iab" "\n"
	"\t" ".ent ee_dbg_get_iab" "\n"
	"\t" "ee_dbg_get_iab:" "\n"
	"\t" "\t" "mfiab   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_get_iab" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_get_iabm" "\n"
	"\t" ".ent ee_dbg_get_iabm" "\n"
	"\t" "ee_dbg_get_iabm:" "\n"
	"\t" "\t" "mfiabm   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_get_iabm" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_get_dab" "\n"
	"\t" ".ent ee_dbg_get_dab" "\n"
	"\t" "ee_dbg_get_dab:" "\n"
	"\t" "\t" "mfdab   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_get_dab" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_get_dabm" "\n"
	"\t" ".ent ee_dbg_get_dabm" "\n"
	"\t" "ee_dbg_get_dabm:" "\n"
	"\t" "\t" "mfdabm   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_get_dabm" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_get_dvb" "\n"
	"\t" ".ent ee_dbg_get_dvb" "\n"
	"\t" "ee_dbg_get_dvb:" "\n"
	"\t" "\t" "mfdvb   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_get_dvb" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_get_dvbm" "\n"
	"\t" ".ent ee_dbg_get_dvbm" "\n"
	"\t" "ee_dbg_get_dvbm:" "\n"
	"\t" "\t" "mfdvbm   $v0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_get_dvbm" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_bpc" "\n"
	"\t" ".ent ee_dbg_set_bpc" "\n"
	"\t" "ee_dbg_set_bpc:" "\n"
	"\t" "\t" "mtbpc   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_bpc" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_iab" "\n"
	"\t" ".ent ee_dbg_set_iab" "\n"
	"\t" "ee_dbg_set_iab:" "\n"
	"\t" "\t" "mtiab   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_iab" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_iabm" "\n"
	"\t" ".ent ee_dbg_set_iabm" "\n"
	"\t" "ee_dbg_set_iabm:" "\n"
	"\t" "\t" "mtiabm   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_iabm" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_dab" "\n"
	"\t" ".ent ee_dbg_set_dab" "\n"
	"\t" "ee_dbg_set_dab:" "\n"
	"\t" "\t" "mtdab   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_dab" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_dabm" "\n"
	"\t" ".ent ee_dbg_set_dabm" "\n"
	"\t" "ee_dbg_set_dabm:" "\n"
	"\t" "\t" "mtdabm   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_dabm" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_dvb" "\n"
	"\t" ".ent ee_dbg_set_dvb" "\n"
	"\t" "ee_dbg_set_dvb:" "\n"
	"\t" "\t" "mtdvb   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_dvb" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global ee_dbg_set_dvbm" "\n"
	"\t" ".ent ee_dbg_set_dvbm" "\n"
	"\t" "ee_dbg_set_dvbm:" "\n"
	"\t" "\t" "mtdvbm   $a0" "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end ee_dbg_set_dvbm" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global _ee_dbg_set_bpda" "\n"
	"\t" ".ent _ee_dbg_set_bpda" "\n"
	"\t" "_ee_dbg_set_bpda:" "\n"

	"\t" "\t" "mfbpc   $" STR(t0) "\n"
	"\t" "\t" "li      $9, (" STR(EE_BPC_BED) ")" "\n"
	"\t" "\t" "mtbpc   $9" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "mtdab   $a0" "\n"
	"\t" "\t" "mtdabm  $a1" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "li      $9, ~(" STR(EE_BPC_DRE) " | " STR(EE_BPC_DWE) " | " STR(EE_BPC_DUE) " | " STR(EE_BPC_DSE) " | " STR(EE_BPC_DKE) " | " STR(EE_BPC_DXE) " | " STR(EE_BPC_DTE) " | " STR(EE_BPC_DWB) " | " STR(EE_BPC_DRB) ")" "\n"
	"\t" "\t" "and     $" STR(t0) ", $" STR(t0) ", $9" "\n"

	"\t" "\t" "li      $9, (" STR(EE_BPC_DTE) ")" "\n"
	"\t" "\t" "or      $" STR(t0) ", $" STR(t0) ", $9" "\n"
	"\t" "\t" "or      $" STR(t0) ", $" STR(t0) ", $a2" "\n"
	"\t" "\t" "mtbpc   $" STR(t0) "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end _ee_dbg_set_bpda" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global _ee_dbg_set_bpdv" "\n"
	"\t" ".ent _ee_dbg_set_bpdv" "\n"
	"\t" "_ee_dbg_set_bpdv:" "\n"

	"\t" "\t" "mfbpc   $" STR(t0) "\n"
	"\t" "\t" "li      $9, (" STR(EE_BPC_BED) ")" "\n"
	"\t" "\t" "mtbpc   $9" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "mtdvb   $a0" "\n"
	"\t" "\t" "mtdvbm  $a1" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "li      $9, (" STR(EE_BPC_DVE) " | " STR(EE_BPC_DTE) ")" "\n"
	"\t" "\t" "or      $" STR(t0) ", $" STR(t0) ", $9" "\n"
	"\t" "\t" "or      $" STR(t0) ", $" STR(t0) ", $a2" "\n"
	"\t" "\t" "mtbpc   $" STR(t0) "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end _ee_dbg_set_bpdv" "\n"

	"\t" "	.set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".set noat" "\n"

	"\t" ".global _ee_dbg_set_bpx" "\n"
	"\t" ".ent _ee_dbg_set_bpx" "\n"
	"\t" "_ee_dbg_set_bpx:" "\n"

	"\t" "\t" "mfbpc   $" STR(t0) "\n"
	"\t" "\t" "li      $9, (" STR(EE_BPC_BED) ")" "\n"
	"\t" "\t" "mtbpc   $9" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "mtiab   $a0" "\n"
	"\t" "\t" "mtiabm  $a1" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "li      $9, (" STR(EE_BPC_IAE) " | " STR(EE_BPC_ITE) ")" "\n"
	"\t" "\t" "or      $" STR(t0) ", $" STR(t0) ", $9" "\n"
	"\t" "\t" "or      $" STR(t0) ", $" STR(t0) ", $a2" "\n"
	"\t" "\t" "mtbpc   $" STR(t0) "\n"
	"\t" "\t" "sync.p" "\n"
	"\t" "\t" "jr      $ra" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end _ee_dbg_set_bpx" "\n"

	"\t" ".set pop" "\n"
);
