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
.globl _MPEG_dma_ref_image
.globl _MPEG_do_mc
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
.set at
    sw      $a1, s_SetDMA + 0
    sw      $v1, 0x2010($v0)
    sw      $a2, s_SetDMA + 4
    sw      $a3, s_pEOF
.set noat
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
.set at
    sw      $v0, s_Sema
.set noat
    addiu   $a0, $zero,  3
    addiu   $v1, $zero, 18
    lui     $a1, %hi( _mpeg_dmac_handler )
    la      $a3, s_CSCParam
    xor     $a2, $a2, $a2
    addiu   $a1, %lo( _mpeg_dmac_handler )
    lw      $ra, 0($sp)
    syscall
    addiu   $sp, $sp, 48
.set at
    sw      $v0, s_CSCID
    sd      $zero, s_DataBuf
.set noat
    jr      $ra
    nop
    

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
    lw      $t8, -19456($a1)
    sra     $a3, $v1, 8
    subu    $a5, $a1, $v0
    and     $t8, $t8, $a3
    sw      $t8, -19456($a1)
    lw      $a2, -2784($a1)
.set at
    sw      $t8, s_IPUState + 0
.set noat
    and     $a2, $a2, $v1
    sw      $a2, -2672($a1)
    ei
    lw      $t8, -19440($a1)
    lw      $a2, -19424($a1)
.set at
    sw      $t8, s_IPUState + 4
    sw      $a2, s_IPUState + 8
.set noat
1:
    lw      $at, 0x2010($a5)
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
    lw      $t8, -20480($a1)
    and     $t8, $t8, $a3
    sw      $t8, -20480($a1)
    lw      $a2, -2784($a1)
.set at
    sw      $t8, s_IPUState + 12
    and     $a2, $a2, $v1
    sw      $a2, -2672($a1)
    ei
    lw      $t8, -20464($a1)
    lw      $a2, -20448($a1)
    sw      $t8, s_IPUState + 16
    sw      $a2, s_IPUState + 20
    lw      $t8, 0x2010($a5)
    lw      $a2, 0x2020($a5)
    sw      $t8, s_IPUState + 24
    sw      $a2, s_IPUState + 28
.set noat
    jr      $ra
    nop

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
    lui     $a5, 0x1001
    sll     $v0, $a3, 10
    mult    $v1, $v1, $a3
    subu    $at, $at, $a3
    sw      $a2, -20464($a5)
    sw      $a0, -19440($a5)
    addu    $a2, $a2, $v0
    srl     $v0, $v0, 4
    addu    $a0, $a0, $v1
    sw      $a0, 0($a1)
    srl     $v1, $v1, 4
    sw      $a2, 4($a1)
    lui     $a4, 0x1000
    sw      $at, 8($a1)
    sw      $v0, -20448($a5)
    lui     $v0, 0x7000
    sw      $v1, -19424($a5)
    addiu   $v1, $zero, 0x0101
    or      $v0, $v0, $a3
    sw      $v1, -19456($a5)
    andi    $v1, 0x0100
    sw      $v0, 0x2000($a4)
    sw      $v1, -20480($a5)
    jr      $ra
    nor     $v0, $zero, $zero
1:
    addiu   $a0, $zero, 3
    syscall
    lw      $a0, s_Sema
    addiu   $v1, $zero, -67
    syscall
.set at
    sb      $zero, s_CSCFlag
.set noat
    jr      $ra
    nor     $v0, $zero, $zero

_MPEG_CSCImage:
    addiu   $sp, $sp, -16
    sw      $ra,  0($sp)
    sw      $a0,  4($sp)
    sw      $a1,  8($sp)
    bgezal  $zero, _ipu_suspend
    sw      $a2, 12($sp)
    sw      $zero, 0x2000($a5)
    addiu   $a4, $zero, 1023
    addiu   $v0, $zero,    8
    addiu   $a0, $zero,    3
    addiu   $v1, $zero,   22
    lw      $a2, 12($sp)
    addiu   $a7, $zero,  384
    sw      $v0, -8176($a1)
    pminw   $a4, $a4, $a2
    lw      $t0, 4($sp)
    lw      $a3, 8($sp)
    subu    $a2, $a2, $a4
    mult    $a7, $a7, $a4
    sll     $t1, $a4, 10
    sw      $a3, -20464($a1)
    sw      $t0, -19440($a1)
