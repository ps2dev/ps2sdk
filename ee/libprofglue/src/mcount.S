    .set noreorder
    .set noat

    .global _mcount
    .ent _mcount

_mcount:

# Generated code already substracts 8 bytes
# We store our ra, at and a0-a3
        daddiu $sp, $sp, -56      # Adjust stack pointer for 64-bit registers, 7 registers * 8 bytes each
        sd   $ra, 0($sp)          # store ra
        sd   $at, 8($sp)          # at = ra of caller
        sd   $a0, 16($sp)
        sd   $a1, 24($sp)
        sd   $a2, 32($sp)
        sd   $a3, 40($sp)

# Call internal C handler
        move $a0, $at
        move $a1, $ra
        jal  __mcount
        nop

# Restore registers
        ld   $ra, 0($sp)
        ld   $at, 8($sp)
        ld   $a0, 16($sp)
        ld   $a1, 24($sp)
        ld   $a2, 32($sp)
        ld   $a3, 40($sp)
        daddiu $sp, $sp, 56      # Adjust stack pointer back
        jr   $ra
        move $ra, $at            # restore caller's ra

    .end _mcount

    .set reorder
    .set at
