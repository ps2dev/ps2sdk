# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
.set noreorder
.set nomacro
.set noat

.globl _MPEG_Initialize
.globl _MPEG_Destroy
.globl _MPEG_GetBits
.globl _MPEG_ShowBits
.globl _MPEG_AlignBits
.globl _MPEG_NextStartCode
.globl _MPEG_SetDefQM
.globl _MPEG_SetQM
.globl _MPEG_GetMBAI
.globl _MPEG_GetMBType
.globl _MPEG_GetMotionCode
.globl _MPEG_GetDMVector
.globl _MPEG_SetIDCP
.globl _MPEG_SetQSTIVFAS
.globl _MPEG_SetPCT
.globl _MPEG_BDEC
.globl _MPEG_WaitBDEC
.globl _MPEG_put_block_fr
.globl _MPEG_put_block_fl
.globl _MPEG_put_block_il
.globl _MPEG_add_block_frfr
.globl _MPEG_add_block_ilfl
.globl _MPEG_add_block_frfl
.globl _MPEG_dma_ref_image
.globl _MPEG_do_mc
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
.globl _MPEG_CSCImage
.globl _MPEG_Suspend
.globl _MPEG_Resume

.sdata
.align 4
s_DefQM:    .word 0x13101008, 0x16161310, 0x16161616, 0x1B1A181A
            .word 0x1A1A1B1B, 0x1B1B1A1A, 0x1D1D1D1B, 0x1D222222
            .word 0x1B1B1D1D, 0x20201D1D, 0x26252222, 0x22232325
            .word 0x28262623, 0x30302828, 0x38382E2E, 0x5345453A
            .word 0x10101010, 0x10101010, 0x10101010, 0x10101010

.section ".sbss"
.align 6
s_DMAPack : .space 128
s_DataBuf : .space   8
s_SetDMA  : .space   8
s_IPUState: .space  32
s_pEOF    : .space   4
s_Sema    : .space   4
s_CSCParam: .space  12
s_CSCID   : .space   4
s_CSCFlag : .space   1

.text

_MPEG_Initialize:
    addiu   $sp, $sp, -48
    lui     $v0, 0x1000
    lui     $v1, 0x4000
    sw      $a1, s_SetDMA + 0
    sw      $v1, 0x2010($v0)
    sw      $a2, s_SetDMA + 4
    sw      $a3, s_pEOF
1:
    lw      $v1, 0x2010($v0)
    bltz    $v1, 1b
    nop
    sw      $zero, 0x2000($v0)
1:
    lw      $v1, 0x2010($v0)
    bltz    $v1, 1b
    nop
    lui     $at, 0x0080
    sw      $ra, 0($sp)
    or      $v1, $v1, $at
    sw      $v1, 0x2010($v0)
    lui     $v0, 0x1001
    sw      $zero, -20448($v0)
    sw      $zero, -19424($v0)
    sw      $zero, 0($a3)
    sw      $zero, 12($sp)
    addiu   $v1, $zero, 64
    addu    $a0, $sp, 4
    syscall
    sw      $v0, s_Sema
    addiu   $a0, $zero,  3
    addiu   $v1, $zero, 18
    lui     $a1, %hi( _mpeg_dmac_handler )
    la      $a3, s_CSCParam
    xor     $a2, $a2, $a2
    addiu   $a1, %lo( _mpeg_dmac_handler )
    lw      $ra, 0($sp)
    syscall
    addiu   $sp, $sp, 48
    sw      $v0, s_CSCID
    jr      $ra
    sd      $zero, s_DataBuf

_MPEG_Destroy:
1:
    lb      $v1, s_CSCFlag
    bne     $v1, $zero, 1b
    lw      $a1, s_CSCID
    addiu   $a0, $zero, 3
    addiu   $v1, $zero, 19
    syscall
    addiu   $v1, $zero, 65
    lw      $a0, s_Sema
    syscall
    jr      $ra

_MPEG_Suspend:
1:
    lb      $v0, s_CSCFlag
    bne     $v0, $zero, 1b
_ipu_suspend:
    lui     $a1, 0x1001
    lui     $v0, 0x0001
1:
    di
    sync.p
    mfc0    $at, $12
    and     $at, $at, $v0
    bne     $at, $zero, 1b
    lui     $v0, 0x0001
    lw      $a2, -2784($a1)
    nor     $v1, $v0, $zero
    or      $a2, $a2, $v0
    sw      $a2, -2672($a1)
    lw      $at, -19456($a1)
    sra     $a3, $v1, 8
    subu    $t1, $a1, $v0
    and     $at, $at, $a3
    sw      $at, -19456($a1)
    lw      $a2, -2784($a1)
    sw      $at, s_IPUState + 0
    and     $a2, $a2, $v1
    sw      $a2, -2672($a1)
    ei
    lw      $at, -19440($a1)
    lw      $a2, -19424($a1)
    sw      $at, s_IPUState + 4
    sw      $a2, s_IPUState + 8
1:
    lw      $at, 0x2010($t1)
    andi    $at, $at, 0x00F0
    bne     $at, $zero, 1b
    nop
1:
    di
    sync.p
    mfc0    $at, $12
    and     $at, $at, $v0
    bne     $at, $zero, 1b
    nop
    lw      $a2, -2784($a1)
    or      $a2, $a2, $v0
    sw      $a2, -2672($a1)
    lw      $at, -20480($a1)
    and     $at, $at, $a3
    sw      $at, -20480($a1)
    lw      $a2, -2784($a1)
    sw      $at, s_IPUState + 12
    and     $a2, $a2, $v1
    sw      $a2, -2672($a1)
    ei
    lw      $at, -20464($a1)
    lw      $a2, -20448($a1)
    sw      $at, s_IPUState + 16
    sw      $a2, s_IPUState + 20
    lw      $at, 0x2010($t1)
    lw      $a2, 0x2020($t1)
    sw      $at, s_IPUState + 24
    jr      $ra
    sw      $a2, s_IPUState + 28

_MPEG_Resume:
_ipu_resume:
    lw      $v1, s_IPUState + 20
    lui     $a0, 0x1001
    lui     $a1, 0x1000
    addiu   $a2, $zero, 0x0100
    beq     $v1, $zero, 1f
    lw      $at, s_IPUState + 28
    lw      $a3, s_IPUState + 12
    lw      $v0, s_IPUState + 16
    sw      $v0, -20464($a0)
    or      $a3, $a3, $a2
    sw      $v1, -20448($a0)
    sw      $a3, -20480($a0)
1:
    lw      $a3, s_IPUState + 8
    andi    $v0, $at, 0x007F
    srl     $v1, $at, 16
    srl     $at, $at,  8
    andi    $v1, $v1, 0x0003
    andi    $at, $at, 0x000F
    addu    $v1, $v1, $at
    lw      $at, s_IPUState + 4
    addu    $a3, $a3, $v1
    beq     $a3, $zero, 2f
    sll     $v1, $v1, 4
    subu    $at, $at, $v1
    sw      $v0, 0x2000($a1)
    lw      $v1, s_IPUState + 0
1:
    lw      $v0, 0x2010($a1)
    bltz    $v0, 1b
    nop
    lw      $v0, s_IPUState + 24
    or      $v1, $v1, $a2
    sw      $v0, 0x2010($a1)
    sw      $at, -19440($a0)
    sw      $a3, -19424($a0)
    sw      $v1, -19456($a0)
2:
    jr      $ra
    addiu   $v0, $v0, 1

