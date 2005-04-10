# fptoint.s

    .globl fptosi
    .ent fptosi
fptosi:
    cvt.w.s $f12, $f12
    mfc1 $2, $f12

    #mfc1 $2, $f12
    #qmtc2 $2, $vf4
    #vftoi0.x $vf4x, $vf4x
    #qmfc2 $2, $vf4
    #li $3, 0x0000ffff
    #and $2, $3

    jr $31
    nop
    .end fptosi


    .globl fptoui
    .ent fptoui
fptoui:
    abs.s $f12, $f12
    cvt.w.s $f12, $f12
    mfc1 $2, $f12

    jr $31
    nop
    .end fptoui