.set at
    sw      $a2, s_CSCParam + 8
    addu    $t0, $t0, $a7
    addu    $a3, $a3, $t1
    sw      $t0, s_CSCParam
    srl     $a7, $a7, 4
    sw      $a3, s_CSCParam + 4
.set noat
    srl     $t1, $t1, 4
    sw      $a7, -19424($a1)
    sw      $t1, -20448($a1)
    sw      $a4, 4($sp)
    syscall
    lw      $a4, 4($sp)
    addiu   $v1, $zero, 0x0101
    lui     $at, 0x1001
    lui     $v0, 0x7000
    lui     $a0, 0x1000
    or      $v0, $v0, $a4
    sw      $v1, -19456($at)
    andi    $v1, $v1, 0x0100
    sw      $v0, 0x2000($a0)
    sw      $v1, -20480($at)
    lw      $a0, s_Sema
    addiu   $v1, $zero, 68
.set at
    sb      $v1, s_CSCFlag
.set noat
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
.set at
    sw      $a0, s_DataBuf
    sw      $v0, s_DataBuf + 4
.set noat
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
.set at
    sw      $v1, s_DataBuf + 0
    subu    $a2, $zero, $s0
    sllv    $v1, $v0, $s0
    srlv    $v0, $v0, $a2
    sw      $v1, s_DataBuf + 4
.set noat
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
.set at
    sw      $v1, s_DataBuf + 0
    sw      $v0, s_DataBuf + 4
.set noat
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
    lq      $a4, 64($at)
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
    sq      $a4, 0x7010($v1)
    sq      $a4, 0x7010($v1)
    sq      $a4, 0x7010($v1)
    sq      $a4, 0x7010($v1)
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
.set at
    sd      $zero, s_DataBuf
.set noat
    jr      $ra
    nop

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
.set at
    sw      $v1, s_DataBuf + 0
    sw      $a0, s_DataBuf + 4
.set noat
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
.set at
    sw      $v1, s_DataBuf + 0
    sw      $a1, s_DataBuf + 4
.set noat
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
.set at
    sw      $v1, s_DataBuf + 0
    sw      $a1, s_DataBuf + 4
.set noat
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
.set at
    sw      $v1, s_DataBuf + 0
    sw      $a1, s_DataBuf + 4
.set noat
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
    sll     $a4, $a4, 4
    or      $s0, $s0, $a3
    srl     $a4, $a4, 4
    lui     $a1, 0x1001
    lui     $at, 0x1000
    or      $a4, $a4, $a0
    lw      $v1, 0x2010($at)
    addiu   $a0, $zero, 48
    addiu   $a2, $zero, 0x0100
    sw      $a4, -20464($a1)
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
.set at
    sd      $v1, s_DataBuf
.set noat
    jr      $ra
    addiu   $sp, $sp, 16
3:
    bgezal  $zero, _ipu_suspend
    lui     $a4, 0x4000
    bgezal  $zero, _ipu_resume
    sw      $a4, 0x2010($a5)
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
    lui     $a5, 0x2000
    la      $a4, s_DMAPack
1:
    lw      $v1, -11264($v0)
    andi    $v1, $v1, 0x0100
    bne     $v1, $zero, 1b
    nop
    srl     $at, $at, 4
    sw      $zero, -11232($v0)
    or      $a5, $a5, $a4
    sw      $at, -11136($v0)
    lui     $v1, 0x3000
    sw      $a4, -11216($v0)
    ori     $v1, $v1, 0x0030
1:
    lw      $a4, 0($a1)
    addiu   $a2, $a2, -1
    sw      $v1,  0($a5)
    sw      $a4,  4($a5)
    addu    $a4, $a4, $a3
    sw      $v1, 16($a5)
    sw      $a4, 20($a5)
    sw      $a0, 0($a1)
    addiu   $a1, $a1, 40
    addiu   $a5, $a5, 32
    bgtz    $a2, 1b
    addiu   $a0, $a0, 1536
    andi    $v1, $v1, 0xFFFF
    addiu   $at, $zero, 0x0105
    sw      $v1, -16($a5)
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
