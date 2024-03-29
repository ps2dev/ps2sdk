.global  WriteBackDCache
.ent     WriteBackDCache
WriteBackDCache:

    lui    $25, 0xffff
    ori    $25, $25, 0xffc0
    blez   $5, last
    addu   $10, $4, $5
    and    $8, $4, $25
    addiu  $10, $10, -1
    and    $9, $10, $25
    subu   $10, $9, $8
    srl    $11, $10, 0x6
    addiu  $11, $11, 1
    andi   $9, $11, 0x7
    beqz   $9, eight
    srl    $10, $11, 0x3
loop1:
    sync
    cache    0x18, 0($8)
    sync
    addiu    $9, $9, -1
    nop
    bgtz     $9, loop1
    addiu    $8, $8, 64

eight:
    beqz     $10, last
loop8:
    addiu    $10, $10, -1
    sync
    cache    0x18, 0($8)
    sync
    cache    0x18, 64($8)
    sync
    cache    0x18, 128($8)
    sync
    cache    0x18, 192($8)
    sync
    cache    0x18, 256($8)
    sync
    cache    0x18, 320($8)
    sync
    cache    0x18, 384($8)
    sync
    cache    0x18, 448($8)
    sync
    bgtz     $10, loop8
    addiu    $8, $8, 512
last:
    jr       $31
    nop

.end    WriteBackDCache
