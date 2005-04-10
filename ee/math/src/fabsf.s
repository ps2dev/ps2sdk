    .set noreorder
    .set noat

.text

    .globl fabsf
    .ent fabsf
# float fabsf(float x);
fabsf:
    abs.s $f0, $f12
    jr $31
    nop
    .end fabsf
