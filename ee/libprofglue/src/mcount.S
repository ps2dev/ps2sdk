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

# Make sure we're not recursively called when compiling __mcount()
# With -pg
        la   $a0, _busy
        lw   $a1, 0($a0)
        bnez $a1, done
        nop

# Mark busy
        li   $a1, 1
        sw   $a1, 0($a0)

# Call internal C handler
        move $a0, $at
        move $a1, $ra
        jal  __mcount
        nop

# Unmark busy
        la   $a0, _busy
        li   $a1, 0
        sw   $a1, 0($a0)

done:

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

_busy:
        .space 4

    .end _mcount

    .set reorder
    .set at
