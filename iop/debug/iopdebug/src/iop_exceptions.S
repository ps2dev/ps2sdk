/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: iop_exceptions.S $
#
# IOPDEBUG - IOP debugging library.
#
# iop_exceptions.S - low-level IOP exception-handler code.
#
*/

#include <iop_cop0_defs.h>
#include "iopdebug_defs.h"

.set push
.set noreorder
.set noat

.global _iop_save_frame
.ent _iop_save_frame
_iop_save_frame:
                        sw      $zero, 0x000($s0)

                        sw      $at, 0x004($s0)

                        sw      $v0, 0x008($s0)
                        sw      $v1, 0x00C($s0)

                        sw      $a0, 0x010($s0)
                        sw      $a1, 0x014($s0)
                        sw      $a2, 0x018($s0)
                        sw      $a3, 0x01C($s0)

                        sw      $t0, 0x020($s0)
                        sw      $t1, 0x024($s0)
                        sw      $t2, 0x028($s0)
                        sw      $t3, 0x02C($s0)
                        sw      $t4, 0x030($s0)
                        sw      $t5, 0x034($s0)
                        sw      $t6, 0x038($s0)
                        sw      $t7, 0x03C($s0)

//                        sw      $s0, 0x040($s0)
                        sw      $s1, 0x044($s0)
                        sw      $s2, 0x048($s0)
                        sw      $s3, 0x04C($s0)
                        sw      $s4, 0x050($s0)
                        sw      $s5, 0x054($s0)
                        sw      $s6, 0x058($s0)
                        sw      $s7, 0x05C($s0)

                        sw      $t8, 0x060($s0)
                        sw      $t9, 0x064($s0)

//                        sw      $k0, 0x068($s0)
                        sw      $k1, 0x06C($s0)

                        sw      $gp, 0x070($s0)
//                        sw      $sp, 0x074($s0)
                        sw      $fp, 0x078($s0)
//                        sw      $ra, 0x07C($s0)

                        mfhi    $t0 // hi
                        mflo    $t1 // lo
                        nop // necessary??
                        sw      $t0, 0x080($s0)
                        sw      $t1, 0x084($s0)
                        nop // necessary??

                        mfc0    $t0, $3 // IOP_COP0_BPC
                        mfc0    $t1, $5 // IOP_COP0_BDA
                        mfc0    $t2, $8 // IOP_COP0_BADVADDR
                        mfc0    $t3, $9 // IOP_COP0_BDAM
                        mfc0    $t4, $11 // IOP_COP0_BPCM
                        mfc0    $t5, $12 // SR
                        mfc0    $t6, $13 // Cause
                        mfc0    $t7, $14 // EPC

                        sw      $t0, 0x088($s0)
                        sw      $t1, 0x08C($s0)
                        // gap for IOP_COP0_DCIC
                        sw      $t2, 0x094($s0)
                        sw      $t3, 0x098($s0)
                        sw      $t4, 0x09C($s0)
                        sw      $t5, 0x0A0($s0)
                        sw      $t6, 0x0A4($s0)
                        sw      $t7, 0x0A8($s0)

                        jr      $ra
                        nop
.end _iop_save_frame

.global _iop_load_frame
.ent _iop_load_frame
_iop_load_frame:
//                        lw      $zero, 0x000($s0)

                        lw      $at, 0x004($s0)

                        lw      $v0, 0x008($s0)
                        lw      $v1, 0x00C($s0)

                        lw      $a0, 0x010($s0)
                        lw      $a1, 0x014($s0)
                        lw      $a2, 0x018($s0)
                        lw      $a3, 0x01C($s0)

//                        lw      $s0, 0x040($s0)
                        lw      $s1, 0x044($s0)
                        lw      $s2, 0x048($s0)
                        lw      $s3, 0x04C($s0)
                        lw      $s4, 0x050($s0)
                        lw      $s5, 0x054($s0)
                        lw      $s6, 0x058($s0)
                        lw      $s7, 0x05C($s0)

                        lw      $t8, 0x060($s0)
                        lw      $t9, 0x064($s0)

//                        lw      $k0, 0x068($s0)
                        lw      $k1, 0x06C($s0)

                        lw      $gp, 0x070($s0)
//                        lw      $sp, 0x074($s0)
                        lw      $fp, 0x078($s0)
