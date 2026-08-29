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
 * low-level EE exception-handler code.
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

	"\t" ".local _ee_save_frame" "\n"
	"\t" ".ent _ee_save_frame" "\n"
	"\t" "_ee_save_frame:" "\n"
	"\t" "\t" "sq          $zero, 0x000($s0)" "\n"
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" "\t" "sq          $at,   0x010($s0)" "\n"
	"\t" ".set pop" "\n"
	"\t" "\t" "sq          $v0,   0x020($s0)" "\n"
	"\t" "\t" "sq          $v1,   0x030($s0)" "\n"
	"\t" "\t" "sq          $a0,   0x040($s0)" "\n"
	"\t" "\t" "sq          $a1,   0x050($s0)" "\n"
	"\t" "\t" "sq          $a2,   0x060($s0)" "\n"
	"\t" "\t" "sq          $a3,   0x070($s0)" "\n"
	"\t" "\t" "sq          $" STR(t0) ",   0x080($s0)" "\n"
	"\t" "\t" "sq          $" STR(t1) ",   0x090($s0)" "\n"
	"\t" "\t" "sq          $" STR(t2) ",   0x0A0($s0)" "\n"
	"\t" "\t" "sq          $" STR(t3) ",   0x0B0($s0)" "\n"
	"\t" "\t" "sq          $" STR(t4) ",   0x0C0($s0)" "\n"
	"\t" "\t" "sq          $" STR(t5) ",   0x0D0($s0)" "\n"
	"\t" "\t" "sq          $" STR(t6) ",   0x0E0($s0)" "\n"
	"\t" "\t" "sq          $" STR(t7) ",   0x0F0($s0)" "\n"
	// "\t" "\t" "sq          $s0,   0x100($s0)" "\n" // s0 already saved
	"\t" "\t" "sq          $s1,   0x110($s0)" "\n"
	"\t" "\t" "sq          $s2,   0x120($s0)" "\n"
	"\t" "\t" "sq          $s3,   0x130($s0)" "\n"
	"\t" "\t" "sq          $s4,   0x140($s0)" "\n"
	"\t" "\t" "sq          $s5,   0x150($s0)" "\n"
	"\t" "\t" "sq          $s6,   0x160($s0)" "\n"
	"\t" "\t" "sq          $s7,   0x170($s0)" "\n"

	"\t" "\t" "sq          $t8,   0x180($s0)" "\n"
	"\t" "\t" "sq          $t9,   0x190($s0)" "\n"

	// "\t" "\t" "sq          $k0,   0x1A0($s0)" "\n"
	"\t" "\t" "sq          $k1,   0x1B0($s0)" "\n"

	"\t" "\t" "sq          $gp,   0x1C0($s0)" "\n"
	// "\t" "\t" "sq          $sp,   0x1D0($s0)" "\n"
	"\t" "\t" "sq          $fp,   0x1E0($s0)" "\n"

	// "\t" "\t" "sq          $ra,   0x1F0($s0)" "\n"

	"\t" "\t" "mfc0        $" STR(t1) ", " STR(EE_COP0_Status) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x200($s0)" "\n"

	"\t" "\t" "mfc0        $" STR(t1) ", " STR(EE_COP0_Cause) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x204($s0)" "\n"

	"\t" "\t" "mfc0        $" STR(t1) ", " STR(EE_COP0_EPC) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x208($s0)" "\n"

	"\t" "\t" "mfc0        $" STR(t1) ", " STR(EE_COP0_ErrorEPC) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x20C($s0)" "\n"

	"\t" "\t" "mfc0        $" STR(t1) ", " STR(EE_COP0_BadVAddr) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x210($s0)" "\n"

	"\t" "\t" "mfhi        $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x214($s0)" "\n"

	"\t" "\t" "mfhi1       $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x218($s0)" "\n"

	"\t" "\t" "mflo        $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x21C($s0)" "\n"

	"\t" "\t" "mflo1       $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x220($s0)" "\n"

	"\t" "\t" "mfsa        $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x224($s0)" "\n"

	"\t" "\t" "mfbpc       $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x228($s0)" "\n"
	"\t" "\t" "sync.l" "\n"

	"\t" "\t" "li          $" STR(t1) ",   " STR(EE_BPC_BED) "" "\n"
	"\t" "\t" "mtbpc       $" STR(t1) "" "\n"

	"\t" "\t" "mfiab       $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x22C($s0)" "\n"

	"\t" "\t" "mfiabm      $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x230($s0)" "\n"

	"\t" "\t" "mfdab       $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x234($s0)" "\n"

	"\t" "\t" "mfdabm      $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x238($s0)" "\n"

	"\t" "\t" "mfdvb       $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x23C($s0)" "\n"

	"\t" "\t" "mfdvbm      $" STR(t1) "" "\n"
	"\t" "\t" "sw          $" STR(t1) ",   0x240($s0)" "\n"

	"\t" "\t" "jr          $ra" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end _ee_save_frame" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".global _ee_load_frame" "\n"
	"\t" ".ent _ee_load_frame" "\n"
	"\t" "_ee_load_frame:" "\n"
	"\t" "\t" "lq          $zero, 0x000($s0)" "\n"
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" "\t" "lq          $at,   0x010($s0)" "\n"
	"\t" ".set pop" "\n"

	"\t" "\t" "lq          $v0,   0x020($s0)" "\n"
	"\t" "\t" "lq          $v1,   0x030($s0)" "\n"
	"\t" "\t" "lq          $a0,   0x040($s0)" "\n"
	"\t" "\t" "lq          $a1,   0x050($s0)" "\n"
	"\t" "\t" "lq          $a2,   0x060($s0)" "\n"
	"\t" "\t" "lq          $a3,   0x070($s0)" "\n"
	"\t" "\t" "lq          $" STR(t1) ",   0x090($s0)" "\n"
	"\t" "\t" "lq          $" STR(t2) ",   0x0A0($s0)" "\n"
	"\t" "\t" "lq          $" STR(t3) ",   0x0B0($s0)" "\n"
	"\t" "\t" "lq          $" STR(t4) ",   0x0C0($s0)" "\n"
	"\t" "\t" "lq          $" STR(t5) ",   0x0D0($s0)" "\n"
	"\t" "\t" "lq          $" STR(t6) ",   0x0E0($s0)" "\n"
	"\t" "\t" "lq          $" STR(t7) ",   0x0F0($s0)" "\n"
	// "\t" "\t" "lq          $s0,   0x100($s0)" "\n"
	"\t" "\t" "lq          $s1,   0x110($s0)" "\n"
	"\t" "\t" "lq          $s2,   0x120($s0)" "\n"
	"\t" "\t" "lq          $s3,   0x130($s0)" "\n"
	"\t" "\t" "lq          $s4,   0x140($s0)" "\n"
	"\t" "\t" "lq          $s5,   0x150($s0)" "\n"
	"\t" "\t" "lq          $s6,   0x160($s0)" "\n"
	"\t" "\t" "lq          $s7,   0x170($s0)" "\n"

	"\t" "\t" "lq          $t8,   0x180($s0)" "\n"
	"\t" "\t" "lq          $t9,   0x190($s0)" "\n"

	"\t" "\t" "lq          $k0,   0x1A0($s0)" "\n"
	"\t" "\t" "lq          $k1,   0x1B0($s0)" "\n"

	"\t" "\t" "lq          $gp,   0x1C0($s0)" "\n"
	"\t" "\t" "lq          $sp,   0x1D0($s0)" "\n"
	"\t" "\t" "lq          $fp,   0x1E0($s0)" "\n"
	// "\t" "\t" "lq          $ra,   0x1F0($s0)" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x200($s0)" "\n"
	"\t" "\t" "mtc0        $" STR(t0) ", " STR(EE_COP0_Status) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x204($s0)" "\n"
	"\t" "\t" "mtc0        $" STR(t0) ", " STR(EE_COP0_Cause) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x208($s0)" "\n"
	"\t" "\t" "mtc0        $" STR(t0) ", " STR(EE_COP0_EPC) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x20C($s0)" "\n"
	"\t" "\t" "mtc0        $" STR(t0) ", " STR(EE_COP0_ErrorEPC) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x210($s0)" "\n"
	"\t" "\t" "mtc0        $" STR(t0) ", " STR(EE_COP0_BadVAddr) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x214($s0)" "\n"
	"\t" "\t" "mthi        $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x218($s0)" "\n"
	"\t" "\t" "mthi        $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x21C($s0)" "\n"
	"\t" "\t" "mtlo        $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x220($s0)" "\n"
	"\t" "\t" "mtlo1       $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x224($s0)" "\n"
	"\t" "\t" "mtsa        $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x22C($s0)" "\n"
	"\t" "\t" "mtiab       $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x230($s0)" "\n"
	"\t" "\t" "mtiabm      $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x234($s0)" "\n"
	"\t" "\t" "mtdab       $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x238($s0)" "\n"
	"\t" "\t" "mtdabm      $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x23C($s0)" "\n"
	"\t" "\t" "mtdvb       $" STR(t0) "" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x240($s0)" "\n"
	"\t" "\t" "mtdvbm      $" STR(t0) "" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "lw          $" STR(t0) ",   0x228($s0)" "\n"
	"\t" "\t" "mtbpc       $" STR(t0) "" "\n"
	"\t" "\t" "sync.p" "\n"

	"\t" "\t" "jr          $ra" "\n"
	"\t" "\t" "lq          $" STR(t0) ",   0x080($s0)" "\n"
	"\t" ".end _ee_load_frame" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".global __ee_level2_ex_vector" "\n"
	"\t" ".ent __ee_level2_ex_vector" "\n"
	"\t" "__ee_level2_ex_vector:" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "j           __ee_level2_ex_handler" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end __ee_level2_ex_vector" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".global __ee_level1_ex_vector" "\n"
	"\t" ".ent __ee_level1_ex_vector" "\n"
	"\t" "__ee_level1_ex_vector:" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "la          $k0, __ee_ex_l1_frame" "\n"
	"\t" "\t" "sq          $s0, 0x100($k0)" "\n" // save s0
	"\t" "\t" "sq          $zero, 0x1A0($k0)" "\n" // k0 is not preserved in l1 exceptions
	"\t" "\t" "sq          $sp, 0x1D0($k0)" "\n" // save sp
	"\t" "\t" "sq          $ra, 0x1F0($k0)" "\n" // save ra

	"\t" "\t" "la          $sp, __ee_ex_l1_stack + (" STR(_EX_L1_STACK_SIZE) ")" "\n"
	"\t" "\t" "move        $s0, $k0" "\n"

	"\t" "\t" "jal         _ee_save_frame" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "jal         ee_level1_ex_dispatcher" "\n"
	"\t" "\t" "move        $a0, $s0" "\n"

	"\t" "\t" "jal         _ee_load_frame" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "lq          $ra, 0x1F0($s0)" "\n"
	"\t" "\t" "lq          $s0, 0x100($s0)" "\n"
	"\t" "\t" "sync.l" "\n"

	"\t" "\t" "eret" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" ".end __ee_level1_ex_vector" "\n"

	"\t" ".set pop" "\n"
);