_mpeg_dmac_handler:
    lw      $at, 8($a1)
    beql    $at, $zero, 1f
    addiu   $v1, $zero, -29
    lw      $a0, 0($a1)
    lw      $a2, 4($a1)
    addiu   $a3, $zero, 1023
    addiu   $v1, $zero,  384
    pminw   $a3, $a3, $at
    lui     $t1, 0x1001
    sll     $v0, $a3, 10
    mult    $v1, $v1, $a3
    subu    $at, $at, $a3
    sw      $a2, -20464($t1)
    sw      $a0, -19440($t1)
    addu    $a2, $a2, $v0
    srl     $v0, $v0, 4
    addu    $a0, $a0, $v1
    sw      $a0, 0($a1)
    srl     $v1, $v1, 4
    sw      $a2, 4($a1)
    lui     $t0, 0x1000
    sw      $at, 8($a1)
    sw      $v0, -20448($t1)
    lui     $v0, 0x7000
    sw      $v1, -19424($t1)
    addiu   $v1, $zero, 0x0101
    or      $v0, $v0, $a3
    sw      $v1, -19456($t1)
    andi    $v1, 0x0100
    sw      $v0, 0x2000($t0)
    sw      $v1, -20480($t1)
    jr      $ra
    nor     $v0, $zero, $zero
1:
    addiu   $a0, $zero, 3
    syscall
    lw      $a0, s_Sema
    addiu   $v1, $zero, -67
    syscall
    sb      $zero, s_CSCFlag
    jr      $ra
    nor     $v0, $zero, $zero

_MPEG_CSCImage:
    addiu   $sp, $sp, -16
    sw      $ra,  0($sp)
    sw      $a0,  4($sp)
    sw      $a1,  8($sp)
    bgezal  $zero, _ipu_suspend
    sw      $a2, 12($sp)
    sw      $zero, 0x2000($t1)
    addiu   $t0, $zero, 1023
    addiu   $v0, $zero,    8
    addiu   $a0, $zero,    3
    addiu   $v1, $zero,   22
    lw      $a2, 12($sp)
    addiu   $t3, $zero,  384
    sw      $v0, -8176($a1)
    pminw   $t0, $t0, $a2
    lw      $t4, 4($sp)
    lw      $a3, 8($sp)
    subu    $a2, $a2, $t0
    mult    $t3, $t3, $t0
    sll     $t5, $t0, 10
    sw      $a3, -20464($a1)
    sw      $t4, -19440($a1)
    sw      $a2, s_CSCParam + 8
    addu    $t4, $t4, $t3
    addu    $a3, $a3, $t5
    sw      $t4, s_CSCParam
    srl     $t3, $t3, 4
    sw      $a3, s_CSCParam + 4
    srl     $t5, $t5, 4
    sw      $t3, -19424($a1)
    sw      $t5, -20448($a1)
    sw      $t0, 4($sp)
    syscall
    lw      $t0, 4($sp)
    addiu   $v1, $zero, 0x0101
    lui     $at, 0x1001
    lui     $v0, 0x7000
    lui     $a0, 0x1000
    or      $v0, $v0, $t0
    sw      $v1, -19456($at)
    andi    $v1, $v1, 0x0100
    sw      $v0, 0x2000($a0)
    sw      $v1, -20480($at)
    lw      $a0, s_Sema
    addiu   $v1, $zero, 68
    sb      $v1, s_CSCFlag
    syscall
    lw      $ra, 0($sp)
    beq     $zero, $zero, _ipu_resume
    addiu   $sp, $sp, 16
1:
    lw      $v1, 0x2010($at)
_ipu_sync:
    lui     $a1, 0x0003
    andi    $a2, $a0, 0xFF00
    and     $v0, $a0, $a1
    andi    $a0, $a0, 0x007F
    addiu   $a1, $zero, 0x4000
    srl     $a2, $a2, 1
    srl     $v0, $v0, 9
    and     $a1, $a1, $v1
    addu    $a2, $a2, $v0
    subu    $a2, $a2, $a0
    bne     $a1, $zero, 3f
    slti    $a2, $a2, 32
    beq     $a2, $zero, 2f
    lui     $a2, 0x1001
    lw      $a2, -19424($a2)
    bgtzl   $a2, 1b
    lw      $a0, 0x2020($at)
    addiu   $sp, $sp, -16
    lw      $a2, s_SetDMA + 0
    sw      $ra, 0($sp)
    jalr    $a2
    lw      $a0, s_SetDMA + 4
    lw      $ra, 0($sp)
    addiu   $sp, $sp, 16
    beql    $v0, $zero, 4f
    lw      $v1, s_pEOF
    lui     $at, 0x1000
2:
    lw      $v1, 0x2010($at)
    bltzl   $v1, _ipu_sync
    lw      $a0, 0x2020($at)
3:
    jr      $ra
4:
    addiu   $a0, $zero, 32
    addiu   $v0, $zero, 0x01B7
    sw      $a0, s_DataBuf
    sw      $v0, s_DataBuf + 4
    jr      $ra
    sw      $a0, 0($v1)

_ipu_sync_data:
    lui     $at, 0x1000
    ld      $v0, 0x2000($at)
    bltzl   $v0, 1f
    lw      $a0, 0x2020($at)
    jr      $ra
1:
    lui     $a1, 0x0003
    andi    $v1, $a0, 0xFF00
    and     $v0, $a0, $a1
    srl     $v1, $v1, 1
    srl     $v0, $v0, 9
    addu    $v1, $v1, $v0
    andi    $a0, $a0, 0x7F
    subu    $v0, $v1, $a0
    sltiu   $v0, $v0, 32
    beq     $v0, $zero, 2f
    lui     $v0, 0x1001
    lw      $v0, -19424($v0)
    bgtzl   $v0, 1b
    lw      $a0, 0x2020($at)
    lw      $v0, s_SetDMA + 0
    addiu   $sp, $sp, -16
    sw      $ra, 0($sp)
    jalr    $v0
    lw      $a0, s_SetDMA + 4
    lw      $ra, 0($sp)
    addiu   $sp, $sp, 16
    beql    $v0, $zero, 4b
    lw      $v1, s_pEOF
    lui     $at, 0x1000
2:
    ld      $v0, 0x2000($at)
    bltzl   $v0, 1b
    lw      $a0, 0x2020($at)
    jr      $ra

_MPEG_GetBits:
_ipu_get_bits:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sd      $ra, 0($sp)
    sd      $s0, 8($sp)
    addu    $s0, $zero, $a0
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lw      $v1, s_DataBuf + 0
    slt     $v0, $v1, $s0
    beqzl   $v0, 1f
    lw      $v0, s_DataBuf + 4
    lui     $at, 0x1000
    lui     $a1, 0x4000
    bgezal  $zero, _ipu_sync_data
    sw      $a1, 0x2000($at)
    addiu   $v1, $zero, 32
1:
    lui     $a1, 0x4000
    or      $a1, $a1, $s0
    subu    $v1, $v1, $s0
    sw      $a1, 0x2000($at)
    sw      $v1, s_DataBuf + 0
    subu    $a2, $zero, $s0
    sllv    $v1, $v0, $s0
    srlv    $v0, $v0, $a2
    sw      $v1, s_DataBuf + 4
    ld      $ra, 0($sp)
    ld      $s0, 8($sp)
    jr      $ra
    addiu   $sp, $sp, 16

_MPEG_ShowBits:
_ipu_show_bits:
    lw      $v1, s_DataBuf + 0
    slt     $v0, $v1, $a0
    beqzl   $v0, 1f
    lw      $v0, s_DataBuf + 4
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    sw      $a0, 4($sp)
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lui     $at, 0x1000
    lui     $a1, 0x4000
    bgezal  $zero, _ipu_sync_data
    sw      $a1, 0x2000($at)
    addiu   $v1, $zero, 32
    sw      $v1, s_DataBuf + 0
    sw      $v0, s_DataBuf + 4
    lw      $ra, 0($sp)
    lw      $a0, 4($sp)
    addiu   $sp, $sp, 16
