# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


.set push
.set noreorder
.set nomacro
.set noat

.globl _MPEG_put_block_fr
.globl _MPEG_put_block_fl
.globl _MPEG_put_block_il
.globl _MPEG_add_block_frfr
.globl _MPEG_add_block_ilfl
.globl _MPEG_add_block_frfl
.globl _MPEG_put_luma
.globl _MPEG_put_chroma
.globl _MPEG_put_luma_X
.globl _MPEG_put_chroma_X
.globl _MPEG_put_luma_Y
.globl _MPEG_put_chroma_Y
.globl _MPEG_put_luma_XY
.globl _MPEG_put_chroma_XY
.globl _MPEG_avg_luma
.globl _MPEG_avg_chroma
.globl _MPEG_avg_luma_X
.globl _MPEG_avg_chroma_X
.globl _MPEG_avg_luma_Y
.globl _MPEG_avg_chroma_Y
.globl _MPEG_avg_luma_XY
.globl _MPEG_avg_chroma_XY

_MPEG_put_block_fr:
    lw      $a2, 0($a0)
    lw      $a3, 8($a0)
    pnor    $v0, $zero, $zero
    addiu   $v1, $zero, 6
    psrlh   $v0, $v0, 8
1:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1;
    lq      $t0,  64($a3)
    lq      $t1,  80($a3)
    lq      $t2,  96($a3)
    lq      $t3, 112($a3)
    addiu   $a3, $a3, 128
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $a4,  0($a2)
    sq      $a6, 16($a2)
    sq      $t0, 32($a2)
    sq      $t2, 48($a2)
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    jr      $ra
    nop

