/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: ee_dbg_low.S $
#
# EEDEBUG - EE debugging library.
#
# ee_dbg_low.S - low-level EE "debug" helper functions.
#
*/


#include <ee_cop0_defs.h>
#include "eedebug_defs.h"

.text

.set push
.set noreorder
.set noat

.ent ee_dbg_get_bpc
.global ee_dbg_get_bpc
ee_dbg_get_bpc:
                        mfbpc   $v0
                        sync.p
                        jr      $ra
                        nop

.end ee_dbg_get_bpc

.global ee_dbg_get_iab
.ent ee_dbg_get_iab
ee_dbg_get_iab:
                        mfiab   $v0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_get_iab

.global ee_dbg_get_iabm
.ent ee_dbg_get_iabm
ee_dbg_get_iabm:
                        mfiabm   $v0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_get_iabm

.global ee_dbg_get_dab
.ent ee_dbg_get_dab
ee_dbg_get_dab:
                        mfdab   $v0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_get_dab

.global ee_dbg_get_dabm
.ent ee_dbg_get_dabm
ee_dbg_get_dabm:
                        mfdabm   $v0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_get_dabm

.global ee_dbg_get_dvb
.ent ee_dbg_get_dvb
ee_dbg_get_dvb:
                        mfdvb   $v0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_get_dvb

.global ee_dbg_get_dvbm
.ent ee_dbg_get_dvbm
ee_dbg_get_dvbm:
                        mfdvbm   $v0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_get_dvbm

.global ee_dbg_set_bpc
.ent ee_dbg_set_bpc
ee_dbg_set_bpc:
                        mtbpc   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_bpc

.global ee_dbg_set_iab
.ent ee_dbg_set_iab
ee_dbg_set_iab:
                        mtiab   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_iab

.global ee_dbg_set_iabm
.ent ee_dbg_set_iabm
ee_dbg_set_iabm:
                        mtiabm   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_iabm

.global ee_dbg_set_dab
.ent ee_dbg_set_dab
ee_dbg_set_dab:
                        mtdab   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_dab

.global ee_dbg_set_dabm
.ent ee_dbg_set_dabm
ee_dbg_set_dabm:
                        mtdabm   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_dabm

.global ee_dbg_set_dvb
.ent ee_dbg_set_dvb
ee_dbg_set_dvb:
                        mtdvb   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_dvb

.global ee_dbg_set_dvbm
.ent ee_dbg_set_dvbm
ee_dbg_set_dvbm:
                        mtdvbm   $a0
                        sync.p
                        jr      $ra
                        nop
.end ee_dbg_set_dvbm

.global _ee_dbg_set_bpda
.ent _ee_dbg_set_bpda
_ee_dbg_set_bpda:

                        mfbpc   $t0
                        li      $t1, (EE_BPC_BED)
                        mtbpc   $t1
                        sync.p

                        mtdab   $a0
                        mtdabm  $a1
                        sync.p

                        li      $t1, ~(EE_BPC_DRE | EE_BPC_DWE | EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE | EE_BPC_DTE | EE_BPC_DWB | EE_BPC_DRB)
                        and     $t0, $t0, $t1

                        li      $t1, (EE_BPC_DTE)
                        or      $t0, $t0, $t1
                        or      $t0, $t0, $a2
                        mtbpc   $t0
                        sync.p
                        jr      $ra
                        nop

.end _ee_dbg_set_bpda

.global _ee_dbg_set_bpdv
.ent _ee_dbg_set_bpdv
_ee_dbg_set_bpdv:

                        mfbpc   $t0
                        li      $t1, (EE_BPC_BED)
                        mtbpc   $t1
                        sync.p

                        mtdvb   $a0
                        mtdvbm  $a1
                        sync.p

                        li      $t1, (EE_BPC_DVE | EE_BPC_DTE)
                        or      $t0, $t0, $t1
                        or      $t0, $t0, $a2
                        mtbpc   $t0
                        sync.p
                        jr      $ra
                        nop

.end _ee_dbg_set_bpdv

.global _ee_dbg_set_bpx
.ent _ee_dbg_set_bpx
_ee_dbg_set_bpx:

                        mfbpc   $t0
                        li      $t1, (EE_BPC_BED)
                        mtbpc   $t1
                        sync.p

                        mtiab   $a0
                        mtiabm  $a1
                        sync.p

                        li      $t1, (EE_BPC_IAE | EE_BPC_ITE)
                        or      $t0, $t0, $t1
                        or      $t0, $t0, $a2
                        mtbpc   $t0
                        sync.p
                        jr      $ra
                        nop

.end _ee_dbg_set_bpx

.set pop
