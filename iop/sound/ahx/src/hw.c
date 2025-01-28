/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

__asm__
(
    "\t" ".set push" "\n"
    "\t" ".set noreorder" "\n"

    "\t" "\t" ".globl  wmemcpy" "\n"
    "\t" "\t" ".ent    wmemcpy" "\n"
    "\t" "wmemcpy:" "\n"
    "\t" "\t" "sra     $6,$6,0x2" "\n"
    "\t" "\t" "addiu   $6,$6,-1" "\n"
    "\t" "\t" "li      $2,-1" "\n"
    "\t" "\t" "beq     $6,$2,wmemcopy_end" "\n"
    "\t" "\t" "move    $3,$2" "\n"
    "\t" "wmemcopy_loop:" "\n"
    "\t" "\t" "lw      $2,0($5)" "\n"
    "\t" "\t" "addiu   $5,$5,4" "\n"
    "\t" "\t" "addiu   $6,$6,-1" "\n"
    "\t" "\t" "sw      $2,0($4)" "\n"
    "\t" "\t" "bne     $6,$3,wmemcopy_loop" "\n"
    "\t" "\t" "addiu   $4,$4,4" "\n"
    "\t" "wmemcopy_end:" "\n"
    "\t" "\t" "jr      $31" "\n"
    "\t" "\t" "nop" "\n"
    "\t" "\t" ".end    wmemcpy" "\n"

    "\t" ".set pop" "\n"
);