_MPEG_put_block_fl:
    pnor    $v0, $zero, $zero
    lw      $a2, 0($a0)
    lw      $a3, 8($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
1:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t0, 256($a3)
    lq      $t1, 272($a3)
    lq      $t2, 288($a3)
    lq      $t3, 304($a3)
    addiu   $a3, $a3, 64
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $a4,  0($a2)
    sq      $t0, 16($a2)
    sq      $a6, 32($a2)
    sq      $t2, 48($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 64
    addiu   $v1, $v1, 2
2:
    lq      $a4, 256($a3)
    lq      $a5, 272($a3)
    lq      $a6, 288($a3)
    lq      $a7, 304($a3)
    addiu   $v1, $v1, -1
    lq      $t0, 320($a3)
    lq      $t1, 336($a3)
    lq      $t2, 352($a3)
    lq      $t3, 368($a3)
    addiu   $a3, $a3, 128
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $a4,  0($a2)
    sq      $a6, 16($a2)
    sq      $t0, 32($a2)
    sq      $t2, 48($a2)
    bgtzl   $v1, 2b
    addiu   $a2, $a2, 64
    jr      $ra
    nop

_MPEG_put_block_il:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3,  8($a0)
    lw      $at, 24($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
    addu    $at, $at, $a2
1:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t0, 256($a3)
    lq      $t1, 272($a3)
    lq      $t2, 288($a3)
    lq      $t3, 304($a3)
    addiu   $a3, $a3, 64
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $a4,  0($a2)
    sq      $a6, 32($a2)
    addiu   $a2, $a2, 64
    sq      $t0,  0($at)
    sq      $t2, 32($at)
    bgtzl   $v1, 1b
    addiu   $at, $at, 64
    lw      $a2,  4($a0)
    lw      $at, 24($a0)
    addiu   $v1, $zero, 2
    addu    $at, $at, $a2
2:
    lq      $a4, 256($a3)
    lq      $a5, 272($a3)
    lq      $a6, 288($a3)
    lq      $a7, 304($a3)
    addiu   $v1, $v1, -1
    lq      $t0, 320($a3)
    lq      $t1, 336($a3)
    lq      $t2, 352($a3)
    lq      $t3, 368($a3)
    addiu   $a3, $a3, 128
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $a4, $zero, $a4
    ppacb   $a5, $zero, $a5
    ppacb   $a6, $zero, $a6
    ppacb   $a7, $zero, $a7
    ppacb   $t0, $zero, $t0
    ppacb   $t1, $zero, $t1
    ppacb   $t2, $zero, $t2
    ppacb   $t3, $zero, $t3
    sd      $a4,  0($a2)
    sd      $a5, 16($a2)
    sd      $a6, 32($a2)
    sd      $a7, 48($a2)
    sd      $t0,  0($at)
    sd      $t1, 16($at)
    sd      $t2, 32($at)
    sd      $t3, 48($at)
    addiu   $a2, $a2, 64
    bgtzl   $v1, 2b
    addiu   $at, $at, 64
    jr      $ra
    nop

_MPEG_add_block_frfr:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3, 12($a0)
    lw      $a0, 16($a0)
    addiu   $v1, $zero, 6
    psrlh   $v0, $v0, 8
1:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t0,   0($a0)
    lq      $t1,  16($a0)
    lq      $t2,  32($a0)
    lq      $t3,  48($a0)
    paddh   $a4, $a4, $t0
    paddh   $a5, $a5, $t1
    paddh   $a6, $a6, $t2
    paddh   $a7, $a7, $t3
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    sq      $a4,  0($a2)
    sq      $a6, 16($a2)
    lq      $t0,  64($a3)
    lq      $t1,  80($a3)
    lq      $t2,  96($a3)
    lq      $t3, 112($a3)
    addiu   $a3, $a3, 128
    lq      $a4,  64($a0)
    lq      $a5,  80($a0)
    lq      $a6,  96($a0)
    lq      $a7, 112($a0)
    addiu   $a0, $a0, 128
    paddh   $t0, $t0, $a4
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t3, $t3, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $t0, 32($a2)
    sq      $t2, 48($a2)
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    jr      $ra
    nop

_MPEG_add_block_ilfl:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3, 12($a0)
    lw      $at, 24($a0)
    lw      $a1, 16($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
    addu    $at, $at, $a2
1:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t0,   0($a1)
    lq      $t1,  16($a1)
    lq      $t2,  32($a1)
    lq      $t3,  48($a1)
    paddh   $a4, $a4, $t0
    paddh   $a5, $a5, $t1
    paddh   $a6, $a6, $t2
    paddh   $a7, $a7, $t3
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    sq      $a4,   0($a2)
    sq      $a6,  32($a2)
    lq      $t0, 256($a3)
    lq      $t1, 272($a3)
    lq      $t2, 288($a3)
    lq      $t3, 304($a3)
    addiu   $a3, $a3, 64
    lq      $a4, 256($a1)
    lq      $a5, 272($a1)
    lq      $a6, 288($a1)
    lq      $a7, 304($a1)
    paddh   $t0, $t0, $a4
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t3, $t3, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $t0,  0($at)
    sq      $t2, 32($at)
    addiu   $at, $at, 64
    addiu   $a1, $a1, 64
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    lw      $a2,  4($a0)
    lw      $at, 24($a0)
    addiu   $v1, $zero, 2
    addu    $at, $at, $a2
2:
    lq      $a4, 256($a3)
    lq      $a5, 272($a3)
    lq      $a6, 288($a3)
    lq      $a7, 304($a3)
    addiu   $v1, $v1, -1
    lq      $t0, 256($a1)
    lq      $t1, 272($a1)
    lq      $t2, 288($a1)
    lq      $t3, 304($a1)
    paddh   $a4, $a4, $t0
    paddh   $a5, $a5, $t1
    paddh   $a6, $a6, $t2
    paddh   $a7, $a7, $t3
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    ppacb   $a4, $zero, $a4
    ppacb   $a5, $zero, $a5
    ppacb   $a6, $zero, $a6
    ppacb   $a7, $zero, $a7
    sd      $a4,  0($a2)
    sd      $a5, 16($a2)
    sd      $a6, 32($a2)
    sd      $a7, 48($a2)
    lq      $t0, 320($a3)
    lq      $t1, 336($a3)
    lq      $t2, 352($a3)
    lq      $t3, 368($a3)
    addiu   $a3, $a3, 128
    lq      $a4, 320($a1)
    lq      $a5, 336($a1)
    lq      $a6, 352($a1)
    lq      $a7, 368($a1)
    paddh   $t0, $t0, $a4
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t3, $t3, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $t0, $zero, $t0
    ppacb   $t1, $zero, $t1
    ppacb   $t2, $zero, $t2
    ppacb   $t3, $zero, $t3
    sd      $t0,  0($at)
    sd      $t1, 16($at)
    sd      $t2, 32($at)
    sd      $t3, 48($at)
    addiu   $a2, $a2, 64
    addiu   $at, $at, 64
    bgtzl   $v1, 2b
    addiu   $a1, $a1, 128
    jr      $ra
    nop

_MPEG_add_block_frfl:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3, 12($a0)
    lw      $a1, 16($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
1:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t0,   0($a1)
    lq      $t1,  16($a1)
    lq      $t2, 256($a1)
    lq      $t3, 272($a1)
    paddh   $a4, $a4, $t0
    paddh   $a5, $a5, $t1
    paddh   $a6, $a6, $t2
    paddh   $a7, $a7, $t3
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    sq      $a4,   0($a2)
    sq      $a6,  16($a2)
    lq      $t0,  64($a3)
    lq      $t1,  80($a3)
    lq      $t2,  96($a3)
    lq      $t3, 112($a3)
    addiu   $a3, $a3, 128
    lq      $a4,  32($a1)
    lq      $a5,  48($a1)
    lq      $a6, 288($a1)
    lq      $a7, 304($a1)
    paddh   $t0, $t0, $a4
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t3, $t3, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $t0, 32($a2)
    sq      $t2, 48($a2)
    addiu   $a1, $a1, 64
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    lw      $a2, 4($a0)
    addiu   $v1, $zero, 2
2:
    lq      $a4,   0($a3)
    lq      $a5,  16($a3)
    lq      $a6,  32($a3)
    lq      $a7,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t0, 256($a1)
    lq      $t1, 320($a1)
    lq      $t2, 272($a1)
    lq      $t3, 336($a1)
    paddh   $a4, $a4, $t0
    paddh   $a5, $a5, $t1
    paddh   $a6, $a6, $t2
    paddh   $a7, $a7, $t3
    pmaxh   $a4, $zero, $a4
    pmaxh   $a5, $zero, $a5
    pmaxh   $a6, $zero, $a6
    pmaxh   $a7, $zero, $a7
    pminh   $a4, $v0, $a4
    pminh   $a5, $v0, $a5
    pminh   $a6, $v0, $a6
    pminh   $a7, $v0, $a7
    ppacb   $a4, $a5, $a4
    ppacb   $a6, $a7, $a6
    sq      $a4,  0($a2)
    sq      $a6, 16($a2)
    lq      $t0,  64($a3)
    lq      $t1,  80($a3)
    lq      $t2,  96($a3)
    lq      $t3, 112($a3)
    addiu   $a3, $a3, 128
    lq      $a4, 288($a1)
    lq      $a5, 352($a1)
    lq      $a6, 304($a1)
    lq      $a7, 368($a1)
    paddh   $t0, $t0, $a4
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t3, $t3, $a7
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    sq      $t0, 32($a2)
    sq      $t2, 48($a2)
    addiu   $a2, $a2, 64
    bgtzl   $v1, 2b
    addiu   $a1, $a1, 128
    jr      $ra
    nop

_MPEG_put_luma:
    mtsab   $a3, 0
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $t1, $t2, $t1
    pextlb  $t2, $zero, $t1
    pextub  $t1, $zero, $t1
    sq      $t2,  0($a2)
    sq      $t1, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_chroma:
    mtsab   $a3, 0
1:
    ld      $t1,   0($a1)
    ld      $t2,  64($a1)
    ld      $t3, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    pcpyld  $t1, $t3, $t1
    pcpyld  $t2, $t8, $t2
    qfsrv   $t1, $t1, $t1
    qfsrv   $t2, $t2, $t2
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    sq      $t1,   0($a2)
    sq      $t2, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_luma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    mtsab   $a3, 0
    qfsrv   $t3, $t2, $t1
    qfsrv   $t8, $t1, $t2
    pextlb  $t1, $zero, $t3
    pextub  $t2, $zero, $t3
    addu    $a1, $a1, $a7
    mtsab   $zero, 1
    addiu   $v1, $v1, -1
    qfsrv   $t8, $t8, $t3
    pextlb  $t3, $zero, $t8
    pextub  $t8, $zero, $t8
    paddh   $t1, $t1, $t3
    paddh   $t2, $t2, $t8
    paddh   $t1, $t1, $v0
    paddh   $t2, $t2, $v0
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,  0($a2)
    sq      $t2, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_chroma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    ld      $t1,   0($a1)
    ld      $t2,  64($a1)
    ld      $t3, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t1, $t3, $t1
    pcpyld  $t2, $t8, $t2
    mtsab   $a3, 0
    qfsrv   $t1, $t1, $t1
    qfsrv   $t2, $t2, $t2
    addiu   $t9, $zero, 1
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    mtsab   $t9, 0
    qfsrv   $a5, $t1, $t1
    qfsrv   $a6, $t2, $t2
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    pextlb  $a5, $zero, $a5
    pextlb  $a6, $zero, $a6
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t1, $t1, $v0
    paddh   $t2, $t2, $v0
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,   0($a2)
    sq      $t2, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_luma_Y:
    mtsab   $a3, 0
    lq      $t3,   0($a1)
    lq      $t8, 384($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $t3, $t8, $t3
    pextub  $t8, $zero, $t3
    pextlb  $t3, $zero, $t3
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $t1, $t2, $t1
    pextub  $t2, $zero, $t1
    pextlb  $t1, $zero, $t1
    paddh   $v0, $t2, $t8
    pnor    $t8, $zero, $zero
    paddh   $t9, $t1, $t3
    psrlh   $t8, $t8, 15
    por     $t3, $zero, $t1
    paddh   $t9, $t9, $t8
    paddh   $v0, $v0, $t8
    por     $t8, $zero, $t2
    psrlh   $t9, $t9, 1
    psrlh   $v0, $v0, 1
    sq      $t9,  0($a2)
    sq      $v0, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_chroma_Y:
    mtsab   $a3, 0
    ld      $a0,   0($a1)
    ld      $a3,  64($a1)
    ld      $a4, 384($a1)
    ld      $a5, 448($a1)
    pnor    $v0, $zero, $zero
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    psrlh   $v0, $v0, 15
    pcpyld  $a0, $a4, $a0
    pcpyld  $a3, $a5, $a3
    qfsrv   $a0, $a0, $a0
    qfsrv   $a3, $a3, $a3
    pextlb  $a0, $zero, $a0
    pextlb  $a3, $zero, $a3
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t1,   0($a1)
    ld      $t2,  64($a1)
    ld      $t3, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    pcpyld  $t1, $t3, $t1
    pcpyld  $t2, $t8, $t2
    qfsrv   $t1, $t1, $t1
    qfsrv   $t2, $t2, $t2
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    paddh   $a5, $t1, $a0
    paddh   $a6, $t2, $a3
    por     $a0, $zero, $t1
    por     $a3, $zero, $t2
    paddh   $a5, $a5, $v0
    paddh   $a6, $a6, $v0
    psrlh   $a5, $a5, 1
    psrlh   $a6, $a6, 1
    sq      $a5,   0($a2)
    sq      $a6, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_luma_XY:
    mtsab   $a3, 0
    lq      $v0,   0($a1)
    lq      $t3, 384($a1)
    addu    $a1, $a1, $a7
    qfsrv   $t8, $t3, $v0
    qfsrv   $t9, $v0, $t3
    addiu   $v1, $v1, -1
    pextlb  $v0, $zero, $t8
    pextub  $t3, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $v0, $v0, $t8
    paddh   $t3, $t3, $t9
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    mtsab   $a3, 0
    addu    $a1, $a1, $a7
    qfsrv   $t8, $t2, $t1
    qfsrv   $t9, $t1, $t2
    addiu   $v1, $v1, -1
    pextlb  $t1, $zero, $t8
    pextub  $t2, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $t1, $t1, $t8
    paddh   $t2, $t2, $t9
    paddh   $t8, $v0, $t1
    paddh   $t9, $t3, $t2
    por     $v0, $zero, $t1
    pnor    $t1, $zero, $zero
    por     $t3, $zero, $t2
    psrlh   $t1, $t1, 15
    psllh   $t1, $t1,  1
    paddh   $t8, $t8, $t1
    paddh   $t9, $t9, $t1
    psrlh   $t8, $t8, 2
    psrlh   $t9, $t9, 2
    sq      $t8,  0($a2)
    sq      $t9, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_put_chroma_XY:
    mtsab   $a3, 0
    pnor    $t9, $zero, $zero
    ld      $a0,   0($a1)
    ld      $v0,  64($a1)
    mtsab   $zero, 1
    ld      $a4, 384($a1)
    ld      $a5, 448($a1)
    pcpyld  $a0, $a4, $a0
    pcpyld  $v0, $a5, $v0
    qfsrv   $a0, $a0, $a0
    qfsrv   $v0, $v0, $v0
    psrlh   $t9, $t9, 15
    psllh   $t9, $t9, 1
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $a4, $a0, $a0
    qfsrv   $a5, $v0, $v0
    pextlb  $a0, $zero, $a0
    pextlb  $v0, $zero, $v0
    pextlb  $a4, $zero, $a4
    pextlb  $a5, $zero, $a5
    paddh   $a0, $a0, $a4
    paddh   $a4, $v0, $a5
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t1,   0($a1)
    ld      $t3,  64($a1)
    mtsab   $a3, 0
    ld      $t2, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t1, $t2, $t1
    pcpyld  $t3, $t8, $t3
    qfsrv   $t1, $t1, $t1
    qfsrv   $t3, $t3, $t3
    addiu   $v0, $zero, 1
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    mtsab   $v0, 0
    qfsrv   $t2, $t1, $t1
    qfsrv   $t8, $t3, $t3
    pextlb  $t1, $zero, $t1
    pextlb  $t3, $zero, $t3
    pextlb  $t2, $zero, $t2
    pextlb  $t8, $zero, $t8
    paddh   $t1, $t1, $t2
    paddh   $t2, $t3, $t8
    paddh   $t3, $a0, $t1
    paddh   $t8, $a4, $t2
    por     $a0, $zero, $t1
    por     $a4, $zero, $t2
    paddh   $t3, $t3, $t9
    paddh   $t8, $t8, $t9
    psrlh   $t3, $t3, 2
    psrlh   $t8, $t8, 2
    sq      $t3,   0($a2)
    sq      $t8, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_luma:
    mtsab   $a3, 0
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $t1, $t2, $t1
    pextlb  $t2, $zero, $t1
    pextub  $t1, $zero, $t1
    lq      $t8,  0($a2)
    lq      $t9, 16($a2)
    paddh   $t2, $t2, $t8
    paddh   $t1, $t1, $t9
    pcgth   $t8, $t2, $zero
    pcgth   $t9, $t1, $zero
    pceqh   $v0, $t2, $zero
    pceqh   $t3, $t1, $zero
    psrlh   $t8, $t8, 15
    psrlh   $t9, $t9, 15
    psrlh   $v0, $v0, 15
    psrlh   $t3, $t3, 15
    por     $t8, $t8, $v0
    por     $t9, $t9, $t3
    paddh   $t2, $t2, $t8
    paddh   $t1, $t1, $t9
    psrlh   $t2, $t2, 1
    psrlh   $t1, $t1, 1
    sq      $t2,  0($a2)
    sq      $t1, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_chroma:
    mtsab   $a3, 0
1:
    ld      $t1,   0($a1)
    ld      $t2,  64($a1)
    addiu   $v1, $v1, -1
    ld      $t3, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $a7
    pcpyld  $t1, $t3, $t1
    pcpyld  $t2, $t8, $t2
    qfsrv   $t1, $t1, $t1
    qfsrv   $t2, $t2, $t2
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    lq      $a4,   0($a2)
    lq      $a5, 128($a2)
    paddh   $t1, $t1, $a4
    paddh   $t2, $t2, $a5
    pcgth   $a4, $t1, $zero
    pcgth   $a5, $t2, $zero
    pceqh   $v0, $t1, $zero
    pceqh   $t9, $t2, $zero
    psrlh   $a4, $a4, 15
    psrlh   $a5, $a5, 15
    psrlh   $v0, $v0, 15
    psrlh   $t9, $t9, 15
    por     $a4, $a4, $v0
    por     $a5, $a5, $t9
    paddh   $t1, $t1, $a4
    paddh   $t2, $t2, $a5
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,   0($a2)
    sq      $t2, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_luma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    mtsab   $a3, 0
    qfsrv   $t3, $t2, $t1
    qfsrv   $t8, $t1, $t2
    pextlb  $t1, $zero, $t3
    pextub  $t2, $zero, $t3
    addu    $a1, $a1, $a7
    mtsab   $zero, 1
    addiu   $v1, $v1, -1
    qfsrv   $t8, $t8, $t3
    pextlb  $t3, $zero, $t8
    pextub  $t8, $zero, $t8
    paddh   $t1, $t1, $t3
    paddh   $t2, $t2, $t8
    paddh   $t1, $t1, $v0
    paddh   $t2, $t2, $v0
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    lq      $t8,  0($a2)
    lq      $t9, 16($a2)
    paddh   $t1, $t1, $t8
    paddh   $t2, $t2, $t9
    pcgth   $t8, $t1, $zero
    pceqh   $t9, $t1, $zero
    psrlh   $t8, $t8, 15
    psrlh   $t9, $t9, 15
    por     $t8, $t8, $t9
    paddh   $t1, $t1, $t8
    pcgth   $t8, $t2, $zero
    pceqh   $t9, $t2, $zero
    psrlh   $t8, $t8, 15
    psrlh   $t9, $t9, 15
    por     $t8, $t8, $t9
    paddh   $t2, $t2, $t8
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,  0($a2)
    sq      $t2, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_chroma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    ld      $t1,   0($a1)
    ld      $t2,  64($a1)
    mtsab   $a3, 0
    ld      $t3, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t1, $t3, $t1
    pcpyld  $t2, $t8, $t2
    qfsrv   $t1, $t1, $t1
    qfsrv   $t2, $t2, $t2
    addiu   $t9, $zero, 1
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    mtsab   $t9, 0
    qfsrv   $a5, $t1, $t1
    qfsrv   $a6, $t2, $t2
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    pextlb  $a5, $zero, $a5
    pextlb  $a6, $zero, $a6
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    paddh   $t1, $t1, $v0
    paddh   $t2, $t2, $v0
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    lq      $a5,   0($a2)
    lq      $a6, 128($a2)
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    pcgth   $a5, $t1, $zero
    pcgth   $a6, $t2, $zero
    pceqh   $t9, $t1, $zero
    pceqh   $a0, $t2, $zero
    psrlh   $a5, $a5, 15
    psrlh   $a6, $a6, 15
    psrlh   $t9, $t9, 15
    psrlh   $a0, $a0, 15
    por     $a5, $a5, $t9
    por     $a6, $a6, $a0
    paddh   $t1, $t1, $a5
    paddh   $t2, $t2, $a6
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,   0($a2)
    sq      $t2, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_luma_Y:
    mtsab   $a3, 0
    lq      $t3,   0($a1)
    lq      $t8, 384($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $t3, $t8, $t3
    pextub  $t8, $zero, $t3
    pextlb  $t3, $zero, $t3
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $t1, $t2, $t1
    pextub  $t2, $zero, $t1
    pextlb  $t1, $zero, $t1
    paddh   $v0, $t2, $t8
    pnor    $t8, $zero, $zero
    paddh   $t9, $t1, $t3
    psrlh   $t8, $t8, 15
    por     $t3, $zero, $t1
    paddh   $t9, $t9, $t8
    paddh   $v0, $v0, $t8
    por     $t8, $zero, $t2
    psrlh   $t9, $t9, 1
    psrlh   $v0, $v0, 1
    lq      $t1,  0($a2)
    lq      $t2, 16($a2)
    paddh   $t9, $t9, $t1
    paddh   $v0, $v0, $t2
    pcgth   $t1, $t9, $zero
    pceqh   $t2, $t9, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $t9, $t9, $t1
    pcgth   $t1, $v0, $zero
    pceqh   $t2, $v0, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $v0, $v0, $t1
    psrlh   $t9, $t9, 1
    psrlh   $v0, $v0, 1
    sq      $t9,  0($a2)
    sq      $v0, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_chroma_Y:
    mtsab   $a3, 0
    ld      $a0,   0($a1)
    ld      $a3,  64($a1)
    ld      $a4, 384($a1)
    ld      $a5, 448($a1)
    pnor    $v0, $zero, $zero
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    psrlh   $v0, $v0, 15
    pcpyld  $a0, $a4, $a0
    pcpyld  $a3, $a5, $a3
    qfsrv   $a0, $a0, $a0
    qfsrv   $a3, $a3, $a3
    pextlb  $a0, $zero, $a0
    pextlb  $a3, $zero, $a3
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t1,   0($a1)
    ld      $t2,  64($a1)
    addiu   $v1, $v1, -1
    ld      $t3, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $a7
    pcpyld  $t1, $t3, $t1
    pcpyld  $t2, $t8, $t2
    qfsrv   $t1, $t1, $t1
    qfsrv   $t2, $t2, $t2
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    paddh   $a5, $t1, $a0
    paddh   $a6, $t2, $a3
    por     $a0, $zero, $t1
    por     $a3, $zero, $t2
    paddh   $a5, $a5, $v0
    paddh   $a6, $a6, $v0
    psrlh   $a5, $a5, 1
    psrlh   $a6, $a6, 1
    lq      $t1,   0($a2)
    lq      $t2, 128($a2)
    paddh   $a5, $a5, $t1
    paddh   $a6, $a6, $t2
    pcgth   $t1, $a5, $zero
    pceqh   $t2, $a5, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $a5, $a5, $t1
    pcgth   $t1, $a6, $zero
    pceqh   $t2, $a6, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $a6, $a6, $t1
    psrlh   $a5, $a5, 1
    psrlh   $a6, $a6, 1
    sq      $a5,   0($a2)
    sq      $a6, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_luma_XY:
    mtsab   $a3, 0
    lq      $v0,   0($a1)
    lq      $t3, 384($a1)
    addu    $a1, $a1, $a7
    qfsrv   $t8, $t3, $v0
    qfsrv   $t9, $v0, $t3
    addiu   $v1, $v1, -1
    pextlb  $v0, $zero, $t8
    pextub  $t3, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $v0, $v0, $t8
    paddh   $t3, $t3, $t9
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t1,   0($a1)
    lq      $t2, 384($a1)
    mtsab   $a3, 0
    addu    $a1, $a1, $a7
    qfsrv   $t8, $t2, $t1
    qfsrv   $t9, $t1, $t2
    addiu   $v1, $v1, -1
    pextlb  $t1, $zero, $t8
    pextub  $t2, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $t1, $t1, $t8
    paddh   $t2, $t2, $t9
    paddh   $t8, $v0, $t1
    paddh   $t9, $t3, $t2
    por     $v0, $zero, $t1
    pnor    $t1, $zero, $zero
    por     $t3, $zero, $t2
    psrlh   $t1, $t1, 15
    psllh   $t1, $t1,  1
    paddh   $t8, $t8, $t1
    paddh   $t9, $t9, $t1
    psrlh   $t8, $t8, 2
    psrlh   $t9, $t9, 2
    lq      $t1,  0($a2)
    lq      $t2, 16($a2)
    paddh   $t8, $t8, $t1
    paddh   $t9, $t9, $t2
    pcgth   $t1, $t8, $zero
    pceqh   $t2, $t8, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $t8, $t8, $t1
    pcgth   $t1, $t9, $zero
    pceqh   $t2, $t9, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $t9, $t9, $t1
    psrlh   $t8, $t8, 1
    psrlh   $t9, $t9, 1
    sq      $t8,  0($a2)
    sq      $t9, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

_MPEG_avg_chroma_XY:
    mtsab   $a3, 0
    pnor    $t9, $zero, $zero
    ld      $a0,   0($a1)
    ld      $v0,  64($a1)
    mtsab   $zero, 1
    ld      $a4, 384($a1)
    ld      $a5, 448($a1)
    pcpyld  $a0, $a4, $a0
    pcpyld  $v0, $a5, $v0
    qfsrv   $a0, $a0, $a0
    qfsrv   $v0, $v0, $v0
    psrlh   $t9, $t9, 15
    psllh   $t9, $t9,  1
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    qfsrv   $a4, $a0, $a0
    qfsrv   $a5, $v0, $v0
    pextlb  $a0, $zero, $a0
    pextlb  $v0, $zero, $v0
    pextlb  $a4, $zero, $a4
    pextlb  $a5, $zero, $a5
    paddh   $a0, $a0, $a4
    paddh   $a4, $v0, $a5
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t1,   0($a1)
    ld      $t3,  64($a1)
    mtsab   $a3, 0
    ld      $t2, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t1, $t2, $t1
    pcpyld  $t3, $t8, $t3
    qfsrv   $t1, $t1, $t1
    qfsrv   $t3, $t3, $t3
    addiu   $v0, $zero, 1
    addu    $a1, $a1, $a7
    addiu   $v1, $v1, -1
    mtsab   $v0, 0
    qfsrv   $t2, $t1, $t1
    qfsrv   $t8, $t3, $t3
    pextlb  $t1, $zero, $t1
    pextlb  $t3, $zero, $t3
    pextlb  $t2, $zero, $t2
    pextlb  $t8, $zero, $t8
    paddh   $t1, $t1, $t2
    paddh   $t2, $t3, $t8
    paddh   $t3, $a0, $t1
    paddh   $t8, $a4, $t2
    por     $a0, $zero, $t1
    por     $a4, $zero, $t2
    paddh   $t3, $t3, $t9
    paddh   $t8, $t8, $t9
    psrlh   $t3, $t3, 2
    psrlh   $t8, $t8, 2
    lq      $t1,   0($a2)
    lq      $t2, 128($a2)
    paddh   $t3, $t3, $t1
    paddh   $t8, $t8, $t2
    pcgth   $t1, $t3, $zero
    pceqh   $t2, $t3, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $t3, $t3, $t1
    pcgth   $t1, $t8, $zero
    pceqh   $t2, $t8, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    por     $t1, $t1, $t2
    paddh   $t8, $t8, $t1
    psrlh   $t3, $t3, 1
    psrlh   $t8, $t8, 1
    sq      $t3,   0($a2)
    sq      $t8, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra
    nop

    .globl _MPEG_do_mc
_MPEG_do_mc:
    addiu   $v0, $zero, 16
    lw      $a1,  0($a0)
    addiu   $sp, $sp, -16
    lw      $a2,  4($a0)
    lw      $a3, 12($a0)
    lw      $a4, 16($a0)
    lw      $a5, 20($a0)
    lw      $a6, 24($a0)
    lw      $t0, 28($a0)
    subu    $a4, $a4, $t0
    lw      $t1, 32($a0)
    sll     $t0, $t0, 4
    addu    $a1, $a1, $t0
    subu    $v1, $v0, $a4
    sllv    $a7, $v0, $a6
    srlv    $v1, $v1, $a6
    sll     $at, $a4, 4
    sw      $ra, 0($sp)
    addu    $a1, $a1, $at
    jalr    $t1
    subu    $at, $a5, $v1
    lw      $a1,  0($a0)
    lw      $a2,  8($a0)
    lw      $t1, 36($a0)
    addiu   $a1, $a1, 256
    srl     $t0, $t0, 1
    srl     $a3, $a3, 1
    srl     $a4, $a4, 1
    srl     $a5, $a5, 1
    lw      $ra, 0($sp)
    srlv    $a4, $a4, $a6
    addu    $a1, $a1, $t0
    addiu   $v0, $zero, 8
    sllv    $a4, $a4, $a6
    subu    $v1, $v0, $a4
    sllv    $a7, $v0, $a6
    srlv    $v1, $v1, $a6
    sll     $at, $a4, 3
    addu    $a1, $a1, $at
    subu    $at, $a5, $v1
    jr      $t1
    addiu   $sp, $sp, 16

.set pop
