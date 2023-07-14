#include "as_reg_compat.h"

.ifdef .gasversion.
.section .mdebug.abi32
.nan legacy
.module softfloat
.module nooddspreg
.endif
.set push
.set noat
.set noreorder

.text

.globl CpuExecuteKmode
.ent CpuExecuteKmode
CpuExecuteKmode:
    li      $v0, 0x0C
    syscall
    jr      $ra
    nop
.end CpuExecuteKmode

.set pop