__asm__
(
	"\t" ".set push" "\n"
	"\t" ".set noreorder" "\n"

	"\t" ".global __ee_level2_ex_handler" "\n"
	"\t" ".ent __ee_level2_ex_handler" "\n"
	"\t" "__ee_level2_ex_handler:" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "sq          $k0, -0x20($0)" "\n" // preserve k0
	"\t" "\t" "la          $k0, __ee_ex_l2_frame" "\n"
	"\t" "\t" "sq          $s0, 0x100($k0)" "\n" // save s0
	"\t" "\t" "sq          $sp, 0x1D0($k0)" "\n" // save sp
	"\t" "\t" "lq          $sp, -0x20($0)" "\n"
	"\t" "\t" "sq          $sp, 0x1A0($k0)" "\n" // save k0
	"\t" "\t" "sq          $ra, 0x1F0($k0)" "\n" // save ra

	"\t" "\t" "la          $sp, __ee_ex_l2_stack + (" STR(_EX_L2_STACK_SIZE) ")" "\n"
	"\t" "\t" "move        $s0, $k0" "\n"

	"\t" "\t" "jal         _ee_save_frame" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "jal         ee_level2_ex_dispatcher" "\n"
	"\t" "\t" "move        $a0, $s0" "\n"

	"\t" "\t" "jal         _ee_load_frame" "\n"
	"\t" "\t" "nop" "\n"

	"\t" "\t" "lq          $ra, 0x1F0($s0)" "\n"
	"\t" "\t" "lq          $s0, 0x100($s0)" "\n"
	"\t" "\t" "sync.l" "\n"

	"\t" "\t" "eret" "\n"
	"\t" "\t" "nop" "\n"
	"\t" "\t" "nop" "\n"

	"\t" ".end __ee_level2_ex_handler" "\n"

	"\t" ".set pop" "\n"
);