//                        lw      $ra, 0x07C($s0)

                        lw      $t0, 0x080($s0)
                        lw      $t1, 0x084($s0)
                        mthi    $t0 // hi
                        mtlo    $t1 // lo
                        nop
                        nop

                        lw      $t0, 0x088($s0)
                        lw      $t1, 0x08C($s0)
                        mtc0    $t0, $3 // IOP_COP0_BPC
                        mtc0    $t1, $5 // IOP_COP0_BDA

                        // gap for IOP_COP0_DCIC
                        lw      $t2, 0x094($s0)
                        lw      $t3, 0x098($s0)
                        mtc0    $t2, $8 // IOP_COP0_BADVADDR
                        mtc0    $t3, $9 // IOP_COP0_BDAM

                        lw      $t4, 0x09C($s0)
                        lw      $t5, 0x0A0($s0)
                        mtc0    $t4, $11 // IOP_COP0_BPCM
                        mtc0    $t5, $12 // SR

                        lw      $t6, 0x0A4($s0)
                        lw      $t7, 0x0A8($s0)
                        mtc0    $t6, $13 // Cause
                        mtc0    $t7, $14 // EPC

                        lw      $t0, 0x020($s0)
                        lw      $t1, 0x024($s0)
                        lw      $t2, 0x028($s0)
                        lw      $t3, 0x02c($s0)
                        lw      $t4, 0x030($s0)
                        lw      $t5, 0x034($s0)
                        lw      $t6, 0x038($s0)
                        lw      $t7, 0x03C($s0)

                        jr      $ra
                        nop
.end _iop_load_frame

.global _iop_ex_cmn
.ent _iop_ex_cmn
_iop_ex_cmn:
                        addiu   $sp, $sp, -0x10
                        sw      $ra, 0x00($sp)
                        nop

                        jal     _iop_save_frame
                        nop

                        jal     iop_exception_dispatcher
                        move    $a0, $s0

                        jal     _iop_load_frame
                        nop

                        lw      $ra, 0x00($sp)
                        nop
                        jr      $ra
                        addiu   $sp, $sp, 0x10
.end _iop_ex_cmn

.global _iop_ex_def_handler
.ent _iop_ex_def_handler
_iop_ex_def_handler:
                        nop
                        nop

___iop_ex_def_handler_start:
                        la      $k0, _iop_ex_def_frame

                        sw      $s0, 0x040($k0) // Save $s0
                        sw      $sp, 0x074($k0) // Save $sp
                        mfc0    $s0, IOP_COP0_DCIC
                        sw      $ra, 0x07C($k0) // Save $ra
                        lw      $sp, 0x410($zero) // k0 saved here by default exception prologue (at 0x80)
                        sw      $s0, 0x090($k0) // Save IOP_COP0_DCIC
                        sw      $sp, 0x068($k0) // Save $k0
                        lw      $at, 0x400($0) // at is saved here by default exception prologue (at 0x80)
                        la      $sp, _iop_ex_def_stack + _EX_DEF_STACK_SIZE

                        jal     _iop_ex_cmn
                        move    $s0, $k0

                        lw      $k0, 0x068($s0) // restore k0
                        lw      $sp, 0x074($s0) // restore sp
                        lw      $ra, 0x07C($s0) // restore ra

                        lw      $k1, 0x090($s0) // restore IOP_COP0_DCIC
                        nop
                        mtc0    $k1, IOP_COP0_DCIC
                        nop

                        lw      $k1, 0x0A8($s0)
                        nop
                        mtc0    $k1, $14 // EPC
                        lw      $s0, 0x040($s0) // restore s0
                        nop
                        jr      $k1
                        cop0    0x10
                        nop
                        nop
.end _iop_ex_def_handler

.global _iop_ex_break_handler
.ent _iop_ex_break_handler
_iop_ex_break_handler:
                        nop
                        nop
                        j       ___iop_ex_def_handler_start
                        nop
.end _iop_ex_break_handler

.global _iop_ex_dbg_handler
.ent _iop_ex_dbg_handler
_iop_ex_dbg_handler:
                        nop
                        nop

                        la      $k0, _iop_ex_dbg_frame

                        sw      $s0, 0x040($k0) // Save $s0
                        sw      $sp, 0x074($k0) // Save $sp
                        sw      $ra, 0x07C($k0) // Save $ra
                        lw      $sp, 0x420($zero) // k0 saved here by debug exception prologue (at 0x40)
                        lw      $s0, 0x430($0) // IOP_COP0_DCIC is saved here by debug exception prologue (at 0x40)
                        sw      $sp, 0x068($k0) // Save $k0
                        sw      $s0, 0x090($k0) // Save IOP_COP0_DCIC
                        la      $sp, _iop_ex_dbg_stack + _EX_DBG_STACK_SIZE

                        jal     _iop_ex_cmn
                        move    $s0, $k0

                        lw      $k0, 0x068($s0) // restore k0
                        lw      $sp, 0x074($s0) // restore sp
                        lw      $ra, 0x07C($s0) // restore ra

                        lw      $k1, 0x090($s0) // restore IOP_COP0_DCIC
                        nop
                        mtc0    $k1, IOP_COP0_DCIC
                        nop

                        lw      $k1, 0x0A8($s0)
                        nop
                        mtc0    $k1, $14 // EPC
                        lw      $s0, 0x040($s0) // restore s0
                        nop
                        jr      $k1
                        cop0    0x10
                        nop
                        nop
.end _iop_ex_dbg_handler

.set pop
