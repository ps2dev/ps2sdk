/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: ee_exceptions.S $
#
# EEDEBUG - EE debugging library.
#
# ee_exceptions.S - low-level EE exception-handler code.
#
*/

#include <ee_cop0_defs.h>
#include "eedebug_defs.h"

.text

.set push
.set noreorder

.local _ee_save_frame
.ent _ee_save_frame
_ee_save_frame:
                        sq          $zero, 0x000($s0)
.set noat
                        sq          $at,   0x010($s0)
.set at
                        sq          $v0,   0x020($s0)
                        sq          $v1,   0x030($s0)
                        sq          $a0,   0x040($s0)
                        sq          $a1,   0x050($s0)
                        sq          $a2,   0x060($s0)
                        sq          $a3,   0x070($s0)
                        sq          $t0,   0x080($s0)
                        sq          $t1,   0x090($s0)
                        sq          $t2,   0x0A0($s0)
                        sq          $t3,   0x0B0($s0)
                        sq          $t4,   0x0C0($s0)
                        sq          $t5,   0x0D0($s0)
                        sq          $t6,   0x0E0($s0)
                        sq          $t7,   0x0F0($s0)
#                        sq          $s0,   0x100($s0) # s0 already saved
                        sq          $s1,   0x110($s0)
                        sq          $s2,   0x120($s0)
                        sq          $s3,   0x130($s0)
                        sq          $s4,   0x140($s0)
                        sq          $s5,   0x150($s0)
                        sq          $s6,   0x160($s0)
                        sq          $s7,   0x170($s0)

                        sq          $t8,   0x180($s0)
                        sq          $t9,   0x190($s0)

#                        sq          $k0,   0x1A0($s0)
                        sq          $k1,   0x1B0($s0)

                        sq          $gp,   0x1C0($s0)
#                        sq          $sp,   0x1D0($s0)
                        sq          $fp,   0x1E0($s0)

#                        sq          $ra,   0x1F0($s0)

                        mfc0        $t1, EE_COP0_Status
                        sw          $t1,   0x200($s0)

                        mfc0        $t1, EE_COP0_Cause
                        sw          $t1,   0x204($s0)

                        mfc0        $t1, EE_COP0_EPC
                        sw          $t1,   0x208($s0)

                        mfc0        $t1, EE_COP0_ErrorEPC
                        sw          $t1,   0x20C($s0)

                        mfc0        $t1, EE_COP0_BadVAddr
                        sw          $t1,   0x210($s0)

                        mfhi        $t1
                        sw          $t1,   0x214($s0)

                        mfhi1       $t1
                        sw          $t1,   0x218($s0)

                        mflo        $t1
                        sw          $t1,   0x21C($s0)

                        mflo1       $t1
                        sw          $t1,   0x220($s0)

                        mfsa        $t1
                        sw          $t1,   0x224($s0)

                        mfbpc       $t1
                        sw          $t1,   0x228($s0)
                        sync.l

                        li          $t1,   EE_BPC_BED
                        mtbpc       $t1

                        mfiab       $t1
                        sw          $t1,   0x22C($s0)

                        mfiabm      $t1
                        sw          $t1,   0x230($s0)

                        mfdab       $t1
                        sw          $t1,   0x234($s0)

                        mfdabm      $t1
                        sw          $t1,   0x238($s0)

                        mfdvb       $t1
                        sw          $t1,   0x23C($s0)

                        mfdvbm      $t1
                        sw          $t1,   0x240($s0)

                        jr          $ra
                        nop
.end _ee_save_frame

.global _ee_load_frame
.ent _ee_load_frame
_ee_load_frame:
                        lq          $zero, 0x000($s0)
.set noat
                        lq          $at,   0x010($s0)
.set at

                        lq          $v0,   0x020($s0)
                        lq          $v1,   0x030($s0)
                        lq          $a0,   0x040($s0)
                        lq          $a1,   0x050($s0)
                        lq          $a2,   0x060($s0)
                        lq          $a3,   0x070($s0)
                        lq          $t1,   0x090($s0)
                        lq          $t2,   0x0A0($s0)
                        lq          $t3,   0x0B0($s0)
                        lq          $t4,   0x0C0($s0)
                        lq          $t5,   0x0D0($s0)
                        lq          $t6,   0x0E0($s0)
                        lq          $t7,   0x0F0($s0)