1:
    subu    $a0, $zero, $a0
    jr      $ra
    srlv    $v0, $v0, $a0

_MPEG_AlignBits:
_ipu_align_bits:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lw      $a0, 0x2020($at)
    andi    $a0, $a0, 7
    subu    $a0, $zero, $a0
    andi    $a0, $a0, 7
    beq     $a0, $zero, 1f
    lw      $ra, 0($sp)
    beq     $zero, $zero, _ipu_get_bits
1:
    addiu   $sp, $sp, 16
    jr      $ra
    nop

_MPEG_NextStartCode:
    addiu   $sp, $sp, -16
    sw      $ra, 0($sp)
    bgezal  $zero, _ipu_align_bits
    nop
1:
    bgezal  $zero, _ipu_show_bits
    addiu   $a0, $zero, 24
    addiu   $v1, $zero, 1
4:
    bne     $v0, $v1, 5f
    addiu   $a0, $zero, 32
    lw      $ra, 0($sp)
    beq     $zero, $zero, _ipu_show_bits
    addiu   $sp, $sp, 16
5:
    bgezal  $zero, _ipu_get_bits
    addiu   $a0, $zero, 8
    beq     $zero, $zero, 1b
    nop

_MPEG_SetDefQM:
    addiu   $sp, $sp, -16
    sw      $ra, 0($sp)
    bgezal  $zero, _ipu_suspend
    nop
    lui     $v1, 0x1000
    la      $at, s_DefQM
    sw      $zero, 0x2000($v1)
    lq      $a0,  0($at)
    lq      $a1, 16($at)
    lq      $a2, 32($at)
    lq      $a3, 48($at)
    lq      $t0, 64($at)
    lui     $v0, 0x5000
1:
    lw      $at, 0x2010($v1)
    bltz    $at, 1b
    nop
    sq      $a0, 0x7010($v1)
    sq      $a1, 0x7010($v1)
    sq      $a2, 0x7010($v1)
    sq      $a3, 0x7010($v1)
    sw      $v0, 0x2000($v1)
    lui     $v0, 0x5800
1:
    lw      $at, 0x2010($v1)
    bltz    $at, 1b
    nop
    sq      $t0, 0x7010($v1)
    sq      $t0, 0x7010($v1)
    sq      $t0, 0x7010($v1)
    sq      $t0, 0x7010($v1)
    sw      $v0, 0x2000($v1)
1:
    lw      $at, 0x2010($v1)
    bltz    $at, 1b
    nop
    lw      $ra, 0($sp)
    beq     $zero, $zero, _ipu_resume
    addiu   $sp, $sp, 16

_MPEG_SetQM:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    sd      $s0, 8($sp)
    sll     $s0, $a0, 27
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lui     $a0, 0x5000
    or      $a0, $a0, $s0
    sw      $a0, 0x2000($at)
    lw      $ra, 0($sp)
    ld      $s0, 8($sp)
    addiu   $sp, $sp, 16
    jr      $ra
    sd      $zero, s_DataBuf

_MPEG_GetMBAI:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    sd      $s0, 8($sp)
    addu    $s0, $zero, $zero
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
3:
    lui     $v0, 0x3000
4:
    bgezal  $zero, _ipu_sync_data
    sw      $v0, 0x2000($at)
    beql    $v0, $zero, 1f
    addu    $s0, $zero, $zero
    andi    $v0, $v0, 0xFFFF
    slti    $v1, $v0, 0x0022
    bnel    $v1, $zero, 2f
    addu    $s0, $s0, $v0
    addiu   $v1, $zero, 0x0023
    beql    $v0, $v1, 3b
    addiu   $s0, $s0, 0x0021
    beq     $zero, $zero, 4b
    lui     $v0, 0x3000
2:
    addiu   $v1, $zero, 32
    ld      $a0, 0x2030($at)
    sw      $v1, s_DataBuf + 0
    sw      $a0, s_DataBuf + 4
1:
    addu    $v0, $zero, $s0
    lw      $ra, 0($sp)
    ld      $s0, 8($sp)
    jr      $ra
    addiu   $sp, $sp, 16

_MPEG_GetMBType:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lui     $a2, 0x3400
    bgezal  $zero, _ipu_sync_data
    sw      $a2, 0x2000($at)
    beq     $v0, $zero, 1f
    addiu   $v1, $zero, 32
    ld      $a1, 0x2030($at)
    andi    $v0, $v0, 0xFFFF
    sw      $v1, s_DataBuf + 0
    sw      $a1, s_DataBuf + 4
1:
    lw      $ra, 0($sp)
    jr      $ra
    addiu   $sp, $sp, 16

_MPEG_GetMotionCode:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lui     $a2, 0x3800
    bgezal  $zero, _ipu_sync_data
    sw      $a2, 0x2000($at)
    beql    $v0, $zero, 1f
    addiu   $v0, $zero, 0x8000
    addiu   $v1, $zero, 32
    ld      $a1, 0x2030($at)
    andi    $v0, $v0, 0xFFFF
    sw      $v1, s_DataBuf + 0
    sw      $a1, s_DataBuf + 4
1:
    dsll32  $v0, $v0, 16
    lw      $ra, 0($sp)
    dsra32  $v0, $v0, 16
    jr      $ra
    addiu   $sp, $sp, 16

_MPEG_GetDMVector:
    lui     $at, 0x1000
    addiu   $sp, $sp, -16
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lui     $a2, 0x3C00
    bgezal  $zero, _ipu_sync_data
    sw      $a2, 0x2000($at)
    addiu   $v1, $zero, 32
    ld      $a1, 0x2030($at)
    dsll32  $v0, $v0, 16
    sw      $v1, s_DataBuf + 0
    sw      $a1, s_DataBuf + 4
    lw      $ra, 0($sp)
    dsra32  $v0, $v0, 16
    jr      $ra
    addiu   $sp, $sp, 16

_MPEG_SetIDCP:
    addiu   $sp, $sp, -16
    sw      $ra, 0($sp)
    bgezal  $zero, _ipu_get_bits
    addiu   $a0, $zero, 2
    lui     $v1, 0xFFFC
    sll     $v0, $v0, 16
    lw      $a0, 0x2010($at)
    ori     $v1, $v1, 0xFFFF
    lw      $ra, 0($sp)
    and     $a0, $a0, $v1
    addiu   $sp, $sp, 16
    or      $a0, $a0, $v0
    jr      $ra
    sw      $a0, 0x2010($at)

_MPEG_SetQSTIVFAS:
    addiu   $sp, $sp, -16
    sd      $ra, 0($sp)
    sd      $s0, 8($sp)
    bgezal  $zero, _ipu_get_bits
    addiu   $a0, $zero, 1
    sll     $s0, $v0, 22
    bgezal  $zero, _ipu_get_bits
    addiu   $a0, $zero, 1
    sll     $v0, $v0, 21
    addiu   $a0, $zero, 1
    bgezal  $zero, _ipu_get_bits
    or      $s0, $s0, $v0
    sll     $v0, $v0, 20
    lw      $a0, 0x2010($at)
    lui     $v1, 0xFF8F
    or      $s0, $s0, $v0
    ori     $v1, $v1, 0xFFFF
    ld      $ra, 0($sp)
    and     $a0, $a0, $v1
    addiu   $sp, $sp, 16
    or      $a0, $a0, $s0
    ld      $s0, -8($sp)
    jr      $ra
    sw      $a0, 0x2010($at)

