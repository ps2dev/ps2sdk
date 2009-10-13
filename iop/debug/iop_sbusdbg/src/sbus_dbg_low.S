.set push
.set noreorder
.set noat

.global soft_break
.ent soft_break
soft_break:
                        nop
                        break
                        nop
                        jr      $ra
                        nop
.end soft_break

.set pop