#                        lq          $s0,   0x100($s0)
                        lq          $s1,   0x110($s0)
                        lq          $s2,   0x120($s0)
                        lq          $s3,   0x130($s0)
                        lq          $s4,   0x140($s0)
                        lq          $s5,   0x150($s0)
                        lq          $s6,   0x160($s0)
                        lq          $s7,   0x170($s0)

                        lq          $t8,   0x180($s0)
                        lq          $t9,   0x190($s0)

                        lq          $k0,   0x1A0($s0)
                        lq          $k1,   0x1B0($s0)

                        lq          $gp,   0x1C0($s0)
                        lq          $sp,   0x1D0($s0)
                        lq          $fp,   0x1E0($s0)
#                        lq          $ra,   0x1F0($s0)

                        lw          $t0,   0x200($s0)
                        mtc0        $t0, EE_COP0_Status

                        lw          $t0,   0x204($s0)
                        mtc0        $t0, EE_COP0_Cause

                        lw          $t0,   0x208($s0)
                        mtc0        $t0, EE_COP0_EPC

                        lw          $t0,   0x20C($s0)
                        mtc0        $t0, EE_COP0_ErrorEPC

                        lw          $t0,   0x210($s0)
                        mtc0        $t0, EE_COP0_BadVAddr

                        lw          $t0,   0x214($s0)
                        mthi        $t0

                        lw          $t0,   0x218($s0)
                        mthi        $t0

                        lw          $t0,   0x21C($s0)
                        mtlo        $t0

                        lw          $t0,   0x220($s0)
                        mtlo1       $t0

                        lw          $t0,   0x224($s0)
                        mtsa        $t0

                        lw          $t0,   0x22C($s0)
                        mtiab       $t0

                        lw          $t0,   0x230($s0)
                        mtiabm      $t0

                        lw          $t0,   0x234($s0)
                        mtdab       $t0

                        lw          $t0,   0x238($s0)
                        mtdabm      $t0

                        lw          $t0,   0x23C($s0)
                        mtdvb       $t0

                        lw          $t0,   0x240($s0)
                        mtdvbm      $t0
                        sync.p

                        lw          $t0,   0x228($s0)
                        mtbpc       $t0
                        sync.p

                        jr          $ra
                        lq          $t0,   0x080($s0)
.end _ee_load_frame

.global __ee_level2_ex_vector
.ent __ee_level2_ex_vector
__ee_level2_ex_vector:
                        nop
                        nop
                        nop
                        j           __ee_level2_ex_handler      
                        nop
                        nop
                        nop
                        nop

.end __ee_level2_ex_vector

.global __ee_level1_ex_vector
.ent __ee_level1_ex_vector
__ee_level1_ex_vector:
                        nop
                        nop
                        la          $k0, __ee_ex_l1_frame
                        sq          $s0, 0x100($k0) # save s0
                        sq          $zero, 0x1A0($k0) # k0 is not preserved in l1 exceptions
                        sq          $sp, 0x1D0($k0) # save sp
                        sq          $ra, 0x1F0($k0) # save ra

                        la          $sp, __ee_ex_l1_stack + (_EX_L1_STACK_SIZE)
                        move        $s0, $k0

                        jal         _ee_save_frame
                        nop

                        jal         ee_level1_ex_dispatcher
                        move        $a0, $s0

                        jal         _ee_load_frame
                        nop

                        lq          $ra, 0x1F0($s0)
                        lq          $s0, 0x100($s0)
                        sync.l

                        eret
                        nop
                        nop
.end __ee_level1_ex_vector


.global __ee_level2_ex_handler
.ent __ee_level2_ex_handler
__ee_level2_ex_handler:
                        nop
                        nop
                        sq          $k0, -0x20($0) # preserve k0
                        la          $k0, __ee_ex_l2_frame
                        sq          $s0, 0x100($k0) # save s0
                        sq          $sp, 0x1D0($k0) # save sp
                        lq          $sp, -0x20($0)
                        sq          $sp, 0x1A0($k0) # save k0
                        sq          $ra, 0x1F0($k0) # save ra

                        la          $sp, __ee_ex_l2_stack + (_EX_L2_STACK_SIZE)
                        move        $s0, $k0

                        jal         _ee_save_frame
                        nop

                        jal         ee_level2_ex_dispatcher
                        move        $a0, $s0

                        jal         _ee_load_frame
                        nop

                        lq          $ra, 0x1F0($s0)
                        lq          $s0, 0x100($s0)
                        sync.l

                        eret
                        nop
                        nop

.end __ee_level2_ex_handler

.balign 128

.set pop