_MPEG_SetPCT:
    sll     $a0, $a0, 24
    addiu   $sp, $sp, -16
    lui     $at, 0x1000
    sw      $ra, 0($sp)
    sw      $a0, 4($sp)
    lw      $v1, 0x2010($at)
    bltzl   $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lw      $v0, 4($sp)
    lui     $a0, 0xF8FF
    ori     $a0, $a0, 0xFFFF
    and     $v1, $v1, $a0
    or      $v1, $v1, $v0
    lw      $ra, 0($sp)
    addiu   $sp, $sp, 16
    jr      $ra
    sw      $v1, 0x2010($at)

_MPEG_BDEC:
    addiu   $sp, $sp, -16
    sll     $a0, $a0, 27
    sd      $ra, 0($sp)
    sll     $a1, $a1, 26
    sd      $s0, 8($sp)
    lui     $s0, 0x2000
    sll     $a2, $a2, 25
    or      $s0, $s0, $a0
    sll     $a3, $a3, 16
    or      $s0, $s0, $a1
    lui     $a0, 0x8000
    or      $s0, $s0, $a2
    sll     $t0, $t0, 4
    or      $s0, $s0, $a3
    srl     $t0, $t0, 4
    lui     $a1, 0x1001
    lui     $at, 0x1000
    or      $t0, $t0, $a0
    lw      $v1, 0x2010($at)
    addiu   $a0, $zero, 48
    addiu   $a2, $zero, 0x0100
    sw      $t0, -20464($a1)
    sw      $a0, -20448($a1)
    sw      $a2, -20480($a1)
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    ld      $ra, 0($sp)
    sw      $s0, 0x2000($at)
    ld      $s0, 8($sp)
    jr      $ra
    addiu   $sp, $sp, 16

_MPEG_WaitBDEC:
    addiu   $sp, $sp, -16
    lui     $at, 0x1000
    lw      $v1, 0x2010($at)
    sw      $ra, 0($sp)
1:
    bltzall $v1, _ipu_sync
    lw      $a0, 0x2020($at)
    lw      $v1, s_pEOF
    addiu   $a0, $zero, 0x4000
    lw      $v1, 0($v1)
    lui     $a2, 0x1001
    bne     $v1, $zero, 3f
    lw      $v0, 0x2010($at)
    and     $v0, $v0, $a0
    bne     $v0, $zero, 3f
    lw      $a2, -20448($a2)
    addiu   $v0, $zero, 1
    bnel    $a2, $zero, 1b
    lw      $v1, 0x2010($at)
    ld      $v1, 0x2030($at)
    addiu   $ra, $zero, 32
    addiu   $v0, $zero, 1
    pextlw  $v1, $v1, $ra
2:
    lw      $ra, 0($sp)
    sd      $v1, s_DataBuf
    jr      $ra
    addiu   $sp, $sp, 16
3:
    bgezal  $zero, _ipu_suspend
    lui     $t0, 0x4000
    bgezal  $zero, _ipu_resume
    sw      $t0, 0x2010($t1)
    lui     $v0, 0x0001
4:
    di
    sync.p
    mfc0    $at, $12
    and     $at, $at, $v0
    nor     $a2, $v0, $zero
    bne     $at, $zero, 4b
    lw      $at, -2784($a0)
    xor     $v1, $v1, $v1
    or      $at, $at, $v0
    sw      $at, -2672($a0)
    sw      $zero, -20480($a0)
    lw      $at, -2784($a0)
    xor     $v0, $v0, $v0
    and     $at, $at, $a2
    sw      $at, -2672($a0)
    ei
    beq     $zero, $zero, 2b
    sw      $zero, -20448($a0)

_MPEG_put_block_fr:
    lw      $a2, 0($a0)
    lw      $a3, 8($a0)
    pnor    $v0, $zero, $zero
    addiu   $v1, $zero, 6
    psrlh   $v0, $v0, 8
1:
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1;
    lq      $t4,  64($a3)
    lq      $t5,  80($a3)
    lq      $t6,  96($a3)
    lq      $t7, 112($a3)
    addiu   $a3, $a3, 128
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t0,  0($a2)
    sq      $t2, 16($a2)
    sq      $t4, 32($a2)
    sq      $t6, 48($a2)
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    jr      $ra

