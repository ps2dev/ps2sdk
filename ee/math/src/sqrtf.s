# sqrtf.s

    .set noat
    .set noreorder

.text

    .globl sqrtf
    .ent sqrtf
sqrtf:
    sqrt.s $f0, $f12
    jal $31
    nop
    .end sqrtf


    .globl rsqrtf
    .ent rsqrtf
rsqrtf:
    rsqrt.s $f0, $f12
, $f13
    jal $31
    nop
    .end rsqrtf

