/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: iop_dbg_low.S $
#
# IOPDEBUG - IOP debugging library.
#
# iop_dbg_low.S - low-level IOP "debug" helper functions.
#
*/


#include <iop_cop0_defs.h>
#include "iopdebug_defs.h"

.set push
.set noreorder
.set noat

.global iop_dbg_get_dcic
.ent iop_dbg_get_dcic
iop_dbg_get_dcic:
                        mfc0    $v0, IOP_COP0_DCIC
                        nop
                        jr      $ra
                        nop
.end iop_dbg_get_dcic

.global iop_dbg_get_bpc
.ent iop_dbg_get_bpc
iop_dbg_get_bpc:
                        mtc0    $v0, IOP_COP0_BPC
                        nop
                        jr      $ra
                        nop
.end iop_dbg_get_bpc

.global iop_dbg_get_bpcm
.ent iop_dbg_get_bpcm
iop_dbg_get_bpcm:
                        mfc0    $v0, IOP_COP0_BPCM
                        nop
                        jr      $ra
                        nop

.end iop_dbg_get_bpcm

.global iop_dbg_get_bda
.ent iop_dbg_get_bda
iop_dbg_get_bda:
                        mfc0    $v0, IOP_COP0_BDA
                        nop
                        jr      $ra
                        nop
.end iop_dbg_get_bda

.global iop_dbg_get_bdam
.ent iop_dbg_get_bdam
iop_dbg_get_bdam:
                        mfc0    $v0, IOP_COP0_BDAM
                        nop
                        jr      $ra
                        nop
.end iop_dbg_get_bdam

.global iop_dbg_set_dcic
.ent iop_dbg_set_dcic
iop_dbg_set_dcic:
                        mtc0    $a0, IOP_COP0_DCIC
                        nop
                        jr      $ra
                        nop
.end iop_dbg_set_dcic

.global iop_dbg_set_bpc
.ent iop_dbg_set_bpc
iop_dbg_set_bpc:
                        mtc0    $a0, IOP_COP0_BPC
                        nop
                        jr      $ra
                        nop
.end iop_dbg_set_bpc

.global iop_dbg_set_bpcm
.ent iop_dbg_set_bpcm
iop_dbg_set_bpcm:
                        mtc0    $a0, IOP_COP0_BPCM
                        nop
                        jr      $ra
                        nop
.end iop_dbg_set_bpcm


.global iop_dbg_set_bda
.ent iop_dbg_set_bda
iop_dbg_set_bda:
                        mtc0    $a0, IOP_COP0_BDA
                        nop
                        jr      $ra
                        nop
.end iop_dbg_set_bda

.global iop_dbg_set_bdam
.ent iop_dbg_set_bdam
iop_dbg_set_bdam:
                        mtc0    $a0, IOP_COP0_BDAM
                        nop
                        jr      $ra
                        nop
.end iop_dbg_set_bdam

.global _iop_dbg_set_bpda
.ent _iop_dbg_set_bpda
_iop_dbg_set_bpda:

                        mfc0    $t0, IOP_COP0_DCIC
                        nop
                        nop
                        mtc0    $zero, IOP_COP0_DCIC
                        nop
                        nop

                        mtc0    $a0, IOP_COP0_BDA
                        mtc0    $a1, IOP_COP0_BDAM

                        li      $t1, ~(IOP_DCIC_DR | IOP_DCIC_DW)
                        and     $t0, $t0, $t1

                        li      $t1, (IOP_DCIC_TR | IOP_DCIC_DAE | IOP_DCIC_DE)
                        or      $t0, $t0, $t1
                        or      $t0, $t0, $a2
                        mtc0    $t0, IOP_COP0_DCIC
                        nop
                        jr      $ra
                        nop

.end _iop_dbg_set_bpda

.global _iop_dbg_set_bpx
.ent _iop_dbg_set_bpx
_iop_dbg_set_bpx:

                        mfc0    $t0, IOP_COP0_DCIC
                        nop
                        nop
                        mtc0    $zero, IOP_COP0_DCIC
                        nop
                        nop

                        mtc0    $a0, IOP_COP0_BPC
                        mtc0    $a1, IOP_COP0_BPCM
                        nop

                        li      $t1, ~(IOP_DCIC_PC)
                        and     $t0, $t0, $t1

                        li      $t1, (IOP_DCIC_TR | IOP_DCIC_PCE | IOP_DCIC_DE)
                        or      $t0, $t0, $t1
                        or      $t0, $t0, $a2
                        mtc0    $t0, IOP_COP0_DCIC
                        nop
                        jr      $ra
                        nop

.end _iop_dbg_set_bpx

.set pop