_MPEG_put_block_fl:
    pnor    $v0, $zero, $zero
    lw      $a2, 0($a0)
    lw      $a3, 8($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
1:
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t4, 256($a3)
    lq      $t5, 272($a3)
    lq      $t6, 288($a3)
    lq      $t7, 304($a3)
    addiu   $a3, $a3, 64
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t0,  0($a2)
    sq      $t4, 16($a2)
    sq      $t2, 32($a2)
    sq      $t6, 48($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 64
    addiu   $v1, $v1, 2
2:
    lq      $t0, 256($a3)
    lq      $t1, 272($a3)
    lq      $t2, 288($a3)
    lq      $t3, 304($a3)
    addiu   $v1, $v1, -1
    lq      $t4, 320($a3)
    lq      $t5, 336($a3)
    lq      $t6, 352($a3)
    lq      $t7, 368($a3)
    addiu   $a3, $a3, 128
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t0,  0($a2)
    sq      $t2, 16($a2)
    sq      $t4, 32($a2)
    sq      $t6, 48($a2)
    bgtzl   $v1, 2b
    addiu   $a2, $a2, 64
    jr      $ra

_MPEG_put_block_il:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3,  8($a0)
    lw      $at, 24($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
    addu    $at, $at, $a2
1:
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t4, 256($a3)
    lq      $t5, 272($a3)
    lq      $t6, 288($a3)
    lq      $t7, 304($a3)
    addiu   $a3, $a3, 64
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t0, $t1, $t0
    ppacb   $t2, $t3, $t2
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t0,  0($a2)
    sq      $t2, 32($a2)
    addiu   $a2, $a2, 64
    sq      $t4,  0($at)
    sq      $t6, 32($at)
    bgtzl   $v1, 1b
    addiu   $at, $at, 64
    lw      $a2,  4($a0)
    lw      $at, 24($a0)
    addiu   $v1, $zero, 2
    addu    $at, $at, $a2
2:
    lq      $t0, 256($a3)
    lq      $t1, 272($a3)
    lq      $t2, 288($a3)
    lq      $t3, 304($a3)
    addiu   $v1, $v1, -1
    lq      $t4, 320($a3)
    lq      $t5, 336($a3)
    lq      $t6, 352($a3)
    lq      $t7, 368($a3)
    addiu   $a3, $a3, 128
    pmaxh   $t0, $zero, $t0
    pmaxh   $t1, $zero, $t1
    pmaxh   $t2, $zero, $t2
    pmaxh   $t3, $zero, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t0, $v0, $t0
    pminh   $t1, $v0, $t1
    pminh   $t2, $v0, $t2
    pminh   $t3, $v0, $t3
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t0, $zero, $t0
    ppacb   $t1, $zero, $t1
    ppacb   $t2, $zero, $t2
    ppacb   $t3, $zero, $t3
    ppacb   $t4, $zero, $t4
    ppacb   $t5, $zero, $t5
    ppacb   $t6, $zero, $t6
    ppacb   $t7, $zero, $t7
    sd      $t0,  0($a2)
    sd      $t1, 16($a2)
    sd      $t2, 32($a2)
    sd      $t3, 48($a2)
    sd      $t4,  0($at)
    sd      $t5, 16($at)
    sd      $t6, 32($at)
    sd      $t7, 48($at)
    addiu   $a2, $a2, 64
    bgtzl   $v1, 2b
    addiu   $at, $at, 64
    jr      $ra

_MPEG_add_block_frfr:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3, 12($a0)
    lw      $a0, 16($a0)
    addiu   $v1, $zero, 6
    psrlh   $v0, $v0, 8
1:
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t4,   0($a0)
    lq      $t5,  16($a0)
    lq      $t6,  32($a0)
    lq      $t7,  48($a0)
    paddh   $t0, $t0, $t4
    paddh   $t1, $t1, $t5
    paddh   $t2, $t2, $t6 
    paddh   $t3, $t3, $t7
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
    sq      $t0,  0($a2)
    sq      $t2, 16($a2)
    lq      $t4,  64($a3)
    lq      $t5,  80($a3)
    lq      $t6,  96($a3)
    lq      $t7, 112($a3)
    addiu   $a3, $a3, 128
    lq      $t0,  64($a0)
    lq      $t1,  80($a0)
    lq      $t2,  96($a0)
    lq      $t3, 112($a0)
    addiu   $a0, $a0, 128
    paddh   $t4, $t4, $t0
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t7, $t7, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t4, 32($a2)
    sq      $t6, 48($a2)
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    jr      $ra

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
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t4,   0($a1)
    lq      $t5,  16($a1)
    lq      $t6,  32($a1)
    lq      $t7,  48($a1)
    paddh   $t0, $t0, $t4
    paddh   $t1, $t1, $t5
    paddh   $t2, $t2, $t6
    paddh   $t3, $t3, $t7
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
    sq      $t0,   0($a2)
    sq      $t2,  32($a2)
    lq      $t4, 256($a3)
    lq      $t5, 272($a3)
    lq      $t6, 288($a3)
    lq      $t7, 304($a3)
    addiu   $a3, $a3, 64
    lq      $t0, 256($a1)
    lq      $t1, 272($a1)
    lq      $t2, 288($a1)
    lq      $t3, 304($a1)
    paddh   $t4, $t4, $t0
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t7, $t7, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t4,  0($at)
    sq      $t6, 32($at)
    addiu   $at, $at, 64
    addiu   $a1, $a1, 64
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    lw      $a2,  4($a0)
    lw      $at, 24($a0)
    addiu   $v1, $zero, 2
    addu    $at, $at, $a2
2:
    lq      $t0, 256($a3)
    lq      $t1, 272($a3)
    lq      $t2, 288($a3)
    lq      $t3, 304($a3)
    addiu   $v1, $v1, -1
    lq      $t4, 256($a1)
    lq      $t5, 272($a1)
    lq      $t6, 288($a1)
    lq      $t7, 304($a1)
    paddh   $t0, $t0, $t4
    paddh   $t1, $t1, $t5
    paddh   $t2, $t2, $t6
    paddh   $t3, $t3, $t7
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
    sd      $t0,  0($a2)
    sd      $t1, 16($a2)
    sd      $t2, 32($a2)
    sd      $t3, 48($a2)
    lq      $t4, 320($a3)
    lq      $t5, 336($a3)
    lq      $t6, 352($a3)
    lq      $t7, 368($a3)
    addiu   $a3, $a3, 128
    lq      $t0, 320($a1)
    lq      $t1, 336($a1)
    lq      $t2, 352($a1)
    lq      $t3, 368($a1)
    paddh   $t4, $t4, $t0
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t7, $t7, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t4, $zero, $t4
    ppacb   $t5, $zero, $t5
    ppacb   $t6, $zero, $t6
    ppacb   $t7, $zero, $t7
    sd      $t4,  0($at)
    sd      $t5, 16($at)
    sd      $t6, 32($at)
    sd      $t7, 48($at)
    addiu   $a2, $a2, 64
    addiu   $at, $at, 64
    bgtzl   $v1, 2b
    addiu   $a1, $a1, 128
    jr      $ra

_MPEG_add_block_frfl:
    pnor    $v0, $zero, $zero
    lw      $a2,  0($a0)
    lw      $a3, 12($a0)
    lw      $a1, 16($a0)
    addiu   $v1, $zero, 4
    psrlh   $v0, $v0, 8
1:
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t4,   0($a1)
    lq      $t5,  16($a1)
    lq      $t6, 256($a1)
    lq      $t7, 272($a1)
    paddh   $t0, $t0, $t4
    paddh   $t1, $t1, $t5
    paddh   $t2, $t2, $t6
    paddh   $t3, $t3, $t7
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
    sq      $t0,   0($a2)
    sq      $t2,  16($a2)
    lq      $t4,  64($a3)
    lq      $t5,  80($a3)
    lq      $t6,  96($a3)
    lq      $t7, 112($a3)
    addiu   $a3, $a3, 128
    lq      $t0,  32($a1)
    lq      $t1,  48($a1)
    lq      $t2, 288($a1)
    lq      $t3, 304($a1)
    paddh   $t4, $t4, $t0
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t7, $t7, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t4, 32($a2)
    sq      $t6, 48($a2)
    addiu   $a1, $a1, 64
    bgtzl   $v1, 1b
    addiu   $a2, $a2, 64
    lw      $a2, 4($a0)
    addiu   $v1, $zero, 2
2:
    lq      $t0,   0($a3)
    lq      $t1,  16($a3)
    lq      $t2,  32($a3)
    lq      $t3,  48($a3)
    addiu   $v1, $v1, -1
    lq      $t4, 256($a1)
    lq      $t5, 320($a1)
    lq      $t6, 272($a1)
    lq      $t7, 336($a1)
    paddh   $t0, $t0, $t4
    paddh   $t1, $t1, $t5
    paddh   $t2, $t2, $t6
    paddh   $t3, $t3, $t7
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
    sq      $t0,  0($a2)
    sq      $t2, 16($a2)
    lq      $t4,  64($a3)
    lq      $t5,  80($a3)
    lq      $t6,  96($a3)
    lq      $t7, 112($a3)
    addiu   $a3, $a3, 128
    lq      $t0, 288($a1)
    lq      $t1, 352($a1)
    lq      $t2, 304($a1)
    lq      $t3, 368($a1)
    paddh   $t4, $t4, $t0
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t7, $t7, $t3
    pmaxh   $t4, $zero, $t4
    pmaxh   $t5, $zero, $t5
    pmaxh   $t6, $zero, $t6
    pmaxh   $t7, $zero, $t7
    pminh   $t4, $v0, $t4
    pminh   $t5, $v0, $t5
    pminh   $t6, $v0, $t6
    pminh   $t7, $v0, $t7
    ppacb   $t4, $t5, $t4
    ppacb   $t6, $t7, $t6
    sq      $t4, 32($a2)
    sq      $t6, 48($a2)
    addiu   $a2, $a2, 64
    bgtzl   $v1, 2b
    addiu   $a1, $a1, 128
    jr      $ra

_MPEG_dma_ref_image:
    addiu   $at, $zero, 4
    pminw   $a2, $a2, $at
    bgtzl   $a2, 1f
    addiu   $at, $at, 380
    jr      $ra
1:
    lui     $v0, 0x1001
    mult    $a3, $a3, $at
    sll     $at, $a0, 4
    lui     $t1, 0x2000
    la      $t0, s_DMAPack
1:
    lw      $v1, -11264($v0)
    andi    $v1, $v1, 0x0100
    bne     $v1, $zero, 1b
    nop
    srl     $at, $at, 4
    sw      $zero, -11232($v0)
    or      $t1, $t1, $t0
    sw      $at, -11136($v0)
    lui     $v1, 0x3000
    sw      $t0, -11216($v0)
    ori     $v1, $v1, 0x0030
1:
    lw      $t0, 0($a1)
    addiu   $a2, $a2, -1
    sw      $v1,  0($t1)
    sw      $t0,  4($t1)
    addu    $t0, $t0, $a3
    sw      $v1, 16($t1)
    sw      $t0, 20($t1)
    sw      $a0, 0($a1)
    addiu   $a1, $a1, 40
    addiu   $t1, $t1, 32
    bgtz    $a2, 1b
    addiu   $a0, $a0, 1536
    andi    $v1, $v1, 0xFFFF
    addiu   $at, $zero, 0x0105
    sw      $v1, -16($t1)
    sw      $zero, 32($a1)
    sync.l
    jr      $ra
    sw      $at, -11264($v0)

_MPEG_do_mc:
    addiu   $v0, $zero, 16
    lw      $a1,  0($a0)
    addiu   $sp, $sp, -16
    lw      $a2,  4($a0)
    lw      $a3, 12($a0)
    lw      $t0, 16($a0)
    lw      $t1, 20($a0)
    lw      $t2, 24($a0)
    lw      $t4, 28($a0)
    subu    $t0, $t0, $t4
    lw      $t5, 32($a0)
    sll     $t4, $t4, 4
    addu    $a1, $a1, $t4
    subu    $v1, $v0, $t0
    sllv    $t3, $v0, $t2
    srlv    $v1, $v1, $t2
    sll     $at, $t0, 4
    sw      $ra, 0($sp)
    addu    $a1, $a1, $at
    jalr    $t5
    subu    $at, $t1, $v1
    lw      $a1,  0($a0)
    lw      $a2,  8($a0)
    lw      $t5, 36($a0)
    addiu   $a1, $a1, 256
    srl     $t4, $t4, 1
    srl     $a3, $a3, 1
    srl     $t0, $t0, 1
    srl     $t1, $t1, 1
    lw      $ra, 0($sp)
    srlv    $t0, $t0, $t2
    addu    $a1, $a1, $t4
    addiu   $v0, $zero, 8
    sllv    $t0, $t0, $t2
    subu    $v1, $v0, $t0
    sllv    $t3, $v0, $t2
    srlv    $v1, $v1, $t2
    sll     $at, $t0, 3
    addu    $a1, $a1, $at
    subu    $at, $t1, $v1
    jr      $t5
    addiu   $sp, $sp, 16

_MPEG_put_luma:
    mtsab   $a3, 0
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t5, $t6, $t5
    pextlb  $t6, $zero, $t5
    pextub  $t5, $zero, $t5
    sq      $t6,  0($a2)
    sq      $t5, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_put_chroma:
    mtsab   $a3, 0
1:
    ld      $t5,   0($a1)
    ld      $t6,  64($a1)
    ld      $t7, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    pcpyld  $t5, $t7, $t5
    pcpyld  $t6, $t8, $t6
    qfsrv   $t5, $t5, $t5
    qfsrv   $t6, $t6, $t6
    pextlb  $t5, $zero, $t5
    pextlb  $t6, $zero, $t6
    sq      $t5,   0($a2)
    sq      $t6, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_put_luma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    mtsab   $a3, 0
    qfsrv   $t7, $t6, $t5
    qfsrv   $t8, $t5, $t6
    pextlb  $t5, $zero, $t7
    pextub  $t6, $zero, $t7
    addu    $a1, $a1, $t3
    mtsab   $zero, 1
    addiu   $v1, $v1, -1
    qfsrv   $t8, $t8, $t7
    pextlb  $t7, $zero, $t8
    pextub  $t8, $zero, $t8
    paddh   $t5, $t5, $t7
    paddh   $t6, $t6, $t8
    paddh   $t5, $t5, $v0
    paddh   $t6, $t6, $v0
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    sq      $t5,  0($a2)
    sq      $t6, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_put_chroma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    ld      $t5,   0($a1)
    ld      $t6,  64($a1)
    ld      $t7, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t5, $t7, $t5
    pcpyld  $t6, $t8, $t6
    mtsab   $a3, 0
    qfsrv   $t5, $t5, $t5
    qfsrv   $t6, $t6, $t6
    addiu   $t9, $zero, 1
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    mtsab   $t9, 0
    qfsrv   $t1, $t5, $t5
    qfsrv   $t2, $t6, $t6
    pextlb  $t5, $zero, $t5
    pextlb  $t6, $zero, $t6
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t5, $t5, $v0
    paddh   $t6, $t6, $v0
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    sq      $t5,   0($a2)
    sq      $t6, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_put_luma_Y:
    mtsab   $a3, 0
    lq      $t7,   0($a1)
    lq      $t8, 384($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t7, $t8, $t7
    pextub  $t8, $zero, $t7
    pextlb  $t7, $zero, $t7
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t5, $t6, $t5
    pextub  $t6, $zero, $t5
    pextlb  $t5, $zero, $t5
    paddh   $v0, $t6, $t8
    pnor    $t8, $zero, $zero
    paddh   $t9, $t5, $t7
    psrlh   $t8, $t8, 15
    por     $t7, $zero, $t5
    paddh   $t9, $t9, $t8
    paddh   $v0, $v0, $t8
    por     $t8, $zero, $t6
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

_MPEG_put_chroma_Y:
    mtsab   $a3, 0
    ld      $a0,   0($a1)
    ld      $a3,  64($a1)
    ld      $t0, 384($a1)
    ld      $t1, 448($a1)
    pnor    $v0, $zero, $zero
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    psrlh   $v0, $v0, 15
    pcpyld  $a0, $t0, $a0
    pcpyld  $a3, $t1, $a3
    qfsrv   $a0, $a0, $a0
    qfsrv   $a3, $a3, $a3
    pextlb  $a0, $zero, $a0
    pextlb  $a3, $zero, $a3
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t5,   0($a1)
    ld      $t6,  64($a1)
    ld      $t7, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    pcpyld  $t5, $t7, $t5
    pcpyld  $t6, $t8, $t6
    qfsrv   $t5, $t5, $t5
    qfsrv   $t6, $t6, $t6
    pextlb  $t5, $zero, $t5
    pextlb  $t6, $zero, $t6
    paddh   $t1, $t5, $a0
    paddh   $t2, $t6, $a3
    por     $a0, $zero, $t5
    por     $a3, $zero, $t6
    paddh   $t1, $t1, $v0
    paddh   $t2, $t2, $v0
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,   0($a2)
    sq      $t2, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_put_luma_XY:
    mtsab   $a3, 0
    lq      $v0,   0($a1)
    lq      $t7, 384($a1)
    addu    $a1, $a1, $t3
    qfsrv   $t8, $t7, $v0
    qfsrv   $t9, $v0, $t7
    addiu   $v1, $v1, -1
    pextlb  $v0, $zero, $t8
    pextub  $t7, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $v0, $v0, $t8
    paddh   $t7, $t7, $t9
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    mtsab   $a3, 0
    addu    $a1, $a1, $t3
    qfsrv   $t8, $t6, $t5
    qfsrv   $t9, $t5, $t6
    addiu   $v1, $v1, -1
    pextlb  $t5, $zero, $t8
    pextub  $t6, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $t5, $t5, $t8
    paddh   $t6, $t6, $t9
    paddh   $t8, $v0, $t5
    paddh   $t9, $t7, $t6
    por     $v0, $zero, $t5
    pnor    $t5, $zero, $zero
    por     $t7, $zero, $t6
    psrlh   $t5, $t5, 15
    psllh   $t5, $t5,  1
    paddh   $t8, $t8, $t5
    paddh   $t9, $t9, $t5
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

_MPEG_put_chroma_XY:
    mtsab   $a3, 0
    pnor    $t9, $zero, $zero
    ld      $a0,   0($a1)
    ld      $v0,  64($a1)
    mtsab   $zero, 1
    ld      $t0, 384($a1)
    ld      $t1, 448($a1)
    pcpyld  $a0, $t0, $a0
    pcpyld  $v0, $t1, $v0
    qfsrv   $a0, $a0, $a0
    qfsrv   $v0, $v0, $v0
    psrlh   $t9, $t9, 15
    psllh   $t9, $t9, 1
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t0, $a0, $a0
    qfsrv   $t1, $v0, $v0
    pextlb  $a0, $zero, $a0
    pextlb  $v0, $zero, $v0
    pextlb  $t0, $zero, $t0
    pextlb  $t1, $zero, $t1
    paddh   $a0, $a0, $t0
    paddh   $t0, $v0, $t1
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t5,   0($a1)
    ld      $t7,  64($a1)
    mtsab   $a3, 0
    ld      $t6, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t5, $t6, $t5
    pcpyld  $t7, $t8, $t7
    qfsrv   $t5, $t5, $t5
    qfsrv   $t7, $t7, $t7
    addiu   $v0, $zero, 1
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    mtsab   $v0, 0
    qfsrv   $t6, $t5, $t5
    qfsrv   $t8, $t7, $t7
    pextlb  $t5, $zero, $t5
    pextlb  $t7, $zero, $t7
    pextlb  $t6, $zero, $t6
    pextlb  $t8, $zero, $t8
    paddh   $t5, $t5, $t6
    paddh   $t6, $t7, $t8
    paddh   $t7, $a0, $t5
    paddh   $t8, $t0, $t6
    por     $a0, $zero, $t5
    por     $t0, $zero, $t6
    paddh   $t7, $t7, $t9
    paddh   $t8, $t8, $t9
    psrlh   $t7, $t7, 2
    psrlh   $t8, $t8, 2
    sq      $t7,   0($a2)
    sq      $t8, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_avg_luma:
    mtsab   $a3, 0
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t5, $t6, $t5
    pextlb  $t6, $zero, $t5
    pextub  $t5, $zero, $t5
    lq      $t8,  0($a2)
    lq      $t9, 16($a2)
    paddh   $t6, $t6, $t8
    paddh   $t5, $t5, $t9
    pcgth   $t8, $t6, $zero
    pcgth   $t9, $t5, $zero
    pceqh   $v0, $t6, $zero
    pceqh   $t7, $t5, $zero
    psrlh   $t8, $t8, 15
    psrlh   $t9, $t9, 15
    psrlh   $v0, $v0, 15
    psrlh   $t7, $t7, 15
    por     $t8, $t8, $v0
    por     $t9, $t9, $t7
    paddh   $t6, $t6, $t8
    paddh   $t5, $t5, $t9
    psrlh   $t6, $t6, 1
    psrlh   $t5, $t5, 1
    sq      $t6,  0($a2)
    sq      $t5, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_avg_chroma:
    mtsab   $a3, 0
1:
    ld      $t5,   0($a1)
    ld      $t6,  64($a1)
    addiu   $v1, $v1, -1
    ld      $t7, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $t3
    pcpyld  $t5, $t7, $t5
    pcpyld  $t6, $t8, $t6
    qfsrv   $t5, $t5, $t5
    qfsrv   $t6, $t6, $t6
    pextlb  $t5, $zero, $t5
    pextlb  $t6, $zero, $t6
    lq      $t0,   0($a2)
    lq      $t1, 128($a2)
    paddh   $t5, $t5, $t0
    paddh   $t6, $t6, $t1
    pcgth   $t0, $t5, $zero
    pcgth   $t1, $t6, $zero
    pceqh   $v0, $t5, $zero
    pceqh   $t9, $t6, $zero
    psrlh   $t0, $t0, 15
    psrlh   $t1, $t1, 15
    psrlh   $v0, $v0, 15
    psrlh   $t9, $t9, 15
    por     $t0, $t0, $v0
    por     $t1, $t1, $t9
    paddh   $t5, $t5, $t0
    paddh   $t6, $t6, $t1
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    sq      $t5,   0($a2)
    sq      $t6, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_avg_luma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    mtsab   $a3, 0
    qfsrv   $t7, $t6, $t5
    qfsrv   $t8, $t5, $t6
    pextlb  $t5, $zero, $t7
    pextub  $t6, $zero, $t7
    addu    $a1, $a1, $t3
    mtsab   $zero, 1
    addiu   $v1, $v1, -1
    qfsrv   $t8, $t8, $t7
    pextlb  $t7, $zero, $t8
    pextub  $t8, $zero, $t8
    paddh   $t5, $t5, $t7
    paddh   $t6, $t6, $t8
    paddh   $t5, $t5, $v0
    paddh   $t6, $t6, $v0
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    lq      $t8,  0($a2)
    lq      $t9, 16($a2)
    paddh   $t5, $t5, $t8
    paddh   $t6, $t6, $t9
    pcgth   $t8, $t5, $zero
    pceqh   $t9, $t5, $zero
    psrlh   $t8, $t8, 15
    psrlh   $t9, $t9, 15
    por     $t8, $t8, $t9
    paddh   $t5, $t5, $t8
    pcgth   $t8, $t6, $zero
    pceqh   $t9, $t6, $zero
    psrlh   $t8, $t8, 15
    psrlh   $t9, $t9, 15
    por     $t8, $t8, $t9
    paddh   $t6, $t6, $t8
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    sq      $t5,  0($a2)
    sq      $t6, 16($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 32
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 512
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_avg_chroma_X:
    pnor    $v0, $zero, $zero
    psrlh   $v0, $v0, 15
1:
    ld      $t5,   0($a1)
    ld      $t6,  64($a1)
    mtsab   $a3, 0
    ld      $t7, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t5, $t7, $t5
    pcpyld  $t6, $t8, $t6
    qfsrv   $t5, $t5, $t5
    qfsrv   $t6, $t6, $t6
    addiu   $t9, $zero, 1
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    mtsab   $t9, 0
    qfsrv   $t1, $t5, $t5
    qfsrv   $t2, $t6, $t6
    pextlb  $t5, $zero, $t5
    pextlb  $t6, $zero, $t6
    pextlb  $t1, $zero, $t1
    pextlb  $t2, $zero, $t2
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    paddh   $t5, $t5, $v0
    paddh   $t6, $t6, $v0
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    lq      $t1,   0($a2)
    lq      $t2, 128($a2)
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    pcgth   $t1, $t5, $zero
    pcgth   $t2, $t6, $zero
    pceqh   $t9, $t5, $zero
    pceqh   $a0, $t6, $zero
    psrlh   $t1, $t1, 15
    psrlh   $t2, $t2, 15
    psrlh   $t9, $t9, 15
    psrlh   $a0, $a0, 15
    por     $t1, $t1, $t9
    por     $t2, $t2, $a0
    paddh   $t5, $t5, $t1
    paddh   $t6, $t6, $t2
    psrlh   $t5, $t5, 1
    psrlh   $t6, $t6, 1
    sq      $t5,   0($a2)
    sq      $t6, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_avg_luma_Y:
    mtsab   $a3, 0
    lq      $t7,   0($a1)
    lq      $t8, 384($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t7, $t8, $t7
    pextub  $t8, $zero, $t7
    pextlb  $t7, $zero, $t7
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t5, $t6, $t5
    pextub  $t6, $zero, $t5
    pextlb  $t5, $zero, $t5
    paddh   $v0, $t6, $t8
    pnor    $t8, $zero, $zero
    paddh   $t9, $t5, $t7
    psrlh   $t8, $t8, 15
    por     $t7, $zero, $t5
    paddh   $t9, $t9, $t8
    paddh   $v0, $v0, $t8
    por     $t8, $zero, $t6
    psrlh   $t9, $t9, 1
    psrlh   $v0, $v0, 1
    lq      $t5,  0($a2)
    lq      $t6, 16($a2)
    paddh   $t9, $t9, $t5
    paddh   $v0, $v0, $t6
    pcgth   $t5, $t9, $zero
    pceqh   $t6, $t9, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t9, $t9, $t5
    pcgth   $t5, $v0, $zero
    pceqh   $t6, $v0, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $v0, $v0, $t5
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

_MPEG_avg_chroma_Y:
    mtsab   $a3, 0
    ld      $a0,   0($a1)
    ld      $a3,  64($a1)
    ld      $t0, 384($a1)
    ld      $t1, 448($a1)
    pnor    $v0, $zero, $zero
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    psrlh   $v0, $v0, 15
    pcpyld  $a0, $t0, $a0
    pcpyld  $a3, $t1, $a3
    qfsrv   $a0, $a0, $a0
    qfsrv   $a3, $a3, $a3
    pextlb  $a0, $zero, $a0
    pextlb  $a3, $zero, $a3
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t5,   0($a1)
    ld      $t6,  64($a1)
    addiu   $v1, $v1, -1
    ld      $t7, 384($a1)
    ld      $t8, 448($a1)
    addu    $a1, $a1, $t3
    pcpyld  $t5, $t7, $t5
    pcpyld  $t6, $t8, $t6
    qfsrv   $t5, $t5, $t5
    qfsrv   $t6, $t6, $t6
    pextlb  $t5, $zero, $t5
    pextlb  $t6, $zero, $t6
    paddh   $t1, $t5, $a0
    paddh   $t2, $t6, $a3
    por     $a0, $zero, $t5
    por     $a3, $zero, $t6
    paddh   $t1, $t1, $v0
    paddh   $t2, $t2, $v0
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    lq      $t5,   0($a2)
    lq      $t6, 128($a2)
    paddh   $t1, $t1, $t5
    paddh   $t2, $t2, $t6
    pcgth   $t5, $t1, $zero
    pceqh   $t6, $t1, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t1, $t1, $t5
    pcgth   $t5, $t2, $zero
    pceqh   $t6, $t2, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t2, $t2, $t5
    psrlh   $t1, $t1, 1
    psrlh   $t2, $t2, 1
    sq      $t1,   0($a2)
    sq      $t2, 128($a2)
    bgtz    $v1, 1b
    addiu   $a2, $a2, 16
2:
    addu    $v1, $zero, $at
    addiu   $a1, $a1, 704
    bgtzl   $v1, 1b
    addu    $at, $zero, $zero
    jr      $ra

_MPEG_avg_luma_XY:
    mtsab   $a3, 0
    lq      $v0,   0($a1)
    lq      $t7, 384($a1)
    addu    $a1, $a1, $t3
    qfsrv   $t8, $t7, $v0 
    qfsrv   $t9, $v0, $t7
    addiu   $v1, $v1, -1
    pextlb  $v0, $zero, $t8
    pextub  $t7, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $v0, $v0, $t8
    paddh   $t7, $t7, $t9
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    lq      $t5,   0($a1)
    lq      $t6, 384($a1)
    mtsab   $a3, 0
    addu    $a1, $a1, $t3 
    qfsrv   $t8, $t6, $t5
    qfsrv   $t9, $t5, $t6
    addiu   $v1, $v1, -1
    pextlb  $t5, $zero, $t8
    pextub  $t6, $zero, $t8
    mtsab   $zero, 1
    qfsrv   $t9, $t9, $t8
    pextlb  $t8, $zero, $t9
    pextub  $t9, $zero, $t9
    paddh   $t5, $t5, $t8
    paddh   $t6, $t6, $t9
    paddh   $t8, $v0, $t5
    paddh   $t9, $t7, $t6
    por     $v0, $zero, $t5
    pnor    $t5, $zero, $zero
    por     $t7, $zero, $t6
    psrlh   $t5, $t5, 15
    psllh   $t5, $t5,  1
    paddh   $t8, $t8, $t5
    paddh   $t9, $t9, $t5
    psrlh   $t8, $t8, 2
    psrlh   $t9, $t9, 2
    lq      $t5,  0($a2)
    lq      $t6, 16($a2)
    paddh   $t8, $t8, $t5
    paddh   $t9, $t9, $t6
    pcgth   $t5, $t8, $zero
    pceqh   $t6, $t8, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t8, $t8, $t5
    pcgth   $t5, $t9, $zero
    pceqh   $t6, $t9, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t9, $t9, $t5
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

_MPEG_avg_chroma_XY:
    mtsab   $a3, 0
    pnor    $t9, $zero, $zero
    ld      $a0,   0($a1)
    ld      $v0,  64($a1)
    mtsab   $zero, 1
    ld      $t0, 384($a1)
    ld      $t1, 448($a1)
    pcpyld  $a0, $t0, $a0
    pcpyld  $v0, $t1, $v0
    qfsrv   $a0, $a0, $a0
    qfsrv   $v0, $v0, $v0
    psrlh   $t9, $t9, 15
    psllh   $t9, $t9,  1
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    qfsrv   $t0, $a0, $a0
    qfsrv   $t1, $v0, $v0
    pextlb  $a0, $zero, $a0
    pextlb  $v0, $zero, $v0
    pextlb  $t0, $zero, $t0
    pextlb  $t1, $zero, $t1
    paddh   $a0, $a0, $t0
    paddh   $t0, $v0, $t1
    beq     $v1, $zero, 2f
    addiu   $at, $at, 1
1:
    ld      $t5,   0($a1)
    ld      $t7,  64($a1)
    mtsab   $a3, 0
    ld      $t6, 384($a1)
    ld      $t8, 448($a1)
    pcpyld  $t5, $t6, $t5
    pcpyld  $t7, $t8, $t7
    qfsrv   $t5, $t5, $t5
    qfsrv   $t7, $t7, $t7
    addiu   $v0, $zero, 1
    addu    $a1, $a1, $t3
    addiu   $v1, $v1, -1
    mtsab   $v0, 0
    qfsrv   $t6, $t5, $t5
    qfsrv   $t8, $t7, $t7
    pextlb  $t5, $zero, $t5
    pextlb  $t7, $zero, $t7
    pextlb  $t6, $zero, $t6
    pextlb  $t8, $zero, $t8
    paddh   $t5, $t5, $t6
    paddh   $t6, $t7, $t8
    paddh   $t7, $a0, $t5
    paddh   $t8, $t0, $t6
    por     $a0, $zero, $t5
    por     $t0, $zero, $t6
    paddh   $t7, $t7, $t9
    paddh   $t8, $t8, $t9
    psrlh   $t7, $t7, 2
    psrlh   $t8, $t8, 2
    lq      $t5,   0($a2)
    lq      $t6, 128($a2)
    paddh   $t7, $t7, $t5
    paddh   $t8, $t8, $t6
    pcgth   $t5, $t7, $zero
    pceqh   $t6, $t7, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t7, $t7, $t5
    pcgth   $t5, $t8, $zero
    pceqh   $t6, $t8, $zero
    psrlh   $t5, $t5, 15
    psrlh   $t6, $t6, 15
    por     $t5, $t5, $t6
    paddh   $t8, $t8, $t5
    psrlh   $t7, $t7, 1
    psrlh   $t8, $t8, 1
    sq      $t7,   0($a2)
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

